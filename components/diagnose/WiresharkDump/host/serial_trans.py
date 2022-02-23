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

import sys
import serial
import serial.threaded
import time

from threading import Thread, Semaphore, Timer
from ctypes import *
from frame_log import FrameLog
from frame_online import FrameOnline
from frame_type import *

# ./tcp_serial_redirect.py COM11 115200 -P 2217
sem = Semaphore(0)
cmd_strs = [None for i in range(16)]
cmd_strs_w = 0
cmd_strs_r = 0

c_print_socket = None

class FrameRx:
    def __init__(self, FrameDeal):
        self.frame_data = bytearray(0x10000)
        self.frame_cnt = 0
        self.frame_len = 0

        self.sem = Semaphore(1)
        self.rx_timer = None
        self.FrameDeal = FrameDeal

    def rx_timeout(self):
        datas_p = 0
        frame_p = 0

        self.sem.acquire()

        self.rx_timer = None
        if not self.frame_cnt:
            self.sem.release()
            return

        datas = self.frame_data[0:self.frame_cnt]
        datas_len = len(datas)

        while(True):
            data = datas[datas_p]
            datas_p += 1

            if 0 == self.frame_cnt:
                if bytes([0xAA])[0] == data:
                    self.frame_data[self.frame_cnt] = data
                    self.frame_cnt += 1
                    frame_p = datas_p-1
            elif 1 == self.frame_cnt:
                self.frame_data[self.frame_cnt] = data
                self.frame_len = int(data)
                self.frame_cnt += 1
            elif 2 == self.frame_cnt:
                self.frame_data[self.frame_cnt] = data
                self.frame_len = int(data) * 0x100+self.frame_len
                self.frame_cnt += 1
            elif self.frame_cnt < (self.frame_len + 2):
                self.frame_data[self.frame_cnt] = data
                self.frame_cnt += 1
            elif self.frame_cnt == (self.frame_len + 2):
                if bytes([0x55])[0] == data:
                    self.FrameDeal.frame_deal(self.frame_data[3:self.frame_cnt])
                    self.frame_cnt = 0
                    self.frame_len = 0
                else:
                    self.frame_cnt = 0
                    self.frame_len = 0
                    # we should re-deal the data in 'self.frame_data[1:]'
                    datas_p = frame_p + 1

            # the last one has been dealt
            if datas_len == datas_p:
                break

        self.frame_cnt = 0
        self.frame_len = 0

        self.sem.release()

    # redeal
    # frame format:
    # |  1B  |  2B  |  4B  |  1B  |  NB  |  1B  |
    # | 0xAA | len  | time | dir  | data | 0x55 |
    def data_deal(self, datas):
        datas_p = 0
        frame_p = 0
        datas_len = len(datas)

        self.sem.acquire()

        if self.rx_timer:
            self.rx_timer.cancel()
            self.rx_timer = None

        while(True):
            data = datas[datas_p]
            datas_p += 1

            if 0 == self.frame_cnt:
                if bytes([0xAA])[0] == data:
                    self.frame_data[self.frame_cnt] = data
                    self.frame_cnt += 1
                    frame_p = datas_p-1
            elif 1 == self.frame_cnt:
                self.frame_data[self.frame_cnt] = data
                self.frame_len = int(data)
                self.frame_cnt += 1
            elif 2 == self.frame_cnt:
                self.frame_data[self.frame_cnt] = data
                self.frame_len = int(data) * 0x100+self.frame_len
                self.frame_cnt += 1
            elif self.frame_cnt < (self.frame_len + 2):
                self.frame_data[self.frame_cnt] = data
                self.frame_cnt += 1
            elif self.frame_cnt == (self.frame_len + 2):
                if bytes([0x55])[0] == data:
                    self.FrameDeal.frame_deal(self.frame_data[3:self.frame_cnt])
                    self.frame_cnt = 0
                    self.frame_len = 0
                else:
                    self.frame_cnt = 0
                    self.frame_len = 0
                    # we should re-deal the data in 'self.frame_data[1:]'
                    datas_p = frame_p + 1

            # the last one has been dealt
            if datas_len == datas_p:
                break

        # the datas in same frame couldn't be sent with the time interval greater than 10ms
        if self.frame_cnt:
            self.rx_timer = Timer(0.01, self.rx_timeout)
            self.rx_timer.start()

        self.sem.release()

        return


class SerialRX(serial.threaded.Protocol):
    """serial->socket"""

    def __init__(self, FrameDeal):
        self.socket = None
        self.frame_rx = FrameRx(FrameDeal)

    def __call__(self):
        return self

    def data_received(self, data):
        self.frame_rx.data_deal(data)


if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(
        description='Simple Serial RX.')

    parser.add_argument(
        'SERIALPORT',
        help="serial port name")

    parser.add_argument(
        'BAUDRATE',
        type=int,
        nargs='?',
        help='set baud rate, default: %(default)s',
        default=9600)

    group = parser.add_argument_group('serial port')

    group.add_argument(
        "--bytesize",
        choices=[5, 6, 7, 8],
        type=int,
        help="set bytesize, one of {5 6 7 8}, default: 8",
        default=8)

    group.add_argument(
        "--parity",
        choices=['N', 'E', 'O', 'S', 'M'],
        type=lambda c: c.upper(),
        help="set parity, one of {N E O S M}, default: N",
        default='N')

    group.add_argument(
        "--stopbits",
        choices=[1, 1.5, 2],
        type=float,
        help="set stopbits, one of {1 1.5 2}, default: 1",
        default=1)

    group.add_argument(
        '--rtscts',
        action='store_true',
        help='enable RTS/CTS flow control (default off)',
        default=False)

    group.add_argument(
        '--xonxoff',
        action='store_true',
        help='enable software flow control (default off)',
        default=False)
        
    group = parser.add_argument_group('frame deal')

    group.add_argument(
        "--deal",
        choices=['LOG', 'ONLINE'],
        type=lambda c: c.upper(),
        help="set way to deal frame, one of {LOG ONLINE}, default: ONLINE",
        default='ONLINE')

    group.add_argument(
        "--type",
        choices=['ETHERNET', 'BLUETOOTH_HCI_H4_WITH_PHDR'],
        type=lambda c: c.upper(),
        help="set frame type, one of {ETHERNET BLUETOOTH_HCI_H4_WITH_PHDR}, default: ETHERNET",
        default='ETHERNET')

    args = parser.parse_args()

    # connect to serial port
    ser = serial.serial_for_url(args.SERIALPORT, do_not_open=True)
    ser.baudrate = args.BAUDRATE
    ser.bytesize = args.bytesize
    ser.parity = args.parity
    ser.stopbits = args.stopbits
    ser.rtscts = args.rtscts
    ser.xonxoff = args.xonxoff

    try:
        ser.open()
    except serial.SerialException as e:
        sys.stderr.write(
            'Could not open serial port {}: {}\n'.format(ser.name, e))
        sys.exit(1)

    # Read all bytes currently available in the buffer of the OS to clear serial.
    ser.read_all()

    if 'LOG' == args.deal:
        ser_rx = SerialRX(FrameLog())
    elif 'ONLINE' == args.deal:
        if 'ETHERNET' == args.type:
            frame_type = EthernetType()
        elif 'BLUETOOTH_HCI_H4_WITH_PHDR' == args.type:
            frame_type = BluetoothHciH4WithPhdrType()
        ser_rx = SerialRX(FrameOnline(frame_type))
    serial_worker = serial.threaded.ReaderThread(ser, ser_rx)
    serial_worker.start()

    try:
        intentional_exit = False
        while True:
            try:
                while True:
                    time.sleep(1)
            except KeyboardInterrupt:
                intentional_exit = True
                raise
            finally:
                ser_rx.frame_rx.FrameDeal.close()
                sys.stderr.write('Disconnected\n')
                if not intentional_exit:
                    # intentional delay on reconnection as client
                    time.sleep(5)
    except KeyboardInterrupt:
        pass

    sys.stderr.write('\n--- exit ---\n')
    serial_worker.stop()
