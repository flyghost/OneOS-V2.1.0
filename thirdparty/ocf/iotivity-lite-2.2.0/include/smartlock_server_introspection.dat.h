/*
#    copyright 2019-2021 Open Interconnect Consortium, Inc. All rights reserved.
#    Redistribution and use in source and binary forms, with or without modification,
#    are permitted provided that the following conditions are met:
#    1.  Redistributions of source code must retain the above copyright notice,
#        this list of conditions and the following disclaimer.
#    2.  Redistributions in binary form must reproduce the above copyright notice,
#        this list of conditions and the following disclaimer in the documentation and/or other materials provided
#        with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE OPEN INTERCONNECT CONSORTIUM, INC. "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE OR
#    WARRANTIES OF NON-INFRINGEMENT, ARE DISCLAIMED. IN NO EVENT SHALL THE OPEN INTERCONNECT CONSORTIUM, INC. OR
#    CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
#    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
#    OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
#    EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef INTROSPECTION_INCLUDE_H
#define INTROSPECTION_INCLUDE_H

#define introspection_data_size 1748  /* size of the CBOR */
unsigned char introspection_data[] = {
  0xbf, 0x67, 0x73, 0x77, 0x61, 0x67, 0x67, 0x65, 0x72, 0x63, 0x32, 0x2e,
  0x30, 0x64, 0x69, 0x6e, 0x66, 0x6f, 0xbf, 0x65, 0x74, 0x69, 0x74, 0x6c,
  0x65, 0x6a, 0x53, 0x6d, 0x61, 0x72, 0x74, 0x20, 0x4c, 0x6f, 0x63, 0x6b,
  0x67, 0x76, 0x65, 0x72, 0x73, 0x69, 0x6f, 0x6e, 0x63, 0x31, 0x2e, 0x30,
  0xff, 0x67, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x65, 0x73, 0x9f, 0x64, 0x68,
  0x74, 0x74, 0x70, 0xff, 0x68, 0x63, 0x6f, 0x6e, 0x73, 0x75, 0x6d, 0x65,
  0x73, 0x9f, 0x70, 0x61, 0x70, 0x70, 0x6c, 0x69, 0x63, 0x61, 0x74, 0x69,
  0x6f, 0x6e, 0x2f, 0x6a, 0x73, 0x6f, 0x6e, 0xff, 0x68, 0x70, 0x72, 0x6f,
  0x64, 0x75, 0x63, 0x65, 0x73, 0x9f, 0x70, 0x61, 0x70, 0x70, 0x6c, 0x69,
  0x63, 0x61, 0x74, 0x69, 0x6f, 0x6e, 0x2f, 0x6a, 0x73, 0x6f, 0x6e, 0xff,
  0x65, 0x70, 0x61, 0x74, 0x68, 0x73, 0xbf, 0x6c, 0x2f, 0x6c, 0x6f, 0x63,
  0x6b, 0x5f, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73, 0xbf, 0x63, 0x67, 0x65,
  0x74, 0xbf, 0x6a, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65, 0x74, 0x65, 0x72,
  0x73, 0x9f, 0xbf, 0x64, 0x24, 0x72, 0x65, 0x66, 0x76, 0x23, 0x2f, 0x70,
  0x61, 0x72, 0x61, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x73, 0x2f, 0x69, 0x6e,
  0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65, 0xff, 0xff, 0x69, 0x72, 0x65,
  0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x73, 0xbf, 0x63, 0x32, 0x30, 0x30,
  0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f,
  0x6e, 0x60, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0xbf, 0x64, 0x24,
  0x72, 0x65, 0x66, 0x78, 0x18, 0x23, 0x2f, 0x64, 0x65, 0x66, 0x69, 0x6e,
  0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f, 0x4c, 0x6f, 0x63, 0x6b, 0x53,
  0x74, 0x61, 0x74, 0x75, 0x73, 0xff, 0xff, 0xff, 0xff, 0x64, 0x70, 0x6f,
  0x73, 0x74, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74,
  0x69, 0x6f, 0x6e, 0x60, 0x6a, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65, 0x74,
  0x65, 0x72, 0x73, 0x9f, 0xbf, 0x64, 0x24, 0x72, 0x65, 0x66, 0x76, 0x23,
  0x2f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x73, 0x2f,
  0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65, 0xff, 0xbf, 0x64,
  0x6e, 0x61, 0x6d, 0x65, 0x64, 0x62, 0x6f, 0x64, 0x79, 0x62, 0x69, 0x6e,
  0x64, 0x62, 0x6f, 0x64, 0x79, 0x68, 0x72, 0x65, 0x71, 0x75, 0x69, 0x72,
  0x65, 0x64, 0xf5, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0xbf, 0x64,
  0x24, 0x72, 0x65, 0x66, 0x78, 0x18, 0x23, 0x2f, 0x64, 0x65, 0x66, 0x69,
  0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f, 0x4c, 0x6f, 0x63, 0x6b,
  0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0xff, 0xff, 0xff, 0x69, 0x72, 0x65,
  0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x73, 0xbf, 0x63, 0x32, 0x30, 0x30,
  0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f,
  0x6e, 0x60, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0xbf, 0x64, 0x24,
  0x72, 0x65, 0x66, 0x78, 0x18, 0x23, 0x2f, 0x64, 0x65, 0x66, 0x69, 0x6e,
  0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f, 0x4c, 0x6f, 0x63, 0x6b, 0x53,
  0x74, 0x61, 0x74, 0x75, 0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6e, 0x2f,
  0x6c, 0x6f, 0x63, 0x6b, 0x5f, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72,
  0x64, 0xbf, 0x63, 0x67, 0x65, 0x74, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63,
  0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x60, 0x6a, 0x70, 0x61, 0x72,
  0x61, 0x6d, 0x65, 0x74, 0x65, 0x72, 0x73, 0x9f, 0xbf, 0x64, 0x24, 0x72,
  0x65, 0x66, 0x76, 0x23, 0x2f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65, 0x74,
  0x65, 0x72, 0x73, 0x2f, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63,
  0x65, 0xff, 0xff, 0x69, 0x72, 0x65, 0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65,
  0x73, 0xbf, 0x63, 0x32, 0x30, 0x30, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63,
  0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x60, 0x66, 0x73, 0x63, 0x68,
  0x65, 0x6d, 0x61, 0xbf, 0x64, 0x24, 0x72, 0x65, 0x66, 0x76, 0x23, 0x2f,
  0x64, 0x65, 0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f,
  0x50, 0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0xff, 0xff, 0xff, 0xff,
  0x64, 0x70, 0x6f, 0x73, 0x74, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72,
  0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x60, 0x6a, 0x70, 0x61, 0x72, 0x61,
  0x6d, 0x65, 0x74, 0x65, 0x72, 0x73, 0x9f, 0xbf, 0x64, 0x24, 0x72, 0x65,
  0x66, 0x76, 0x23, 0x2f, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65, 0x74, 0x65,
  0x72, 0x73, 0x2f, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65,
  0xff, 0xbf, 0x64, 0x6e, 0x61, 0x6d, 0x65, 0x64, 0x62, 0x6f, 0x64, 0x79,
  0x62, 0x69, 0x6e, 0x64, 0x62, 0x6f, 0x64, 0x79, 0x68, 0x72, 0x65, 0x71,
  0x75, 0x69, 0x72, 0x65, 0x64, 0xf5, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d,
  0x61, 0xbf, 0x64, 0x24, 0x72, 0x65, 0x66, 0x76, 0x23, 0x2f, 0x64, 0x65,
  0x66, 0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f, 0x50, 0x61,
  0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0xff, 0xff, 0xff, 0x69, 0x72, 0x65,
  0x73, 0x70, 0x6f, 0x6e, 0x73, 0x65, 0x73, 0xbf, 0x63, 0x32, 0x30, 0x30,
  0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f,
  0x6e, 0x60, 0x66, 0x73, 0x63, 0x68, 0x65, 0x6d, 0x61, 0xbf, 0x64, 0x24,
  0x72, 0x65, 0x66, 0x76, 0x23, 0x2f, 0x64, 0x65, 0x66, 0x69, 0x6e, 0x69,
  0x74, 0x69, 0x6f, 0x6e, 0x73, 0x2f, 0x50, 0x61, 0x73, 0x73, 0x77, 0x6f,
  0x72, 0x64, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x6b, 0x64, 0x65, 0x66,
  0x69, 0x6e, 0x69, 0x74, 0x69, 0x6f, 0x6e, 0x73, 0xbf, 0x6a, 0x4c, 0x6f,
  0x63, 0x6b, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0xbf, 0x64, 0x74, 0x79,
  0x70, 0x65, 0x66, 0x6f, 0x62, 0x6a, 0x65, 0x63, 0x74, 0x6a, 0x70, 0x72,
  0x6f, 0x70, 0x65, 0x72, 0x74, 0x69, 0x65, 0x73, 0xbf, 0x62, 0x72, 0x74,
  0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f,
  0x6e, 0x6d, 0x52, 0x65, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x54,
  0x79, 0x70, 0x65, 0x65, 0x69, 0x74, 0x65, 0x6d, 0x73, 0xbf, 0x69, 0x6d,
  0x61, 0x78, 0x4c, 0x65, 0x6e, 0x67, 0x74, 0x68, 0x18, 0x40, 0x64, 0x74,
  0x79, 0x70, 0x65, 0x66, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0x64, 0x65,
  0x6e, 0x75, 0x6d, 0x9f, 0x71, 0x6f, 0x69, 0x63, 0x2e, 0x72, 0x2e, 0x6c,
  0x6f, 0x63, 0x6b, 0x2e, 0x73, 0x74, 0x61, 0x74, 0x75, 0x73, 0xff, 0xff,
  0x67, 0x64, 0x65, 0x66, 0x61, 0x75, 0x6c, 0x74, 0x9f, 0x71, 0x6f, 0x69,
  0x63, 0x2e, 0x72, 0x2e, 0x6c, 0x6f, 0x63, 0x6b, 0x2e, 0x73, 0x74, 0x61,
  0x74, 0x75, 0x73, 0xff, 0x68, 0x6d, 0x69, 0x6e, 0x49, 0x74, 0x65, 0x6d,
  0x73, 0x01, 0x6b, 0x75, 0x6e, 0x69, 0x71, 0x75, 0x65, 0x49, 0x74, 0x65,
  0x6d, 0x73, 0xf5, 0x68, 0x72, 0x65, 0x61, 0x64, 0x4f, 0x6e, 0x6c, 0x79,
  0xf5, 0x64, 0x74, 0x79, 0x70, 0x65, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79,
  0xff, 0x62, 0x69, 0x66, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69,
  0x70, 0x74, 0x69, 0x6f, 0x6e, 0x78, 0x30, 0x54, 0x68, 0x65, 0x20, 0x4f,
  0x43, 0x46, 0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65,
  0x20, 0x73, 0x65, 0x74, 0x20, 0x73, 0x75, 0x70, 0x70, 0x6f, 0x72, 0x74,
  0x65, 0x64, 0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x52,
  0x65, 0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x65, 0x69, 0x74, 0x65, 0x6d,
  0x73, 0xbf, 0x64, 0x65, 0x6e, 0x75, 0x6d, 0x9f, 0x6f, 0x6f, 0x69, 0x63,
  0x2e, 0x69, 0x66, 0x2e, 0x62, 0x61, 0x73, 0x65, 0x6c, 0x69, 0x6e, 0x65,
  0x68, 0x6f, 0x69, 0x63, 0x2e, 0x69, 0x66, 0x2e, 0x61, 0xff, 0x64, 0x74,
  0x79, 0x70, 0x65, 0x66, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0xff, 0x68,
  0x6d, 0x69, 0x6e, 0x49, 0x74, 0x65, 0x6d, 0x73, 0x02, 0x6b, 0x75, 0x6e,
  0x69, 0x71, 0x75, 0x65, 0x49, 0x74, 0x65, 0x6d, 0x73, 0xf5, 0x68, 0x72,
  0x65, 0x61, 0x64, 0x4f, 0x6e, 0x6c, 0x79, 0xf5, 0x64, 0x74, 0x79, 0x70,
  0x65, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79, 0xff, 0x69, 0x6c, 0x6f, 0x63,
  0x6b, 0x53, 0x74, 0x61, 0x74, 0x65, 0xbf, 0x64, 0x65, 0x6e, 0x75, 0x6d,
  0x9f, 0x66, 0x4c, 0x6f, 0x63, 0x6b, 0x65, 0x64, 0x68, 0x55, 0x6e, 0x6c,
  0x6f, 0x63, 0x6b, 0x65, 0x64, 0xff, 0x64, 0x74, 0x79, 0x70, 0x65, 0x66,
  0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0xff, 0xff, 0x68, 0x72, 0x65, 0x71,
  0x75, 0x69, 0x72, 0x65, 0x64, 0x9f, 0x69, 0x6c, 0x6f, 0x63, 0x6b, 0x53,
  0x74, 0x61, 0x74, 0x65, 0xff, 0xff, 0x68, 0x50, 0x61, 0x73, 0x73, 0x77,
  0x6f, 0x72, 0x64, 0xbf, 0x64, 0x74, 0x79, 0x70, 0x65, 0x66, 0x6f, 0x62,
  0x6a, 0x65, 0x63, 0x74, 0x6a, 0x70, 0x72, 0x6f, 0x70, 0x65, 0x72, 0x74,
  0x69, 0x65, 0x73, 0xbf, 0x62, 0x72, 0x74, 0xbf, 0x6b, 0x64, 0x65, 0x73,
  0x63, 0x72, 0x69, 0x70, 0x74, 0x69, 0x6f, 0x6e, 0x6d, 0x52, 0x65, 0x73,
  0x6f, 0x75, 0x72, 0x63, 0x65, 0x20, 0x54, 0x79, 0x70, 0x65, 0x65, 0x69,
  0x74, 0x65, 0x6d, 0x73, 0xbf, 0x69, 0x6d, 0x61, 0x78, 0x4c, 0x65, 0x6e,
  0x67, 0x74, 0x68, 0x18, 0x40, 0x64, 0x74, 0x79, 0x70, 0x65, 0x66, 0x73,
  0x74, 0x72, 0x69, 0x6e, 0x67, 0x64, 0x65, 0x6e, 0x75, 0x6d, 0x9f, 0x73,
  0x6f, 0x69, 0x63, 0x2e, 0x72, 0x2e, 0x6c, 0x6f, 0x63, 0x6b, 0x2e, 0x70,
  0x61, 0x73, 0x73, 0x77, 0x6f, 0x72, 0x64, 0xff, 0xff, 0x67, 0x64, 0x65,
  0x66, 0x61, 0x75, 0x6c, 0x74, 0x9f, 0x73, 0x6f, 0x69, 0x63, 0x2e, 0x72,
  0x2e, 0x6c, 0x6f, 0x63, 0x6b, 0x2e, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f,
  0x72, 0x64, 0xff, 0x68, 0x6d, 0x69, 0x6e, 0x49, 0x74, 0x65, 0x6d, 0x73,
  0x01, 0x6b, 0x75, 0x6e, 0x69, 0x71, 0x75, 0x65, 0x49, 0x74, 0x65, 0x6d,
  0x73, 0xf5, 0x68, 0x72, 0x65, 0x61, 0x64, 0x4f, 0x6e, 0x6c, 0x79, 0xf5,
  0x64, 0x74, 0x79, 0x70, 0x65, 0x65, 0x61, 0x72, 0x72, 0x61, 0x79, 0xff,
  0x62, 0x69, 0x66, 0xbf, 0x6b, 0x64, 0x65, 0x73, 0x63, 0x72, 0x69, 0x70,
  0x74, 0x69, 0x6f, 0x6e, 0x78, 0x30, 0x54, 0x68, 0x65, 0x20, 0x4f, 0x43,
  0x46, 0x20, 0x49, 0x6e, 0x74, 0x65, 0x72, 0x66, 0x61, 0x63, 0x65, 0x20,
  0x73, 0x65, 0x74, 0x20, 0x73, 0x75, 0x70, 0x70, 0x6f, 0x72, 0x74, 0x65,
  0x64, 0x20, 0x62, 0x79, 0x20, 0x74, 0x68, 0x69, 0x73, 0x20, 0x52, 0x65,
  0x73, 0x6f, 0x75, 0x72, 0x63, 0x65, 0x65, 0x69, 0x74, 0x65, 0x6d, 0x73,
  0xbf, 0x64, 0x65, 0x6e, 0x75, 0x6d, 0x9f, 0x6f, 0x6f, 0x69, 0x63, 0x2e,
  0x69, 0x66, 0x2e, 0x62, 0x61, 0x73, 0x65, 0x6c, 0x69, 0x6e, 0x65, 0x68,
  0x6f, 0x69, 0x63, 0x2e, 0x69, 0x66, 0x2e, 0x61, 0xff, 0x64, 0x74, 0x79,
  0x70, 0x65, 0x66, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 0xff, 0x68, 0x6d,
  0x69, 0x6e, 0x49, 0x74, 0x65, 0x6d, 0x73, 0x02, 0x6b, 0x75, 0x6e, 0x69,
  0x71, 0x75, 0x65, 0x49, 0x74, 0x65, 0x6d, 0x73, 0xf5, 0x68, 0x72, 0x65,
  0x61, 0x64, 0x4f, 0x6e, 0x6c, 0x79, 0xf5, 0x64, 0x74, 0x79, 0x70, 0x65,
  0x65, 0x61, 0x72, 0x72, 0x61, 0x79, 0xff, 0x68, 0x70, 0x61, 0x73, 0x73,
  0x77, 0x6f, 0x72, 0x64, 0xbf, 0x64, 0x74, 0x79, 0x70, 0x65, 0x66, 0x73,
  0x74, 0x72, 0x69, 0x6e, 0x67, 0xff, 0xff, 0x68, 0x72, 0x65, 0x71, 0x75,
  0x69, 0x72, 0x65, 0x64, 0x9f, 0x68, 0x70, 0x61, 0x73, 0x73, 0x77, 0x6f,
  0x72, 0x64, 0xff, 0xff, 0xff, 0x6a, 0x70, 0x61, 0x72, 0x61, 0x6d, 0x65,
  0x74, 0x65, 0x72, 0x73, 0xbf, 0x69, 0x69, 0x6e, 0x74, 0x65, 0x72, 0x66,
  0x61, 0x63, 0x65, 0xbf, 0x64, 0x65, 0x6e, 0x75, 0x6d, 0x9f, 0x6f, 0x6f,
  0x69, 0x63, 0x2e, 0x69, 0x66, 0x2e, 0x62, 0x61, 0x73, 0x65, 0x6c, 0x69,
  0x6e, 0x65, 0x68, 0x6f, 0x69, 0x63, 0x2e, 0x69, 0x66, 0x2e, 0x61, 0xff,
  0x62, 0x69, 0x6e, 0x65, 0x71, 0x75, 0x65, 0x72, 0x79, 0x64, 0x6e, 0x61,
  0x6d, 0x65, 0x62, 0x69, 0x66, 0x64, 0x74, 0x79, 0x70, 0x65, 0x66, 0x73,
  0x74, 0x72, 0x69, 0x6e, 0x67, 0xff, 0xff, 0xff
};

#endif /* INTROSPECTION_INCLUDE_H */
