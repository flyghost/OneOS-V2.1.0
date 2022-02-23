#test_usocket.py
import usocket as socket

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.bind(socket.getaddrinfo("127.0.0.1", 8000)[0][-1])
'''
[6897] E/BSD_SOCKET: OneOS module is not support bind [bind][387]
Traceback (most recent call last):
  File "<stdin>", line 1, in <module>
OSError: 0
'''
s.connect(("110.10.38.128", 28765))#无输出，成功连接
s.close()
'''
[36879] W/esp8266.netconn: Module esp8266 receive close urc data of connect 0 [urc_close_func][486]
[36881] I/esp8266.netconn: Module esp8266 netconn id 0 destroyed [esp8266_netconn_destroy][205]
'''
socket.getaddrinfo("www.baidu.com", 80)[0][-1]#('39.156.66.18', 80)
