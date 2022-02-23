/**
 ***********************************************************************************************************************
 * Copyright (c) 2021, China Mobile Communications Group Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with 
 * the License. You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License is distributed on 
 * an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the License for the 
 * specific language governing permissions and limitations under the License.
 *
 * @file        onetls_errcode.h
 *
 * @brief       onetls_errcode header file.
 *
 * @revision
 * Date         Author          Notes
 * 2021-03-16   OneOS Team      First Version.
 ***********************************************************************************************************************
 */

#ifndef __ONETLS_ERRCODE_H__
#define __ONETLS_ERRCODE_H__

#define ONETLS_ERR_SYS_BASE        0x00001000
#define ONETLS_ERR_SOCKET_BASE     0x00002000
#define ONETLS_ERR_EXTENSION_BASE  0x00003000
#define ONETLS_ERR_HANDSHAKE_BASE  0x00004000
#define ONETLS_ERR_CONFIG_BASE     0x00005000

enum {
    ONETLS_SUCCESS = 0,                             // 通用成功
    ONETLS_FAIL,                                    // 通用失败
    ONETLS_INVALID_PARA,                            // 通用参数错误
    ONETLS_NULL_PTR,                                // 空指针
    ONETLS_INNER_SYSERR,                            // 内部系统错误
    ONETLS_MALLOC_FAIL,                             // 内存申请失败

    // 系统错误
    ONETLS_ERR_SYS              = ONETLS_ERR_SYS_BASE,
    ONETLS_SYS_TOO_MANY_CACHE,                      // 缓存的报文过多
    ONETLS_SYS_INVALID_STATE,                       // 当前系统状态不对
    ONETLS_SYS_UNEXPECTED_MESSAGE,                  // 收到不是想要的消息

    // 套接字错误
    ONETLS_ERR_SOCKET           = ONETLS_ERR_SOCKET_BASE,
    ONETLS_SOCKET_FAIL,                             // socket操作时出现了异常
    ONETLS_SOCKET_TIMEOUT,                          // socket超时
    ONETLS_SOCKET_TRYAGAIN,                         // socket非阻塞情况下，返回了重试，外部拿到该错误码之后，待socket可操作时，重新进入
    ONETLS_SOCKET_RECV_FAIL,                        // socket读取失败
    ONETLS_SOCKET_SEND_FAIL,                        // socket写入失败
    ONETLS_SOCKET_CLOSED,                           // 远端关闭了socket
    ONETLS_SOCKET_BAD_RECORD_LEN,                   // 报文长度非法
    ONETLS_SOCKET_BAD_RECORD,                       // 报文非法
    ONETLS_SOCKET_FUTURE_RECORD,                    // 未来的分片。先缓存
    
    // 扩展错误
    ONETLS_ERR_EXTENSION        = ONETLS_ERR_EXTENSION_BASE,
    ONETLS_EXT_PACKET_TOO_SMALL,                    // 缓冲区空间不足
    ONETLS_EXT_WRONG_LEN,                           // 扩展携带的长度不正确
    ONETLS_EXT_WRONG_VER,                           // 版本不正确
    ONETLS_EXT_NO_KEY_SHARE,                        // 没有找到keyshare
    ONETLS_EXT_KEY_SHARE_MISS,                      // keyshare前后不一致
    ONETLS_EXT_COOKIE_RECV,                         // 收到了cookie
    ONETLS_EXT_UNKNOWN,                             // 不认识的扩展
    ONETLS_EXT_NOT_NEED,                            // 不应该存在的
    ONETLS_EXT_REPEAT,                              // 有重复的扩展
    ONETLS_EXT_INVALID_PARAM,                       // 参数错误

    // 握手错误
    ONETLS_ERR_HANDSHAKE        = ONETLS_ERR_HANDSHAKE_BASE,
    ONETLS_HANDSHAKE_REPEAT_GEN_KEY_SHARE,          // 重复生成keyshare
    ONETLS_HANDSHAKE_GEN_KEY_SHARE_FAILD,           // 派生keyshare失败
    ONETLS_HANDSHAKE_NEED_A_PSK,                    // 缺少一个psk
    ONETLS_HANDSHAKE_GET_PSK_KEY,                   // 获取key时报错
    ONETLS_HANDSHAKE_GET_CIPHER,                    // 获取算法时报错
    ONETLS_HANDSHAKE_EARLY_DATA,                    // 不能发送early data
    ONETLS_HANDSHAKE_CACHE_TOO_MUCH,                // 缓存的报文太多了
    ONETLS_HANDSHAKE_NEED_DISCARD,                  // 需要丢弃当前分片
    ONETLS_HANDSHAKE_BAD_RECORD,                    // 报文不正确
    ONETLS_HANDSHAKE_NO_CIPHER_MATCH,               // 算法不配套

    // 配置错误
    ONETLS_ERR_CONFIG           = ONETLS_ERR_CONFIG_BASE,
    ONETLS_CONFIG_INVALID_PARA,                     // 输入无效参数
    ONETLS_CONFIG_INVALID_MTU,                      // 配置的mtu值非法
    ONETLS_CONFIG_INVALID_SEM,                      // 非法得锁索引    
    ONETLS_CONFIG_INVALID_PSK,                      // psk配置非法
    ONETLS_CONFIG_INVALID_PSK_HINT,                 // psk hint配置非法
    ONETLS_CONFIG_INVALID_TICKET,                   // ticket配置非法
    ONETLS_CONFIG_NOT_SUPPORT,                      // 不支持
};

#endif
