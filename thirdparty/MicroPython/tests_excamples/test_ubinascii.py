#test_ubinascii.py
import ubinascii
#16进制、字符转二进制ASCII码
print(ubinascii.hexlify(b"\x00\x01\x02\x03\x04\x05\x06\x07")) # b'0001020304050607'
print(ubinascii.hexlify(b"1234ABCDabcd")) # b'313233344142434461626364'
#二进制ASCII码转字符、十六进制
print(ubinascii.unhexlify(b"313233344142434461626364"))  # b'1234ABCDabcd'
print(ubinascii.unhexlify(b"0001020304050607")) # b'\x00\x01\x02\x03\x04\x05\x06\x07'
#将二进制数据转换为一行用 base64 编码的ASCII字符串
print(ubinascii.a2b_base64(b'aGVsbG8sIE9uZU9T\n')) # b'hello, OneOS'
print(ubinascii.b2a_base64(b"hello, OneOS")) # b'aGVsbG8sIE9uZU9T\n'
#CRC32校验码, 注意，第二种追加字符串的crc32检验，第一个参数表示带追加crc校验码的字符串，后一个参数表示已生成的crc码
print(hex(ubinascii.crc32(b"hello OneOS"))) # 0x3bdae23c
print(hex(ubinascii.crc32(b" OneOS", ubinascii.crc32(b"hello")))) # 0x3bdae23c

