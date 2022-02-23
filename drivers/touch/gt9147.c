/**
 ***********************************************************************************************************************
 * Copyright (c) 2020, China Mobile Communications Group Co.,Ltd.
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
 * @file        gt9147.c
 *
 * @brief       gt9147
 *
 * @details     gt9147
 *
 * @revision
 * Date         Author          Notes
 * 2020-02-20   OneOS Team      First Version
 ***********************************************************************************************************************
 */

#include <os_task.h>
#include <device.h>
#include <os_clock.h>
#include <i2c.h>
#include <string.h>
#include <stdlib.h>
#include <os_errno.h>
#include <os_memory.h>
#include <misc/pin.h>
#include <string.h>
#include <drv_gpio.h>
#define LOG_TAG "gt9147"
#define DBG_LVL DBG_INFO
#include <drv_log.h>

#include "touch.h"
#include "gt9147.h"

#define GT9147_RST_PIN OS_GT9147_RST_PIN
#define GT9147_IRQ_PIN OS_GT9147_IRQ_PIN

static struct os_i2c_client *gt9147_client;

/* hardware section */
static const os_uint8_t GT9147_CFG_TBL[] =
{
    0X00, 0XE0, 0X01, 0X10, 0X01, 0X05, 0X3C, 0X00, 0X02, 0X08,
    0X1E, 0X08, 0X50, 0X3C, 0X0F, 0X05, 0X00, 0X00, 0XFF, 0X67,
    0X50, 0X00, 0X00, 0X18, 0X1A, 0X1E, 0X14, 0X89, 0X28, 0X0A,
    0X30, 0X2E, 0XBB, 0X0A, 0X03, 0X00, 0X00, 0X02, 0X33, 0X1D,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X32, 0X00, 0X00,
    0X2A, 0X1C, 0X5A, 0X94, 0XC5, 0X02, 0X07, 0X00, 0X00, 0X00,
    0XB5, 0X1F, 0X00, 0X90, 0X28, 0X00, 0X77, 0X32, 0X00, 0X62,
    0X3F, 0X00, 0X52, 0X50, 0X00, 0X52, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X0F,
    0X0F, 0X03, 0X06, 0X10, 0X42, 0XF8, 0X0F, 0X14, 0X00, 0X00,
    0X00, 0X00, 0X1A, 0X18, 0X16, 0X14, 0X12, 0X10, 0X0E, 0X0C,
    0X0A, 0X08, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0X00, 0X29, 0X28, 0X24, 0X22, 0X20, 0X1F, 0X1E, 0X1D,
    0X0E, 0X0C, 0X0A, 0X08, 0X06, 0X05, 0X04, 0X02, 0X00, 0XFF,
    0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00, 0X00,
    0X00, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF, 0XFF,
    0XFF, 0XFF, 0XFF, 0XFF,
};

static os_err_t gt9147_write_reg(struct os_i2c_client *dev, os_uint8_t write_len, os_uint8_t *write_data)
{
    struct os_i2c_msg msgs;

    msgs.addr  = dev->client_addr;
    msgs.flags = OS_I2C_WR;
    msgs.buf   = write_data;
    msgs.len   = write_len;

    if (os_i2c_transfer(dev->bus, &msgs, 1) == 1)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

static os_err_t gt9147_read_regs(struct os_i2c_client *dev,
                                 os_uint8_t           *cmd_buf,
                                 os_uint8_t            cmd_len,
                                 os_uint8_t            read_len,
                                 os_uint8_t           *read_buf)
{
    struct os_i2c_msg msgs[2];

    msgs[0].addr  = dev->client_addr;
    msgs[0].flags = OS_I2C_WR;
    msgs[0].buf   = cmd_buf;
    msgs[0].len   = cmd_len;

    msgs[1].addr  = dev->client_addr;
    msgs[1].flags = OS_I2C_RD;
    msgs[1].buf   = read_buf;
    msgs[1].len   = read_len;

    if (os_i2c_transfer(dev->bus, msgs, 2) == 2)
    {
        return OS_EOK;
    }
    else
    {
        return OS_ERROR;
    }
}

/**
 * This function read the product id
 *
 * @param dev the pointer of device driver structure
 * @param reg the register for gt9xx
 * @param read data len
 * @param read data pointer
 *
 * @return the read status, OS_EOK reprensents  read the value of the register successfully.
 */
static os_err_t gt9147_get_product_id(struct os_i2c_client *dev, os_uint8_t read_len, os_uint8_t *read_data)
{
    os_uint8_t cmd_buf[2];

    cmd_buf[0] = (os_uint8_t)(GT9XX_PRODUCT_ID >> 8);
    cmd_buf[1] = (os_uint8_t)(GT9XX_PRODUCT_ID & 0xff);

    if (gt9147_read_regs(dev, cmd_buf, 2, read_len, read_data) != OS_EOK)
    {
        LOG_EXT_D("read id failed \r\n");

        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t gt9147_get_info(struct os_i2c_client *dev, struct os_touch_info *info)
{
    os_uint8_t opr_buf[7] = {0};
    os_uint8_t cmd_buf[2];

    cmd_buf[0] = (os_uint8_t)(GT9147_CONFIG >> 8);
    cmd_buf[1] = (os_uint8_t)(GT9147_CONFIG & 0xff);

    if (gt9147_read_regs(dev, cmd_buf, 2, 7, opr_buf) != OS_EOK)
    {
        LOG_EXT_D("read id failed \r\n");

        return OS_ERROR;
    }

    info->range_x   = (opr_buf[2] << 8) + opr_buf[1];
    info->range_y   = (opr_buf[4] << 8) + opr_buf[3];
    info->point_num = opr_buf[5] & 0x0f;

    return OS_EOK;
}

static os_err_t gt9147_soft_reset(struct os_i2c_client *dev)
{
    os_uint8_t buf[3];

    buf[0] = (os_uint8_t)(GT9147_COMMAND >> 8);
    buf[1] = (os_uint8_t)(GT9147_COMMAND & 0xFF);
    buf[2] = 0x02;

    if (gt9147_write_reg(dev, 3, buf) != OS_EOK)
    {
        LOG_EXT_D("soft reset gt9147 failed\r\n");
        return OS_ERROR;
    }

    return OS_EOK;
}

static os_err_t gt9147_control(struct os_touch_device *device, int cmd, void *data)
{
    if (cmd == OS_TOUCH_CTRL_GET_ID)
    {
        return gt9147_get_product_id(gt9147_client, 6, data);
    }

    if (cmd == OS_TOUCH_CTRL_GET_INFO)
    {
        return gt9147_get_info(gt9147_client, data);
    }

    os_uint8_t  buf[4];
    os_uint8_t  i = 0;
    os_uint8_t *config;

    config = (os_uint8_t *)os_calloc(1, sizeof(GT9147_CFG_TBL) + GTP_ADDR_LENGTH);

    if (config == OS_NULL)
    {
        LOG_EXT_D("malloc config memory failed\r\n");
        return OS_ERROR;
    }

    config[0] = (os_uint8_t)((GT9147_CONFIG >> 8) & 0xFF); /* config reg */
    config[1] = (os_uint8_t)(GT9147_CONFIG & 0xFF);

    memcpy(&config[2], GT9147_CFG_TBL, sizeof(GT9147_CFG_TBL)); /* config table */

    switch (cmd)
    {
    case OS_TOUCH_CTRL_SET_X_RANGE: /* set x range */
    {
        os_uint16_t x_ran;

        x_ran     = *(os_uint16_t *)data;
        config[4] = (os_uint8_t)(x_ran >> 8);
        config[3] = (os_uint8_t)(x_ran & 0xff);

        break;
    }
    case OS_TOUCH_CTRL_SET_Y_RANGE: /* set y range */
    {
        os_uint16_t y_ran;

        y_ran     = *(os_uint16_t *)data;
        config[6] = (os_uint8_t)(y_ran >> 8);
        config[5] = (os_uint8_t)(y_ran & 0xff);

        break;
    }
    case OS_TOUCH_CTRL_SET_X_TO_Y: /* change x y */
    {
        config[8] = config[8] ^= (1 << 3);
        break;
    }
    case OS_TOUCH_CTRL_SET_MODE: /* change int trig type */
    {
        os_uint16_t trig_type;
        trig_type = *(os_uint16_t *)data;

        switch (trig_type)
        {
#if 0
        case OS_DEVICE_FLAG_INT_RX:
            config[8] &= 0xFC;
            break;
#endif
        case OS_DEVICE_FLAG_RDONLY:
            config[8] &= 0xFC;
            config[8] |= 0x02;
            break;
        default:
            break;
        }
        break;
    }
    default:
    {
        break;
    }
    }

    if (gt9147_write_reg(gt9147_client, sizeof(GT9147_CFG_TBL) + GTP_ADDR_LENGTH, config) != OS_EOK) /* send config */
    {
        LOG_EXT_D("send config failed\r\n");
        return OS_ERROR;
    }

    buf[0] = (os_uint8_t)((GT9147_CHECK_SUM >> 8) & 0xFF);
    buf[1] = (os_uint8_t)(GT9147_CHECK_SUM & 0xFF);
    buf[2] = 0;

    for (i = GTP_ADDR_LENGTH; i < sizeof(GT9147_CFG_TBL) + GTP_ADDR_LENGTH; i++)
        buf[GTP_ADDR_LENGTH] += config[i];

    buf[2] = (~buf[2]) + 1;
    buf[3] = 1;

    gt9147_write_reg(gt9147_client, 4, buf);
    os_free(config);

    return OS_EOK;
}

static os_int16_t            pre_x[GT9147_MAX_TOUCH] = {-1, -1, -1, -1, -1};
static os_int16_t            pre_y[GT9147_MAX_TOUCH] = {-1, -1, -1, -1, -1};
static os_int16_t            pre_w[GT9147_MAX_TOUCH] = {-1, -1, -1, -1, -1};
static os_uint8_t            s_tp_dowm[GT9147_MAX_TOUCH];
static struct os_touch_data *read_data;

static void gt9147_touch_up(void *buf, os_int8_t id)
{
    read_data = (struct os_touch_data *)buf;

    if (s_tp_dowm[id] == 1)
    {
        s_tp_dowm[id]       = 0;
        read_data[id].event = OS_TOUCH_EVENT_UP;
    }
    else
    {
        read_data[id].event = OS_TOUCH_EVENT_NONE;
    }

    read_data[id].timestamp    = os_touch_get_ts();
    read_data[id].width        = pre_w[id];
    read_data[id].x_coordinate = pre_x[id];
    read_data[id].y_coordinate = pre_y[id];
    read_data[id].track_id     = id;

    pre_x[id] = -1; /* last point is none */
    pre_y[id] = -1;
    pre_w[id] = -1;
}

static void gt9147_touch_down(void *buf, os_int8_t id, os_int16_t x, os_int16_t y, os_int16_t w)
{
    read_data = (struct os_touch_data *)buf;

    if (s_tp_dowm[id] == 1)
    {
        read_data[id].event = OS_TOUCH_EVENT_MOVE;
    }
    else
    {
        read_data[id].event = OS_TOUCH_EVENT_DOWN;
        s_tp_dowm[id]       = 1;
    }

    read_data[id].timestamp    = os_touch_get_ts();
    read_data[id].width        = w;
    read_data[id].x_coordinate = x;
    read_data[id].y_coordinate = y;
    read_data[id].track_id     = id;

    pre_x[id] = x; /* save last point */
    pre_y[id] = y;
    pre_w[id] = w;
}

static os_size_t gt9147_read_point(struct os_touch_device *touch, void *buf, os_size_t read_num)
{
    os_uint8_t point_status = 0;
    os_uint8_t touch_num    = 0;
    os_uint8_t write_buf[3];
    os_uint8_t cmd[2];
    os_uint8_t read_buf[8 * GT9147_MAX_TOUCH] = {0};
    os_uint8_t read_index;
    os_int8_t  read_id = 0;
    os_int16_t input_x = 0;
    os_int16_t input_y = 0;
    os_int16_t input_w = 0;

    static os_uint8_t pre_touch                = 0;
    static os_int8_t  pre_id[GT9147_MAX_TOUCH] = {0};

    /* point status register */
    cmd[0] = (os_uint8_t)((GT9147_READ_STATUS >> 8) & 0xFF);
    cmd[1] = (os_uint8_t)(GT9147_READ_STATUS & 0xFF);

    if (gt9147_read_regs(gt9147_client, cmd, 2, 1, &point_status) != OS_EOK)
    {
        LOG_EXT_D("read point failed\r\n");
        read_num = 0;
        goto exit_;
    }

    if (point_status == 0) /* no data */
    {
        read_num = 0;
        goto exit_;
    }

    if ((point_status & 0x80) == 0) /* data is not ready */
    {
        read_num = 0;
        goto exit_;
    }

    touch_num = point_status & 0x0f; /* get point num */

    if (touch_num > GT9147_MAX_TOUCH) /* point num is not correct */
    {
        read_num = 0;
        goto exit_;
    }

    cmd[0] = (os_uint8_t)((GT9147_POINT1_REG >> 8) & 0xFF);
    cmd[1] = (os_uint8_t)(GT9147_POINT1_REG & 0xFF);

    /* read point num is read_num */
    if (gt9147_read_regs(gt9147_client, cmd, 2, read_num * GT9147_POINT_INFO_NUM, read_buf) != OS_EOK)
    {
        LOG_EXT_D("read point failed\r\n");
        read_num = 0;
        goto exit_;
    }

    if (pre_touch > touch_num) /* point up */
    {
        for (read_index = 0; read_index < pre_touch; read_index++)
        {
            os_uint8_t j;

            for (j = 0; j < touch_num; j++) /* this time touch num */
            {
                read_id = read_buf[j * 8] & 0x0F;

                if (pre_id[read_index] == read_id) /* this id is not free */
                    break;

                if (j >= touch_num - 1)
                {
                    os_uint8_t up_id;
                    up_id = pre_id[read_index];
                    gt9147_touch_up(buf, up_id);
                }
            }
        }
    }

    if (touch_num) /* point down */
    {
        os_uint8_t off_set;

        for (read_index = 0; read_index < touch_num; read_index++)
        {
            off_set            = read_index * 8;
            read_id            = read_buf[off_set] & 0x0f;
            pre_id[read_index] = read_id;
            input_x            = read_buf[off_set + 1] | (read_buf[off_set + 2] << 8); /* x */
            input_y            = read_buf[off_set + 3] | (read_buf[off_set + 4] << 8); /* y */
            input_w            = read_buf[off_set + 5] | (read_buf[off_set + 6] << 8); /* size */

            gt9147_touch_down(buf, read_id, input_x, input_y, input_w);
        }
    }
    else if (pre_touch)
    {
        for (read_index = 0; read_index < pre_touch; read_index++)
        {
            gt9147_touch_up(buf, pre_id[read_index]);
        }
    }

    pre_touch = touch_num;

exit_:
    write_buf[0] = (os_uint8_t)((GT9147_READ_STATUS >> 8) & 0xFF);
    write_buf[1] = (os_uint8_t)(GT9147_READ_STATUS & 0xFF);
    write_buf[2] = 0x00;
    gt9147_write_reg(gt9147_client, 3, write_buf);
    return read_num;
}

static struct os_touch_ops touch_ops =
{
    .touch_readpoint = gt9147_read_point,
    .touch_control   = gt9147_control,
};

static int os_hw_gt9147_init(const char *name, struct os_touch_config *cfg)
{
    os_touch_t *touch_device = OS_NULL;

    touch_device = (os_touch_t *)os_calloc(1, sizeof(os_touch_t));

    if (touch_device == OS_NULL)
        return OS_ERROR;

    /* hardware init */
    os_pin_mode(*(os_uint8_t *)cfg->user_data, PIN_MODE_OUTPUT);
    os_pin_mode(cfg->irq_pin->pin, PIN_MODE_OUTPUT);
    os_pin_write(*(os_uint8_t *)cfg->user_data, PIN_LOW);
    os_task_msleep(10);
    os_pin_write(*(os_uint8_t *)cfg->user_data, PIN_HIGH);
    os_task_msleep(10);
    os_pin_mode(cfg->irq_pin->pin, PIN_MODE_INPUT);
    os_task_msleep(100);

    /* interface bus */
    gt9147_client = (struct os_i2c_client *)os_calloc(1, sizeof(struct os_i2c_client));

    gt9147_client->bus = (struct os_i2c_bus_device *)os_device_find(cfg->dev_name);

    if (gt9147_client->bus == OS_NULL)
    {
        LOG_E(DBG_TAG,"Can't find device\r\n");
        return OS_ERROR;
    }

    if (os_device_open((os_device_t *)gt9147_client->bus) != OS_EOK)
    {
        LOG_E(DBG_TAG,"open device failed\r\n");
        return OS_ERROR;
    }

    gt9147_client->client_addr = GT9147_ADDRESS_HIGH;
    gt9147_soft_reset(gt9147_client);

    /* register touch device */
    touch_device->info.type   = OS_TOUCH_TYPE_CAPACITANCE;
    touch_device->info.vendor = OS_TOUCH_VENDOR_GT;
    memcpy(&touch_device->config, cfg, sizeof(struct os_touch_config));
    touch_device->ops = &touch_ops;

    os_hw_touch_register(touch_device, name, OS_NULL);

    LOG_I(DBG_TAG,"touch device gt9147 init success\r\n");

    return OS_EOK;
}

static int os_hw_gt9147_port(void)
{
    struct os_touch_config config;
    os_uint8_t             rst;

    rst                  = GT9147_RST_PIN;
    config.dev_name      = OS_GT9147_I2C_BUS_NAME;
    config.irq_pin->pin  = GT9147_IRQ_PIN;
    config.irq_pin->mode = PIN_MODE_INPUT_PULLDOWN;
    config.user_data     = &rst;

    os_hw_gt9147_init("gt", &config);

    return 0;
}
OS_CMPOENT_INIT(os_hw_gt9147_port);
