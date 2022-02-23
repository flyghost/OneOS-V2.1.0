# eCoreDump组件

## 简介

eCoreDump组件实现了嵌入式系统的coredump功能。

配合使用的gdb是针对嵌入式系统修改的，下载路径为[arm-none-eabi-gdb](https://gitee.com/cmcc-oneos/openOCD/releases)。

## 代码结构

eCoreDump组件源代码相对路径为.\components\debug\eCoreDump，结构如下表所示：

| 文件名       | 功能     |
| ------------ | -------- |
| example      | 示例 |
| inc          | 头文件，提供组件接口 |
| port         | 移植文件 |
| src          | 源代码文件 |
| test         | 测试文件 |

## 功能介绍

eCoreDump组件主要提供下面两个功能：

1. 当assert发生后，生成coredump文件并通过log打印出来。
2. 发生硬件异常时触发，生成coredump文件并保存到flash上。

## 使用说明

### 图形化配置

使用eCoreDump组件可以通过Menuconfig的图形化工具进行配置选择，配置的路径如下所示：

```C
(Top) → Components→ Diagnose→ eCoreDump
                                        OneOS Configuration
[*] Using eCoreDump
        Select the arch (armv7m)  --->
[*]     Using faultdump
[*]     Using example
```

以上配置完成后退出并保存配置，重新生成工程后，编译运行OneOS。

### 运行示例

当代码主动调用`ecd_multi_dump`或`ecd_mini_dump`接口时，程序会以当前调用栈生成corefile，两个接口的区别为是否包含其他线程的信息。在这两个接口内部，会保存mcu寄存器和栈内存，这部分代码依赖于mcu架构和rtos。

eCoreDump的示例代码构造了assert的场景，在shell中输入`trigger_assert`命令触发。
```sh
sh />trigger_assert
Assert failed. Condition(x * a == y * b * c). [sh_trigger_assert][104]
coredump start : {
7f454c460101016100000000000000000400280001000000000000 ...
} coredump end
crc32 : be8f1ddb
sh />
```

生成的corefile以hex字符串的方式打印到终端，至此嵌入式端的工作完成，接下来PC上的操作步骤如下：
1. 将hex字符串格式的corefile转化为二进制文件。例如，hex字符串的内容为“454c46”，转化后的二进制文件长度为3个字节，内容为“ELF”。
2. 调用gdb解析corefile，`arm-none-eabi-gdb oneos.elf corefile`。

上面的操作可以由辅助工具ecdView.exe完成，操作步骤如下：
1. 将hex字符串格式的corefile复制到文本框中，
2. `Open`按钮选择elf文件路径，
3. `gdb`按钮启动gdb，解析coredump。

使用gdb命令解析coredump文件。
```gdb
(gdb) info threads
  Id   Target Id         Frame
* 1    Task 3539         ecd_multi_dump () at ..\\..\\components\\diagnose\\eCoreDump\\src\\armv7m\\/armv7m.c:122
  2    Task 3540         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  3    Task 3541         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  4    Task 3542         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  5    Task 3543         _k_idle_task_entry (arg=<optimized out>) at ..\..\kernel\source\/os_idle.c:17
  6    Task 3544         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  7    Task 3545         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  8    Task 3546         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
  9    Task 3547         k_kernel_exit_sched () at ..\..\kernel\source\/os_sched.c:302
```
```gdb
(gdb) bt
#0  ecd_multi_dump () at ..\\..\\components\\diagnose\\eCoreDump\\src\\armv7m\\/armv7m.c:122
#1  0x0800c49a in after_assert_fail () at ..\..\components\diagnose\eCoreDump\example\trigger_dump.c:69
#2  0x0802a4c2 in sh_trigger_assert (argc=1, argv=0x2000cf30 <stack_shell_stack+1912>) at ..\..\components\diagnose\eCoreDump\example\trigger_dump.c:97
#3  0x0802880c in sh_do_exec_cmd (cmd=0x2000d1f9 <gs_shell+465> "trigger_assert", length=14, retp=0x2000cf78 <stack_shell_stack+1984>) at ..\..\components\shell\source\/shell_process.c:197
#4  0x0802892e in sh_exec (cmd=0x2000d1f9 <gs_shell+465> "trigger_assert", length=14) at ..\..\components\shell\source\/shell_process.c:237
#5  0x08028c7e in sh_handle_enter_key (shell_info=0x2000d028 <gs_shell>) at ..\..\components\shell\source\/shell_main.c:545
#6  0x08028c0c in sh_handle_control_key (shell_info=0x2000d028 <gs_shell>, ch=13 '\r') at ..\..\components\shell\source\/shell_main.c:638
#7  0x0802a3c4 in sh_task_entry (arg=<optimized out>) at ..\..\components\shell\source\/shell_main.c:723
```

### 异常coredump与corefile保存功能

当程序发生异常时，程序会根据发生异常的调用栈生成corefile，并保存到flash中。示例中增加了`trigger_fault`命令触发hardfault。

硬件异常触发后程序停止，此时需要重启设备查看coredump文件，设备上可以保存多个coredump文件，使用`corefile_count`查看。
```sh
sh />corefile_count
core file count : 1
sh />
```

使用`corefile_dump`命令打印出coredump文件，同样是hex字符串格式。`corefile_dump`命令的参数为coredump文件索引，`0`为距现在最久的。
```sh
sh />corefile_dump 0
core file (idx : 0) {
7f454c4601010161000000000000000004002800 ...
}
crc32 : 43930a92
sh />
```

解析coredump文件的流程同上。
