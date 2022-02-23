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
from ctypes import *
import datetime
from frame_deal import FreamDeal

class FrameLog(FreamDeal):
    def __init__(self):
        date_now = datetime.datetime.now()
        date_now_str = date_now.strftime("%Y_%m_%d_%H_%M_%S")
        self.log_f = open('wsk_log_'+date_now_str, "w+")
        FreamDeal.__init__(self, self.log_f)
    
    def close(self):
        self.log_f.close()

    def frame_deal(self, datas):
        hci_str = ''

        dir = datas[4]
        if int(dir):
            hci_str += "I \n"
        else:
            hci_str += "O \n"

        ticks = int(datas[0])+int(datas[1])*0x100 + \
            int(datas[2])*0x10000+int(datas[3])*0x1000000
        tick = ticks % 1000
        secs = (ticks-tick)/1000
        sec = secs % 60
        mins = (secs-sec)/60
        min = mins % 60
        hours = (mins-min)/60
        hour = hours % 60
        days = (hours-hour)/24

        date_now = datetime.datetime.now()

        # timestamp: %H:%M:%S.
        hci_str += ("%02d:%02d:%02d.%06d \n") % (int(hour),
                                                 int(min), int(sec), int(tick*1000))

        line = 0
        for i in range(len(datas[5:])):
            data = datas[5+i]
            if (i & 0x0F) == 0:
                hci_str += '%05X0' % (i >> 4)
            hci_str += ' %02X' % data
            if (i & 0x0F) == 0x0F:
                hci_str += '\n'
        hci_str += '\n'
        self.log_f.write(hci_str)