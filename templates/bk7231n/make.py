#!/usr/bin/python3

import sys
import os

if len(sys.argv) == 1:
    os.system("scons --buildlib=\"beken_sdk\" -j8")
    os.system("scons -j8")
elif len(sys.argv) == 2 and sys.argv[1] == "clean":
    os.system("scons --buildlib=\"beken_sdk\" -c")
    os.system("scons -c")
else:
    print("param error!")