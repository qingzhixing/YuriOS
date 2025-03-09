//
// Created by qingzhixing on 25-3-9.
//

#ifndef YURIOS_MEMORY_H
#define YURIOS_MEMORY_H

struct Memory_E820_Format {
	unsigned int address1;
	unsigned int address2;
	unsigned int length1;
	unsigned int length2;
	unsigned int type;
};

#endif //YURIOS_MEMORY_H
