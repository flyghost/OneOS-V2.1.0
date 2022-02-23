#include <board.h>
#include <fcntl.h>
#include <block/block_device.h>
#include <device.h>
#include <string.h>
#include <shell.h>

#define TEST_FS_PART_NAME       "sd0"

#define BUFF_SIZE               4096

char test_buf[BUFF_SIZE];

int test_block(os_int32_t argc, char **argv)
{
    os_uint32_t i;
    os_device_t *dev;
    os_size_t offset;
    os_size_t count;
    os_size_t ret;
    char *ptr;
    os_uint32_t j;
    os_uint32_t number_base;

    dev = os_device_find(TEST_FS_PART_NAME);
    if (os_device_open(dev) != OS_EOK)
    {
        os_kprintf("Can't create a block device on '%s' partition.", TEST_FS_PART_NAME);
        return -1;
    }

    for (i = 0; i < BUFF_SIZE; i++)
    {
        test_buf[i] = (os_uint32_t)i;
    }

    struct os_blk_geometry geometry;

    memset(&geometry, 0, sizeof(geometry));
    os_device_control(dev, OS_DEVICE_CTRL_BLK_GETGEOME, &geometry);
    os_kprintf("geometry block_size:%d\r\n", geometry.block_size);

    offset = 0;
    count = 1;
    number_base = 0;

    for (j = 0; j < 512; j++, offset++, number_base++)
    {
        memset(test_buf, 0, sizeof(test_buf));
        ptr = &test_buf[0];
        ret = os_device_read_block(dev, offset, test_buf, 1);
        os_kprintf("block read offset:%d count:%d ret:%d\r\n", offset, count, ret);
        for (i = 0; i < geometry.block_size; i++)
        {
            //os_kprintf("%02x ", *ptr++);
        }
        os_kprintf("\r\n");

        memset(test_buf, 0, sizeof(test_buf));
        ptr = &test_buf[0];
        for (i = 0; i < geometry.block_size; i++)
        {
            test_buf[i] = (0xFF & (i + number_base));
        }
        ret = os_device_write_nonblock(dev, offset, test_buf, 1);
        os_kprintf("block write offset:%d count:%d ret:%d\r\n", offset, count, ret);
        for (i = 0; i < geometry.block_size; i++)
        {
            //os_kprintf("%02x ", *ptr++);
        }
        os_kprintf("\r\n");

        memset(test_buf, 0, sizeof(test_buf));
        ptr = &test_buf[0];
        ret = os_device_read_nonblock(dev, offset, test_buf, 1);
        os_kprintf("block read offset:%d count:%d ret:%d\r\n", offset, count, ret);
        for (i = 0; i < geometry.block_size; i++)
        {
            //os_kprintf("%02x ", *ptr);
            if (*ptr != (0xFF & (i + number_base)))
            {
                os_kprintf("ERROR, %d, %d\r\n", (os_uint8_t)*ptr, (0xFF & (i + number_base)));
            }
            ptr++;
        }
        os_kprintf("\r\n");
    }
		
    return 0;
}
SH_CMD_EXPORT(test_block, test_block, "test block read write");

