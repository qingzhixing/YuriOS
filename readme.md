# YuriOS

A tiny OS developed by qingzhixing!

## 👍 Waystones

- [X] 🍎 Booting Success at 2024.5.9!

![Booting image](assets/images/boot_success.png)

- [X] 💕 Fat32 Search in Root dir Success at 2024.5.10!

![Fat32 Search in Root dir!](assets/images/Fat32_Search_in_Root_dir!.png)

- [X] 🦄 fs寻址能力超过1M at 2024.6.26
  ![使fs寄存器拥有1M以上寻址的能力](assets/images/使fs寄存器拥有1M以上寻址的能力.png)
- [X] 🖥️ 成功读入kernel.bin at 2024.6.30
  ![成功读入kernel.bin](assets/images/read_kernel_bin.png)
- [X] ⌨️ 成功设置VESA VBE模式 at 2024.7.7
  ![VGA_INFO](assets/images/VGA_INFO.png)
  ![VBE模式](assets/images/VESA_VBE.png)
- [X] 🐳 成功进入保护模式 at 2024.7.30
  ![ProtectedModeCode](assets/images/ProtectedModeCode.png)
  ![ProtectedModeBochs](assets/images/ProtectedModeBochs.png)
  ![ProtectedModeReg](assets/images/ProtectedModeReg.png)
- [X] 📋 成功为64位下段描述符添加注释 at 2024.8.1
  ![LongModeSegmentDescriptor](assets/images/LongModeSegmentDescriptor.png)
- [X] OvO! Load IA32-e GDT! at 2024.8.22
  ![Load_IA32-e_GDT.png](assets/images/Load_IA32-e_GDT.png)

> 玩舞萌导致代码崩溃！！
> 原本的默认段寄存器ds写成了舞萌dx里的dx！
> 导致cpu寻址出错！
> 警惕舞萌dx！！！
> ![dx0](assets/images/dx0.png)
> ![dx1](assets/images/dx1.png)
> ![dx2](assets/images/dx2.png)
> ![dx3](assets/images/dx3.png)

- [X] 🍰 OwO?! Succeed in enabling Long Mode(Compatibility Mode)! at 2024.9.8
  ![LongModeEnable](assets/images/Enable_Long_Mode.png)
- [X] 🐱 ᓚᘏᗢ... Load Into Kernel Space! at 2024.9.8
  ![LoadIntoKernelSpace](assets/images/Load_Into_Kernel.png)
  ![Long_Mode_Kernel_Description.png](assets/images/Long_Mode_Kernel_Description.png)
- [X] 🌵 VGA显示色彩完美! at 2024.9.10
  ![VGA_Color_Perfect.png](assets/images/VGA显示色彩.png)
- [X] 🌈 成功修复putchar at 2024.9.13

> printk无法使用,会使屏幕全部变为背景色并陷入死循环

![putchar](assets/images/putchar.png)

- [X] 😕 使用putchar逐字打印没问题,将问题初步确定在color_printk中处理打印时出错 at 2024.9.15
  ![putchar_str](assets/images/putchar_str.png)
- [X] 🦙 color_printk 打印成功! at 2024.9.15
  ![color_printk](assets/images/color_printk.png)

  > 妈妈我再也不随便用AI了TT

>

- [X] 🦄 成功捕获异常并使用异常处理 at 2024.11.10
  ![成功捕获异常并使用异常处理](assets/images/成功捕获异常并使用异常处理.png)

- [X] 成功实现自定义中断处理函数 at 2025.2.23

> 我真的要写哭了.... DeepSeek nb 问了一堆,改来改去不知道为什么就对了,应该是head.S里面不要加载没写完的tss

### #DE

![自定义中断处理函数](assets/images/自定义中断处理函数成功运行-2025-2-23.png)
![自定义中断处理函数-仅输出](assets/images/自定义中断处理函数-仅输出.png)

### #PF

![PF-display](assets/images/PF_display.png)

- [x] 🈴 成功获取物理内存信息 at 2025.3.9
  ![成功获取物理内存信息](assets/images/init_memory.png)
  ![2GB内存](assets/images/init_memory_2GB.png)

- [x] 🥩 color_printk 颜色测试 & 2M 内存页数量计算 at 2025.3.30
  ![colorTest-2MPages.png](assets/images/colorTest-2MPages.png)

- [x] 🍭 内存管理结构初始化并打印 at 2025.3.30
  ![内存管理结构初始化并打印](assets/images/memory_manager_initialized.png)
