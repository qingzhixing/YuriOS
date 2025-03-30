//
// Created by qingzhixing on 25-3-9.
//

#ifndef YURIOS_MEMORY_H
#define YURIOS_MEMORY_H

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

#define Vitr_To_Phy(addr) ((unsigned long)(addr) - PAGE_OFFSET)
#define Phy_To_Vitr(addr) ((unsigned long *)((unsigned long)(addr) + PAGE_OFFSET))

struct Memory_E820_Format {
	unsigned int address1;
	unsigned int address2;
	unsigned int length1;
	unsigned int length2;
	unsigned int type;
};
struct E820 {
	unsigned long address;
	unsigned long length;
	unsigned int type;
}__attribute__((packed));

struct Global_Memory_descriptor{
	struct E820 e820[32];
	unsigned long e820_length;
};

extern struct Global_Memory_descriptor memory_management_struct;

#endif //YURIOS_MEMORY_H
