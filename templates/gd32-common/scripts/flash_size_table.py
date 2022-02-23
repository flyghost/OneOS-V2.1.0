flash_size_table = {}
flash_size_table['4'] = 16
flash_size_table['6'] = 32
flash_size_table['8'] = 64
flash_size_table['B'] = 128
flash_size_table['C'] = 256
flash_size_table['D'] = 384
flash_size_table['E'] = 512
flash_size_table['F'] = 768
flash_size_table['G'] = 1024
flash_size_table['I'] = 2048
flash_size_table['K'] = 3072

def get_size_kb(soc):
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

