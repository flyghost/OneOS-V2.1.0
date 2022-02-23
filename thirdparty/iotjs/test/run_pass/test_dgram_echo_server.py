import socket
import time

PORT = 6711

server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
address = ("0.0.0.0", PORT)
server_socket.bind(address)

while True:
  try:
      now = time.time()
      receive_data, client = server_socket.recvfrom(1024)
      server_socket.sendto(receive_data, client)
      print(time.strftime('%Y-%m-%d %H:%M:%S',time.localtime(now)))
      print("peer %s,data %s\n" % (client, receive_data))
  except Exception as e:
      print(e)
