import sys
sector_size_table = {}
sector_num_table = {}
bank0_size_array  = {}
partition_size_array = {}
sector_size_table['GD32F4xx'] = (16, 64, 128, 16, 64, 128, 256)
sector_num_table ['GD32F4xx'] = ( 4,  1,   7,  4,  1,   7,   4)
bank0_size_array['GD32F4xx'] = 1024
partition_size_array['GD32F4xx'] = {'bootloader':64,'cfg':64, 'app':0, 'download':128}

sector_size_table['GD32F403'] = (  2,   4)
sector_num_table ['GD32F403'] = (256, 640)
bank0_size_array['GD32F403'] = 512
partition_size_array['GD32F403'] = {'bootloader':32, 'cfg':2, 'app':0, 'download':2}

sector_size_table['GD32F3x0'] = (  1,)
sector_num_table ['GD32F3x0'] = (128,)
bank0_size_array['GD32F3x0']  = 128
partition_size_array['GD32F3x0'] = {'bootloader':0, 'cfg':2, 'app':0, 'download':2}


sector_size_table['GD32F30x'] = (  2,   4)
sector_num_table ['GD32F30x'] = (256, 640)
bank0_size_array['GD32F30x']  = 512
partition_size_array['GD32F30x'] = {'bootloader':32, 'cfg':2, 'app':0, 'download':2}

sector_size_table['GD32E10x'] = (  1,)
sector_num_table ['GD32E10x'] = (128,)
bank0_size_array['GD32E10x']  = 512
partition_size_array['GD32E10x'] = {'bootloader':0, 'cfg':2, 'app':0, 'download':2}

sector_size_table['GD32E50x'] = ( 8,)
sector_num_table ['GD32E50x'] = (64,)
bank0_size_array['GD32E50x']  = 512
partition_size_array['GD32E50x'] = {'bootloader':32, 'cfg':8, 'app':0, 'download':8}

sector_size_table['GD32F10x'] = ( 2, 4)
sector_num_table ['GD32F10x'] = (256, 640)
bank0_size_array['GD32F10x']  = 512
partition_size_array['GD32F10x'] = {'bootloader':0, 'cfg':2, 'app':0, 'download':8}

sector_size_table['GD32F20x'] = ( 2, 4)
sector_num_table ['GD32F20x'] = (256, 640)
bank0_size_array['GD32F20x']  = 512
partition_size_array['GD32F20x'] = {'bootloader':0, 'cfg':2, 'app':0, 'download':8}

sector_size_table['6'] = 32
sector_size_table['8'] = 64
sector_size_table['B'] = 128
sector_size_table['C'] = 256
sector_size_table['D'] = 384
sector_size_table['E'] = 512
sector_size_table['F'] = 768
sector_size_table['G'] = 1024
sector_size_table['I'] = 2048
sector_size_table['K'] = 3072


flash_size_table = {}
flash_size_table['GD32F30x'] = {8}

def get_bank0_size_array(series):
    bank0_size =  bank0_size_array[series]
    if bank0_size != None:
        return bank0_size
    else:
        return 0

def get_partition_size(series, partition_name):
    partition_size = partition_size_array[series][partition_name]
    if partition_size != None:
        return partition_size
    else:
        return 0

def get_cur_sector_size_from_offset(series, offset):
    series_sector_size = sector_size_table[series]
    series_sector_num  = sector_num_table[series]
    cur_size = 0
    sector_idx = -1
    for i in range(len(series_sector_size)):
        cur_size += series_sector_size[i] * series_sector_num[i]
        if (cur_size > offset):
            sector_idx = series_sector_size[i]
            break
    return sector_idx

def get_total_sector_num(series):
    
    total_sector_num = 0
    # test = sector_num_table['GD32F4xx']
    # for i in range(len(test)):
    #     total_sector_num += test[i]
    test = sum(sector_num_table[series])
    print(test)
    return 

def get_sector_offset(series):
    series_sector_num = sector_num_table[series]
    series_sector_size = sector_num_table[series]
    cur_size = 0
    for i in range(len(series_sector_num)):
        for j in range(series_sector_num[i]):
            cur_size += series_sector_size[i]
            print('i=%d; j=%d; cur_size = %d\r\n' % (i, j, cur_size))
    return cur_size     

def get_sector_idx_from_offset(series, offset):
    offset
    partition_size_array =  sector_size_table[series][0]
    if partition_size_array != 0:
        return partition_size_array
    else:
        return 0

def get_sector_size_from_addr(series, addr):
    partition_size_array =  sector_size_table[series]
    if partition_size_array != 0:
        return partition_size_array
    else:
        return 0


def get_sector_size_kb(soc):
    flash_size =  flash_size_table[soc[-1]]
    if flash_size != 0:
        return flash_size
    else:
        return 0

def get_size_byte(soc):
    flash_size =  flash_size_table[soc[-1]]
    if flash_size != 0:
        return flash_size * 1024
    else:
        return 0

def get_startaddr(soc):
    return '08000000'

