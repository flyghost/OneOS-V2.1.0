# 编译与烧录
iotjs现只支持gcc编译，烧录可以使用jflash，或者gdb+jlink gdb server。
## 使用gdb烧录的步骤
1. 启动jlink gdb server，选择`Target Device`与`Target interface`，`Target Device`根据实际情况选择，`Target interface`一般选择`swd`与`fixed 4000kHz`。
2. 启动gdb。
```shell
D:\code\oneos-2.0\projects\stm32f767-atk-apollo
> ..\..\..\build-mingw\binutils-gdb-arm\arm-none-eabi-gdb.exe ..\..\out\stm32f767-atk-apollo\oneos.elf
```
3. 烧录，输入命令`target remote :2331`连接gdb server，输入`load`进行烧录。
```
GNU gdb (GDB) 11.0.50.20201207-git
Copyright (C) 2020 Free Software Foundation, Inc.
License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
This is free software: you are free to change and redistribute it.
There is NO WARRANTY, to the extent permitted by law.
Type "show copying" and "show warranty" for details.
This GDB was configured as "--host=x86_64-w64-mingw32 --target=arm-none-eabi".
Type "show configuration" for configuration details.
For bug reporting instructions, please see:
<https://www.gnu.org/software/gdb/bugs/>.
Find the GDB manual and other documentation resources online at:
    <http://www.gnu.org/software/gdb/documentation/>.

For help, type "help".
Type "apropos word" to search for commands related to "word"...
Reading symbols from ..\..\out\stm32f767-atk-apollo\oneos.elf...
(gdb) target remote :2331
Remote debugging using :2331
_k_idle_task_entry (arg=0x0) at D:\code\oneos-2.0\kernel\source\os_idle.c:15
15      {
(gdb) load
Loading section .text, size 0xd0c28 lma 0x8000000
Loading section .init, size 0x4 lma 0x80d0c28
Loading section .fini, size 0x4 lma 0x80d0c2c
Loading section .ARM.exidx, size 0x8 lma 0x80d0c30
Loading section .data, size 0xa08 lma 0x80d0c38
Start address 0x08002b14, load size 857664
Transfer rate: 25380 KB/sec, 14787 bytes/write.
(gdb)
```
# 上传js文件到设备上
下面提供一种方法上传文件到设备上，如果有其他方法也可以使用其他方法。
1. 启动python实现的terminal，使用前需要安装`pyserial`。
```
python ../../thirdparty\iotjs\src\platform\oneos\miniterm.py COM22
```
2. 设备开启接收状态
```
sh />recvfiles

```
3. 在键盘上敲击`Ctrl + t`与`Ctrl + u`，并在`--- File to upload:`后输入文件或文件夹路径。
```
sh /tmp>recvfiles

--- File to upload: D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1349.js ---
........
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1349.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1360.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1360.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1570.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1570.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1915.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1915.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1917.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test-issue-1917.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_equal.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_equal.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_fail.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_fail.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_notequal.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_assert_notequal.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_events_emit_error.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_events_emit_error.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_fs_callbacks_called.js ---
............
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_fs_callbacks_called.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_iotjs_runtime_error.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_iotjs_runtime_error.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_iotjs_syntax_error.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_iotjs_syntax_error.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_module_require_invalid_file.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_module_require_invalid_file.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_module_require_path_below_root.js ---
..............
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_module_require_path_below_root.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_exitcode_arg.js ---
........
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_exitcode_arg.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_exitcode_var.js ---
........
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_exitcode_var.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_explicit_exit.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_explicit_exit.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_implicit_exit.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_process_implicit_exit.js sent ---
--- Sending file D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_timers_issue_1353.js ---
.......
--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail\test_timers_issue_1353.js sent ---

--- File D:\code\oneos-2.0\thirdparty\iotjs\test\run_fail sent ---
sh /tmp>
```

# 特别说明
1. iotjs依赖于文件系统，poll接口，socket接口与posix接口。、
2. iotjs在stm32f767-atk-apollo上已测试通过，如果使用的开发板flash与ram资源充足也可以运行。
3. iotjs对flash需求较大，在stm32f767-atk-apollo开发板上测试，需要将优化等级改为O1或更高，否则flash将不够用。
4. 现在iotjs支持的js模块：assert、dns、http、net、stream、buffer、console、events、fs、module、process、timers、dgram、gpio、uart、i2c、pwm。