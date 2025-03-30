// Created by qingzhixing on 25-3-9.

#ifndef YURIOS_MEMORY_H
#define YURIOS_MEMORY_H

#include "lib.h"

// 页表项个数
#define PTRS_PER_PAGE 512
#define PAGE_OFFSET ((unsigned long)0xffff800000000000)

#define PAGE_GDT_SHIFT 39
#define PAGE_1G_SHIFT 30
#define PAGE_2M_SHIFT 21
#define PAGE_4K_SHIFT 12

#define PAGE_2M_SIZE (1UL << PAGE_2M_SHIFT)
#define PAGE_4K_SIZE (1UL << PAGE_4K_SHIFT)
#define PAGE_1G_SIZE (1UL << PAGE_1G_SHIFT)

#define PAGE_2M_MASK (~(PAGE_2M_SIZE - 1))
#define PAGE_4K_MASK (~(PAGE_4K_SIZE - 1))
#define PAGE_1G_MASK (~(PAGE_1G_SIZE - 1))

#define PAGE_2M_ALIGN(addr) (((unsigned long)(addr) + (PAGE_2M_SIZE - 1))& PAGE_2M_MASK)
#define PAGE_4K_ALIGN(addr) (((unsigned long)(addr) + (PAGE_4K_SIZE - 1))& PAGE_4K_MASK)
#define PAGE_1G_ALIGN(addr) (((unsigned long)(addr) + (PAGE_1G_SIZE - 1))& PAGE_1G_MASK)

#define Virt_To_Phy(addr) ((unsigned long)(addr) - PAGE_OFFSET)
#define Phy_To_Virt(addr) ((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))

struct Page;
struct Global_Memory_descriptor;
struct Zone;
struct E820;

struct E820 {
	unsigned long address;
	unsigned long length;
	unsigned int type;
}__attribute__((packed));

// each zone index

int ZONE_DMA_INDEX = 0;
int ZONE_NORMAL_INDEX = 0;    //low 1GB RAM ,was mapped in pagetable
int ZONE_UNMAPED_INDEX = 0;    //above 1GB RAM,unmapped in pagetable
#define MAX_NR_ZONES    10    //max zone

struct Zone {
	struct Page *pages_group;                        // Page数组
	unsigned long pages_length;                     // Page结构体数量

	unsigned long zone_start_address;               // 起始页对齐地址
	unsigned long zone_end_address;                 // 结束页对齐地址
	unsigned long zone_length;                      // 本区域经过页对齐之后的地址长度
	unsigned long attribute;                        // 本区域空间属性

	struct Global_Memory_descriptor *GMD_struct;    // 指向全局结构体

	unsigned long page_using_count;                 // 本区域已使用物理内存页数量
	unsigned long page_free_count;                  // 本区域空闲物理内存页数量

	unsigned long total_pages_link;                 // 本区域物理页被引用次数
};

//alloc_pages zone_select
#define ZONE_DMA    (1 << 0)
#define ZONE_NORMAL    (1 << 1)
#define ZONE_UNMAPED    (1 << 2)

//struct page attribute (alloc_pages flags)
#define PG_PTable_Maped    (1 << 0)
#define PG_Kernel_Init    (1 << 1)
#define PG_Referenced    (1 << 2)
#define PG_Dirty    (1 << 3)
#define PG_Active    (1 << 4)
#define PG_Up_To_Date    (1 << 5)
#define PG_Device    (1 << 6)
#define PG_Kernel    (1 << 7)
#define PG_K_Share_To_U    (1 << 8)
#define PG_Slab        (1 << 9)

unsigned long *Global_CR3 = NULL;

struct Page {
	struct Zone *zone_struct;       // 本页所属的zone
	unsigned long PHY_address;      // 物理地址
	unsigned long attribute;        // 属性

	unsigned long reference_count;  // 本页引用次数

	unsigned long age;              // 该页创建时间
};

struct Global_Memory_descriptor {
	// 物理内存段
	struct E820 e820[32];
	unsigned long e820_length;

	// 物理地址空间页映射位图
	unsigned long *bits_map;
	unsigned long bits_size;
	unsigned long bits_length;

	// 全局struct Page数组
	struct Page *pages_struct;
	unsigned long pages_size;
	unsigned long pages_length;

	// 全局struct Zone数组
	struct Zone *zones_struct;
	unsigned long zones_size;
	unsigned long zones_length;

	// 内核代码段起始结束地址, 数据段结束地址, 程序结尾地址
	unsigned long start_code, end_code, end_data, end_brk;

	// 内存页管理结构的结束地址
	unsigned long end_of_struct;
};

extern struct Global_Memory_descriptor memory_management_struct;

unsigned long page_init(struct Page *page, unsigned long flags);

unsigned long page_clean(struct Page *page);

#define    flush_tlb_one(addr)    \
    __asm__ __volatile__    ("invlpg	(%0)	\n\t"::"r"(addr):"memory")
/*

*/

// Translation Lookaside Buffer
#define flush_tlb()                        \
do                                \
{                                \
    unsigned long    tmpreg;                    \
    __asm__ __volatile__    (                \
                "movq	%%cr3,	%0	\n\t"    \
                "movq	%0,	%%cr3	\n\t"    \
                :"=r"(tmpreg)            \
                :                \
                :"memory"            \
                );                \
}while(0)

/*

*/

unsigned long *Get_gdt();


#endif //YURIOS_MEMORY_H
