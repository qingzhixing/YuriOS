//
// Created by qingzhixing on 25-3-9.
//

#include "memory.h"
#include "lib.h"
#include "printk.h"

void init_memory() {
	unsigned long TotalMem = 0;
	struct Memory_E820_Format *p = NULL;

	color_printk(BLUE, BLACK, "Displaying Memory Map, \n\t"
							  "Type: 1.RAM, 2.ROM or Reserved, 3.ACPI Reclaim Memory, 4.ACPI NVS Memory, 5.Undefined\n");
	p = (struct Memory_E820_Format *) 0xffff800000007e00;        // 在bootloader时我们保存在了这里

	for (int i = 0; i < 32; i++) {
		color_printk(ORANGE, BLACK, "Address:%#010x,%08x\tLength:%#010x,%08x\tType:%#010x\n",
					 p->address2, p->address1, p->length2, p->length1, p->type);
		unsigned long tmp = 0;
		if (p->type == 1) {
			tmp = p->length2;
			TotalMem += p->length1;
			TotalMem += tmp << 32;
		}

		p++;
		if (p->type > 4)
			break;
	}

	color_printk(ORANGE, BLACK, "OS Can Used Total RAM:%#018lx\n", TotalMem);
}
