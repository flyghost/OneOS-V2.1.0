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

class FrameType:
    def __init__(self, dlt):
        self.dlt = dlt

    def frame_data_deal(self, datas):
        pass

# LINKTYPE_ETHERNET 	1
class EthernetType(FrameType):
    def __init__(self):
        super().__init__(1)

    def frame_data_deal(self, datas):
        return datas[1:]

# LINKTYPE_BLUETOOTH_HCI_H4_WITH_PHDR 	201
class BluetoothHciH4WithPhdrType(FrameType):
    def __init__(self):
        super().__init__(201)

    def frame_data_deal(self, datas):
        dir = bool(datas[0])

        # the frame contains a 4-byte direction field, in network byte order (big-endian), the low-order bit of which is
        # set if the frame was sent from the host to the controller and clear if the frame was received by the host from
        # the controller, followed by an HCI packet indicator byte, followed by an HCI packet of the specified packet type
        return bytes([0, 0, 0, not dir])+datas[1:]
