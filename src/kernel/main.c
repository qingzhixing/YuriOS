//
// Created by qingzhixing on 24-6-30.
//
// ====== VGA_INFO
#define VGA_WIDTH 1440
#define VGA_HEIGHT 900
#define VGA_DEPTH 32
#define VGA_BASE 0xffff800000a00000
// ====== END VGA_INFO

void Start_Kernel(void){
    // 显示模式: 1440x900x32
    // 模式号: 0x180
    // 显存地址: 0xffff800000a00000
    int *frame_buffer = (int*)(VGA_BASE);
    for(int i = 0; i < VGA_WIDTH * 20; i++){
        *(((char*)frame_buffer) + 0) = 0x00;   // Blue
        *(((char*)frame_buffer) + 1) = 0x00;   // Green
        *(((char*)frame_buffer) + 2) = 0xff;   // Red
        *(((char*)frame_buffer) + 3) = 0x00;   // 保留
        frame_buffer += 1;
    }
    for(int i = 0; i < VGA_WIDTH * 20; i++){
        *(((char*)frame_buffer) + 0) = 0x00;   // Blue
        *(((char*)frame_buffer) + 1) = 0xff;   // Green
        *(((char*)frame_buffer) + 2) = 0x00;   // Red
        *(((char*)frame_buffer) + 3) = 0x00;   // 保留
        frame_buffer += 1;
    }

    for(int i = 0; i < VGA_WIDTH * 20; i++){
        *(((char*)frame_buffer) + 0) = 0xff;   // Blue
        *(((char*)frame_buffer) + 1) = 0x00;   // Green
        *(((char*)frame_buffer) + 2) = 0x00;   // Red
        *(((char*)frame_buffer) + 3) = 0x00;   // 保留
        frame_buffer += 1;
    }

    for(int i = 0; i < VGA_WIDTH * 20; i++){
        *(((char*)frame_buffer) + 0) = 0xff;   // Blue
        *(((char*)frame_buffer) + 1) = 0xff;   // Green
        *(((char*)frame_buffer) + 2) = 0x00;   // Red
        *(((char*)frame_buffer) + 3) = 0x00;   // 保留
        frame_buffer += 1;
    }

    for(int i = 0; i < VGA_WIDTH * 20; i++){
        *(((char*)frame_buffer) + 0) = 0xff;   // Blue
        *(((char*)frame_buffer) + 1) = 0xff;   // Green
        *(((char*)frame_buffer) + 2) = 0xff;   // Red
        *(((char*)frame_buffer) + 3) = 0x00;   // 保留
        frame_buffer += 1;
    }
    while(1);
}