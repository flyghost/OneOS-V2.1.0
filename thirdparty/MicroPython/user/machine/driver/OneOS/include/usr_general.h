#ifndef __USR_GENERAL_H__
#define __USR_GENERAL_H__

typedef enum MP_USR_DRIVER_READ_TYPE{
    MP_USR_DRIVER_READ_NONBLOCK = 0,
    MP_USR_DRIVER_READ_BLOCK,
    MP_USR_DRIVER_READ_TYPE_MAX,
} MP_USR_DRIVER_READ_TYPE_T;

typedef enum MP_USR_DRIVER_WRITE_TYPE{
    MP_USR_DRIVER_WRITE_NONBLOCK = 0,
    MP_USR_DRIVER_WRITE_BLOCK,
    MP_USR_DRIVER_WRITE_TYPE_MAX,
} MP_USR_DRIVER_WRITE_TYPE_T;

int mpy_usr_driver_open(const char *dev_name);

int mpy_usr_driver_close(const char *dev_name);

int mpy_usr_driver_read(const char *dev_name, uint32_t offset, 
                                  void *buf, uint32_t bufsize, uint32_t read_type);

int mpy_usr_driver_write(const char *dev_name, uint32_t offset, 
                                        void *buf, uint32_t bufsize, uint32_t write_type);


#endif /* __USR_GENERAL_H__ */
