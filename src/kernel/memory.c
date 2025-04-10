//
// Created by qingzhixing on 25-3-9.
//

#include "memory.h"
#include "lib.h"
#include "printk.h"
#include <stddef.h>


unsigned long page_init(struct Page *page, unsigned long flags) {
	if (!page->attribute) {
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |=
				1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
		page->attribute = flags;
		page->reference_count++;
		page->zone_struct->page_using_count++;
		page->zone_struct->page_free_count--;
		page->zone_struct->total_pages_link++;
	} else if ((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U) || (flags & PG_Referenced) ||
			   (flags & PG_K_Share_To_U)) {
		page->attribute |= flags;
		page->reference_count++;
		page->zone_struct->total_pages_link++;
	} else {
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) |=
				1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;
		page->attribute |= flags;
	}
	return 0;
}

unsigned long page_clean(struct Page *page) {
	if (!page->attribute) {
		page->attribute = 0;
	} else if ((page->attribute & PG_Referenced) || (page->attribute & PG_K_Share_To_U)) {
		page->reference_count--;
		page->zone_struct->total_pages_link--;
		if (!page->reference_count) {
			page->attribute = 0;
			page->zone_struct->page_using_count--;
			page->zone_struct->page_free_count++;
		}
	} else {
		*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) &= ~(1UL
				<< (page->PHY_address >> PAGE_2M_SHIFT) % 64);

		page->attribute = 0;
		page->reference_count = 0;
		page->zone_struct->page_using_count--;
		page->zone_struct->page_free_count++;
		page->zone_struct->total_pages_link--;
	}
	return 0;
}

void init_memory() {
	unsigned long TotalMem = 0;
	struct E820 *p = NULL;

	color_printk(BLUE, BLACK, "Displaying Memory Map, \n\t"
							  "Type: 1.RAM, 2.ROM or Reserved, 3.ACPI Reclaim Memory, 4.ACPI NVS Memory, 5.Undefined(Bad Memory)\n");
	p = (struct E820 *) 0xffff800000007e00;        // 在bootloader时我们保存在了这里

	for (int i = 0; i < 32; i++) {
		color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n",
					 p->address, p->length, p->type);
		if (p->type == 1) {
			TotalMem += p->length;
		}

		// Construct Memory Management Struct
		struct E820 *e820 = &memory_management_struct.e820[i];
		e820->address = p->address;
		e820->length = p->length;
		e820->type = p->type;
		memory_management_struct.e820_length = i;

		p++;
		if (p->type > 4 || p->length == 0 || p->type < 1)
			break;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

	// 2M Aligned Memory for Kernel
	TotalMem = 0;
	for (int i = 0; i <= memory_management_struct.e820_length; i++) {
		unsigned long mem_start, mem_end;
		if (memory_management_struct.e820[i].type != 1)
			continue;
		mem_start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		mem_end =
				((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT)
						<< PAGE_2M_SHIFT;
		if (mem_end <= mem_start)
			continue;
		TotalMem += (mem_end - mem_start) >> PAGE_2M_SHIFT;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010x = %d\n", TotalMem, TotalMem);

	// bits map construction init
	// 仅建立最后一个e820的位图(最大的可使用区域)
	TotalMem = memory_management_struct.e820[memory_management_struct.e820_length].address +
			   memory_management_struct.e820[memory_management_struct.e820_length].length;

	memory_management_struct.bits_map = (unsigned long *) ((memory_management_struct.end_brk + PAGE_4K_SIZE - 1) &
														   PAGE_4K_MASK);   // 在程序结束的后端预留4k建立位图

	memory_management_struct.bits_size = TotalMem >> PAGE_2M_SHIFT;
	memory_management_struct.bits_length =
			(((unsigned long) (TotalMem >> PAGE_2M_SHIFT) + sizeof(long) * 8 - 1) / 8) &
			(sizeof(long) * 8 - 1);        // 向下对齐
//	memory_management_struct.bits_length =
//			((TotalMem >> PAGE_2M_SHIFT) + 8 * sizeof(long) - 1) /
//			(8 * sizeof(long)) * sizeof(long);   // 向上取整
	memset(memory_management_struct.bits_map, 0xff, memory_management_struct.bits_length);

	// pages construction init
	memory_management_struct.pages_struct = (struct Page *) (
			((unsigned long) memory_management_struct.bits_map + memory_management_struct.bits_length + PAGE_4K_SIZE -
			 1) & PAGE_4K_MASK);
	memory_management_struct.pages_size = TotalMem >> PAGE_2M_SHIFT;
	memory_management_struct.pages_length =
			((TotalMem >> PAGE_2M_SHIFT) * sizeof(struct Page) + sizeof(long) - 1) & (~(sizeof(long) - 1));
	memset(memory_management_struct.pages_struct, 0x00, memory_management_struct.pages_length); // init pages memory

	// zones constructing init
	memory_management_struct.zones_struct = (struct Zone *) (
			((unsigned long) memory_management_struct.pages_struct + memory_management_struct.pages_length +
			 PAGE_4K_SIZE - 1) & PAGE_4K_MASK);
	memory_management_struct.zones_size = 0;
	memory_management_struct.zones_length = (5 * sizeof(struct Zone) + sizeof(long) - 1) &
											(~(sizeof(long) - 1));      // 暂时按5个zone分配
	memset(memory_management_struct.zones_struct, 0x00, memory_management_struct.zones_length); // init zones memory

	for (int i = 0; i <= memory_management_struct.e820_length; ++i) {
		unsigned long mem_start, mem_end;
		struct Zone *zone;
		struct Page *page;
		unsigned long *b;

		if (memory_management_struct.e820[i].type != 1)
			continue;
		mem_start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		mem_end =
				((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length) >> PAGE_2M_SHIFT)
						<< PAGE_2M_SHIFT;
		if (mem_end <= mem_start)
			continue;

		// Zone init
		zone = &memory_management_struct.zones_struct[memory_management_struct.zones_size++];

		zone->zone_start_address = mem_start;
		zone->zone_end_address = mem_end;
		zone->zone_length = mem_end - mem_start;

		zone->page_using_count = 0;
		zone->page_free_count = (mem_end - mem_start) >> PAGE_2M_SHIFT;
		zone->total_pages_link = 0;

		zone->attribute = 0;
		zone->GMD_struct = &memory_management_struct;

		zone->pages_length = (mem_end - mem_start) >> PAGE_2M_SHIFT;
		zone->pages_group = (struct Page *) (memory_management_struct.pages_struct + (mem_start >> PAGE_2M_SHIFT));

		// page init
		page = zone->pages_group;
		for (int j = 0; j < zone->zone_length; ++j) {
			page->zone_struct = zone;
			page->PHY_address = mem_start + j * PAGE_2M_SIZE;
			page->attribute = 0;

			page->reference_count = 0;
			page->age = 0;

			//一个 bit_map 有 8 Byte (sizeof(long)),管理 64 Pages
			*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) >> 6)) ^=
					1UL << (page->PHY_address >> PAGE_2M_SHIFT) & 63;

			// 等价于:
//			*(memory_management_struct.bits_map + ((page->PHY_address >> PAGE_2M_SHIFT) / 64)) ^=
//					1UL << (page->PHY_address >> PAGE_2M_SHIFT) % 64;

			page++;
		}
	}
	// 初始化 0~2M物理内存页
	memory_management_struct.pages_struct[0].zone_struct = &memory_management_struct.zones_struct[0];
	memory_management_struct.pages_struct[0].PHY_address = 0UL;
	memory_management_struct.pages_struct[0].attribute = 0;
	memory_management_struct.pages_struct[0].reference_count = 0;
	memory_management_struct.pages_struct[0].age = 0;


	memory_management_struct.zones_length = (memory_management_struct.zones_size * sizeof(struct Zone) +
											 sizeof(long) - 1) & (~(sizeof(long) - 1));

	// 打印内存管理信息
	color_printk(ORANGE, BLACK, "bits_map:%#018lx,bits_size:%#018lx,bits_length:%#018lx\n",
				 memory_management_struct.bits_map, memory_management_struct.bits_size,
				 memory_management_struct.bits_length);

	color_printk(ORANGE, BLACK, "pages_struct:%#018lx,pages_size:%#018lx,pages_length:%#018lx\n",
				 memory_management_struct.pages_struct, memory_management_struct.pages_size,
				 memory_management_struct.pages_length);

	color_printk(ORANGE, BLACK, "zones_struct:%#018lx,zones_size:%#018lx,zones_length:%#018lx\n",
				 memory_management_struct.zones_struct, memory_management_struct.zones_size,
				 memory_management_struct.zones_length);

	ZONE_DMA_INDEX = 0;       // need rewrite in the future
	ZONE_NORMAL_INDEX = 0; // need rewrite in the future

	for (int i = 0; i < memory_management_struct.zones_size; i++) // need rewrite in the future
	{
		struct Zone *z = memory_management_struct.zones_struct + i;
		color_printk(ORANGE, BLACK,
					 "zone_start_address:%#018lx,zone_end_address:%#018lx,zone_length:%#018lx,pages_group:%#018lx,pages_length:%#018lx\n",
					 z->zone_start_address, z->zone_end_address, z->zone_length, z->pages_group, z->pages_length);

		if (z->zone_start_address == 0x100000000)
			ZONE_UNMAPED_INDEX = i;
	}

	memory_management_struct.end_of_struct = (unsigned long) ((unsigned long) memory_management_struct.zones_struct +
															  memory_management_struct.zones_length +
															  sizeof(long) * 32) & (~(sizeof(long) -
																					  1)); ////need a blank to separate memory_management_struct

	color_printk(ORANGE, BLACK,
				 "start_code:%#018lx,end_code:%#018lx,end_data:%#018lx,end_brk:%#018lx,end_of_struct:%#018lx\n",
				 memory_management_struct.start_code, memory_management_struct.end_code,
				 memory_management_struct.end_data, memory_management_struct.end_brk,
				 memory_management_struct.end_of_struct);

	int i = Virt_To_Phy(memory_management_struct.end_of_struct) >> PAGE_2M_SHIFT;

	for (int j = 0; j <= i; j++) {
		page_init(memory_management_struct.pages_struct + j, PG_PTable_Maped | PG_Kernel_Init | PG_Active | PG_Kernel);
	}
	Global_CR3 = Get_gdt();

	color_printk(INDIGO, BLACK, "Global_CR3\t:%#018lx\n", Global_CR3);
	color_printk(INDIGO, BLACK, "*Global_CR3\t:%#018lx\n", *Phy_To_Virt(Global_CR3) & (~0xff));
	color_printk(PURPLE, BLACK, "**Global_CR3\t:%#018lx\n", *Phy_To_Virt(*Phy_To_Virt(Global_CR3) & (~0xff)) & (~0xff));

	// 消除一致性页表映射
	for (i = 0; i < 10; i++)
		*(Phy_To_Virt(Global_CR3) + i) = 0UL;

	// 刷新页表项
	flush_tlb();
}

inline unsigned long *Get_gdt() {
	unsigned long *tmp;
	__asm__ __volatile__    (
			"movq	%%cr3,	%0	\n\t"
			:"=r"(tmp)
			:
			:"memory"
			);
	return tmp;
}

/*

	number: number < 64

	zone_select: zone select from dma , mapped in  pagetable , unmapped in pagetable

	page_flags: struct Page flages

*/

struct Page * alloc_pages(int zone_select,int number,unsigned long page_flags)
{
	int i;
	unsigned long page = 0;

	int zone_start = 0;
	int zone_end = 0;

	switch(zone_select)
	{
		case ZONE_DMA:
				zone_start = 0;
				zone_end = ZONE_DMA_INDEX;

			break;

		case ZONE_NORMAL:
				zone_start = ZONE_DMA_INDEX;
				zone_end = ZONE_NORMAL_INDEX;

			break;

		case ZONE_UNMAPED:
				zone_start = ZONE_UNMAPED_INDEX;
				zone_end = memory_management_struct.zones_size - 1;

			break;

		default:
			color_printk(RED,BLACK,"alloc_pages error zone_select index\n");
			return NULL;
			break;
	}

	for(i = zone_start;i <= zone_end; i++)
	{
		struct Zone * z;
		unsigned long j;
		unsigned long start,end,length;
		unsigned long tmp;

		if((memory_management_struct.zones_struct + i)->page_free_count < number)
			continue;

		z = memory_management_struct.zones_struct + i;
		start = z->zone_start_address >> PAGE_2M_SHIFT;
		end = z->zone_end_address >> PAGE_2M_SHIFT;
		length = z->zone_length >> PAGE_2M_SHIFT;

		tmp = 64 - start % 64;

		for(j = start;j <= end;j += j % 64 ? tmp : 64)
		{
			unsigned long * p = memory_management_struct.bits_map + (j >> 6);
			unsigned long shift = j % 64;
			unsigned long k;
			for(k = shift; k < 64 - shift; k++)
			{
				if( !(((*p >> k) | (*(p + 1) << (64 - k))) & (number == 64 ? 0xffffffffffffffffUL : ((1UL << number) - 1))) )
				{
					unsigned long	l;
					page = j + k - 1;
					for(l = 0;l < number;l++)
					{
						struct Page * x = memory_management_struct.pages_struct + page + l;
						page_init(x,page_flags);
					}
					goto find_free_pages;
				}
			}
		
		}
	}

	return NULL;

find_free_pages:

	return (struct Page *)(memory_management_struct.pages_struct + page);
}


