arch_table = {}
arch_table['GD32F10x'] = 'M3'
arch_table['GD32F20x'] = 'M3'
arch_table['GD32E10x'] = 'M4'
arch_table['GD32F30x'] = 'M4'
arch_table['GD32F3x0'] = 'M4'
arch_table['GD32F4xx'] = 'M4'
arch_table['GD32F403'] = 'M4'
arch_table['GD32E50x'] = 'M33'

def get_arch(series):
    if series in arch_table:
        arch = arch_table[series]
        return arch
    else:
        return None


