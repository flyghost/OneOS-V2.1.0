#!/usr/bin/env python

# /**
#  ***********************************************************************************************************************
#  * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
#  *
#  * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with
#  * the License. You may obtain a copy of the License at
#  *
#  *     http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on
#  * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the
#  * specific language governing permissions and limitations under the License.
#  *
#  * @file        serial-trans.py
#  *
#  * @brief       Receive datas from serial and save them to a file that can be used by wireshark.
#  *
#  * @details
#  *
#  * @revision
#  * Date          Author          Notes
#  * 2021-06-07    OneOS Team      First Version
#  ***********************************************************************************************************************
#  */
import os
import time
import struct
import select
import errno
from ctypes import *
import datetime
import platform
import subprocess

sys = platform.system()
if sys == "Windows":
    import win32pipe
    import win32file

from frame_deal import FreamDeal

class FrameOnline(FreamDeal):
    def __init__(self, frame_type):
        now = datetime.datetime.now()
        self.timestamp_start = int(time.mktime(now.timetuple()))

        try:
            self.sys = platform.system()
            if self.sys == "Windows":
                # open Wireshark, configure pipe interface and start capture (not mandatory, you can also do this manually)
                wireshark_cmd = [
                    'C:\Program Files\Wireshark\Wireshark.exe', r'-i\\.\pipe\wireshark', '-k']
                proc = subprocess.Popen(wireshark_cmd)

                # create the named pipe \\.\wireshark_pipe
                self.pipe = win32pipe.CreateNamedPipe(
                    r'\\.\pipe\wireshark',
                    win32pipe.PIPE_ACCESS_OUTBOUND,
                    win32pipe.PIPE_TYPE_MESSAGE | win32pipe.PIPE_WAIT,
                    1,
                    65536,
                    65536,
                    300,
                    None)

                # connect to pipe
                win32pipe.ConnectNamedPipe(self.pipe, None)
            else:
                wireshark_cmd = ['wireshark', '-k', '-i', '/tmp/sharkfin']
                proc = subprocess.Popen(wireshark_cmd)

                name = "/tmp/sharkfin"
                try:
                    os.mkfifo(name)
                except FileExistsError:
                    pass
                self.out = open(name, 'wb')
                FreamDeal.__init__(self, self.out)
            # go to http://www.tcpdump.org/linktypes.html for more information 
            self.frame_data_deal = frame_type.frame_data_deal
            dlt = frame_type.dlt

            # write header
            header = struct.pack("=IHHiIII",
                                 0xa1b2c3d4,   # magic number
                                 2,            # major version number
                                 4,            # minor version number
                                 0,            # GMT to local correction
                                 0,            # accuracy of timestamps
                                 65535,        # max length of captured packets, in octets
                                 dlt)          # data link type (DLT) - IEEE 802.15.4
            self.write(header)
        except:
            raise

    def close(self):
        if self.sys == "Windows":
            win32file.CloseHandle(self.pipe)
        else:
            self.out.close()

    # send pcap data trough the pipe
    def write(self, datas):
        try:
            if self.sys == "Windows":
                win32file.WriteFile(self.pipe, datas)
                win32file.FlushFileBuffers(self.pipe)
            else:
                self.out.write(datas)
                self.out.flush()
        except Exception as e:
            print(e)


    def frame_deal(self, datas):
        ticks = int(datas[0])+int(datas[1])*0x100 + \
            int(datas[2])*0x10000+int(datas[3])*0x1000000
        tick = ticks % 1000
        secs = int((ticks-tick)/1000)
        second = self.timestamp_start+secs
        microsecond = tick*1000

        packet_data = self.frame_data_deal(datas[4:])
        packet_data_len = len(packet_data)

        # write packet header
        packet_header = struct.pack("=IIII",
                                    second,         # timestamp seconds
                                    microsecond,    # timestamp microseconds
                                    packet_data_len,    # number of octets of packet saved in file
                                    packet_data_len,        # actual length of packet
                                    )
        packet = packet_header + packet_data
        self.write(packet)
