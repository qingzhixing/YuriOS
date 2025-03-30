//
// Created by qingzhixing on 25-3-9.
//

#include "memory.h"
#include "lib.h"
#include "printk.h"

void init_memory() {
	unsigned long TotalMem = 0;
	struct E820 *p = NULL;

	color_printk(BLUE, BLACK, "Displaying Memory Map, \n\t"
							  "Type: 1.RAM, 2.ROM or Reserved, 3.ACPI Reclaim Memory, 4.ACPI NVS Memory, 5.Undefined(Bad Memory)\n");
	p = (struct E820 *) 0xffff800000007e00;        // 在bootloader时我们保存在了这里

	for (int i = 0; i < 32; i++) {
		color_printk(ORANGE, BLACK, "Address:%#018lx\tLength:%#018lx\tType:%#010x\n",
					 p->address, p->length,p->type);
		if (p->type == 1) {
			TotalMem += p->length;
		}

		// Construct Memory Management Struct
		struct E820* e820 = &memory_management_struct.e820[i];
		e820->address = p->address;
		e820->length = p->length;
		e820->type = p->type;
		memory_management_struct.e820_length = i;

		p++;
		if (p->type > 4)
			break;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);

	// 2M Aligned Memory for Kernel
	TotalMem = 0;
	for(int i = 0; i <= memory_management_struct.e820_length; i++){
		unsigned long mem_start,mem_end;
		if(memory_management_struct.e820[i].type !=1)
			continue;
		mem_start = PAGE_2M_ALIGN(memory_management_struct.e820[i].address);
		mem_end = ((memory_management_struct.e820[i].address + memory_management_struct.e820[i].length)>>PAGE_2M_SHIFT)<<PAGE_2M_SHIFT;
		if(mem_end<= mem_start)
			continue;
		TotalMem += (mem_end - mem_start)>>PAGE_2M_SHIFT;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total 2M PAGEs:%#010x = %d\n", TotalMem,TotalMem);
}
