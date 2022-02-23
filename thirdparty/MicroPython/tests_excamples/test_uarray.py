import uarray as array

a = array.array('B', [1, 2, 3])
print(a, len(a))#array('B', [1, 2, 3]) 3 
i = array.array('I', [1, 2, 3])
print(i, len(i))#array('I', [1, 2, 3]) 3
print(a[0])#1
print(i[-1])#3
a = array.array('l', [-1])
print(len(a), a[0])#1 -1
a1 = array.array('l', [1, 2, 3])
a2 = array.array('L', [1, 2, 3])
print(a2[1])#2
print(a1 == a2)#True

# 空数组
print(len(array.array('h')))#0
print(array.array('i'))#array('i')

# 布尔值数组
print(bool(array.array('i')))#False
print(bool(array.array('i', [1])))#True

# 检查错误类型
print('12' in array.array('B', b'12'))#False
print([] in array.array('B', b'12'))#TypeError

# 不存在的数值类型报错
try:
    array.array('X')
except ValueError:
    print("ValueError")
#ValueError

# 检查数组中的值是否相等
print(bytes(array.array('b', [0x61, 0x62, 0x63])) == b'abc')#True
print(array.array('b', [0x61, 0x62, 0x63]) == b'abc')#True
print(array.array('b', [0x61, 0x62, 0x63]) != b'abc')#False
print(array.array('b', [0x61, 0x62, 0x63]) == b'xyz')#False
print(array.array('b', [0x61, 0x62, 0x63]) != b'xyz')#True
print(b'abc' == array.array('b', [0x61, 0x62, 0x63]))#True
print(b'abc' != array.array('b', [0x61, 0x62, 0x63]))#False
print(b'xyz' == array.array('b', [0x61, 0x62, 0x63]))#False
print(b'xyz' != array.array('b', [0x61, 0x62, 0x63]))#True

class X(array.array):
    pass

print(bytes(X('b', [0x61, 0x62, 0x63])) == b'abc')#True
print(X('b', [0x61, 0x62, 0x63]) == b'abc')#True
print(X('b', [0x61, 0x62, 0x63]) != b'abc')#False
print(X('b', [0x61, 0x62, 0x63]) == array.array('b', [0x61, 0x62, 0x63]))#True
print(X('b', [0x61, 0x62, 0x63]) != array.array('b', [0x61, 0x62, 0x63]))#False