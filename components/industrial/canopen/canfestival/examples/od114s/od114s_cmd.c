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
 * @file        od114s_cmd.c
 *
 * @brief
 *
 * @revision
 * Date         Author          Notes
 * 2021-08-16   OneOS Team      First Version
 ***********************************************************************************************************************
 */
#include <oneos_config.h>
#ifdef OS_USING_CANFESTIVAL
#include "cf_canfestival.h"
#include "shell.h"
#include <drv_cfg.h>
#include <os_types.h>

static CO_Data *get_node_od(UNS8 nodeId)
{
    UNS8      i, num;
    CO_Data **d = get_canopen_nodelist();
    num         = get_canopen_nodenum();
    for (i = 0; i < num; i++)
    {
        if (nodeId == getNodeId(d[i]))
        {
            return d[i];
        }
    }
    return NULL;
}

int od114s_run(int n, char **arvg)
{
    int nodeId = 4;
    int obj_data;
    CO_Data *d = get_node_od(4);
    if(!d) return -1;
    masterSendNMTstateChange(d,0, NMT_Reset_Node);
    os_task_msleep(500);
    obj_data = 0x3;
    writeNetworkDictCallBack(d, nodeId, 0x6060, 0x00, 1, 2, &obj_data, NULL, 0);
    os_task_msleep(100);
    obj_data = 0xF;
    writeNetworkDictCallBack(d, nodeId, 0x6040, 0x00, 2, 6, &obj_data, NULL, 0);
    os_task_msleep(100);
    obj_data = 0x100000;
    writeNetworkDictCallBack(d, nodeId, 0x60ff, 0x00, 4, 4, &obj_data, NULL, 0);
    return 0;
}
SH_CMD_EXPORT(od114s_run, od114s_run, "od114s_run");

int cf_cmd(int n, char **arvg)
{
    int               nodeId, index, subindex;
    int               obj_size, obj_datatype, obj_data;
    int               i, tmp[10];
    CO_Data *         d;
    UNS32             errorCode;
    const indextable *ptrTable;
    ODCallback_t *    Callback;
    const char *const cmd_str = arvg[1];
    const char *      help[]  = {
        "\r\n"
        "cf_cmd send Node index subindex data [size] [datatype] \r\n"
        "cf_cmd read Node index subindex [datatype] \r\n"
        "param note:\r\n",
        "   index:hex input[0x0000 - 0xFFFF]  subindex:hex input[0x00-0xFF]\r\n",
        "   data:hex input[0x0-0xFFFFFFFF]\r\n",
        "   size:[1,2,3,4] \r\n",
        "   datatype:\r\n",
        "         boolean           0x01, octet_string      0x0A, int24    0x10,uint56    0x1A\r\n",
        "         int8              0x02, unicode_string    0x0B, real64   0x11,uint64    0x1B\r\n",
        "         int16             0x03, time_of_day       0x0C, int40    0x12\r\n",
        "         int32             0x04, time_difference   0x0D, int48    0x13\r\n",
        "         uint8             0x05, domain            0x0F, int56    0x14\r\n",
        "         uint16            0x06,                       , int64    0x15\r\n",
        "         uint32            0x07,                       , uint24   0x16\r\n",
        "         real32            0x08,                       , uint40   0x18\r\n",
        "         visible_string    0x09,                       , uint48   0x19\r\n",
    };
    if (n < 4 || n > 10)
        goto _exit;

    for (i = 0; i < n; i++)
    {
        sscanf(arvg[i], "%x", &tmp[i]);
    }
    nodeId = tmp[2];
    index  = tmp[3];

    d = get_node_od(nodeId);
    if (d == NULL)
    {
        os_kprintf("not found Node: %2.2x\r\n", nodeId);
        return 0;
    }

    ptrTable = (*d->scanIndexOD)(index, &errorCode, &Callback);
    if (!ptrTable)
    {
        os_kprintf("unknow od index:%x\r\n", index);
    }

    if (!strcmp(cmd_str, "send") && (n >= 6))
    {
        subindex = tmp[4];
        obj_data = tmp[5];
        if (ptrTable)
        {
            obj_size     = ptrTable->pSubindex[subindex].size;
            obj_datatype = ptrTable->pSubindex[subindex].bDataType;
        }
        else if (n >= 8)
        {
            obj_size     = tmp[6];
            obj_datatype = tmp[7];
        }
        else
        {
            os_kprintf("write missing parameter\r\n");
            return -1;
        }
        os_kprintf("send Node:%2.2x index=%x sub=%x size=%x datatype=%02x\r\n",
                   nodeId,
                   index,
                   subindex,
                   obj_size,
                   obj_datatype);
        writeNetworkDictCallBack(d, nodeId, index, subindex, obj_size, obj_datatype, &obj_data, NULL, 0);
        return 0;
    }
    else if (!strcmp(cmd_str, "read") && (n >= 5))
    {
        subindex = tmp[4];
        if (ptrTable)
        {
            obj_datatype = ptrTable->pSubindex[subindex].bDataType;
        }
        else if (n >= 6)
        {
            obj_datatype = tmp[5];
        }
        else
        {
            os_kprintf("read missing parameter\r\n");
            return -1;
        }
        os_kprintf("read Node:%2.2x index=%x sub=%x datatype=%02x\r\n", nodeId, index, subindex, obj_datatype);
        readNetworkDictCallback(d, nodeId, index, subindex, obj_datatype, NULL, 0);
        return 0;
    }
_exit:
    for (int i = 0; i < sizeof(help) / sizeof(help[0]); i++)
    {
        os_kprintf("%s", help[i]);
    }
    return -1;
}
SH_CMD_EXPORT(cf_cmd, cf_cmd, "cf_cmd send/read ");
#endif
