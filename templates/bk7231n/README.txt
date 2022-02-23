1.因文件太多scons工具无法编译链接，现调整将beken72XX_HAL目录编译成库libbeken_sdk_gcc.a,
若该文件夹下无修改，使用scons编译；若修改了此文件，则需要执行python make.py来编译，用python make.py clean来clean文件。

2.代码上传时，注意尽量不要修改oneos_config.h和.config文件，保证只有基本通信的代码，
功能模块开启，由客户自己需求决定。
