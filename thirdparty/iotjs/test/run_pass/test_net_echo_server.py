import selectors
import socket
import datetime

sel = selectors.DefaultSelector()

def accept(sock, mask):
    conn, addr = sock.accept()  # Should be ready
    print('[accepted]', conn, 'from', addr, datetime.datetime.now().strftime('%H:%M:%S'))
    conn.setblocking(False)
    sel.register(conn, selectors.EVENT_READ, read)

def read(conn, mask):
    try:
        data = conn.recv(1024)  # Should be ready
        if data:
            if data.decode('utf-8') == 'disconnect':
                print('(closing) recv disconnect,', conn)
                sel.unregister(conn)
                conn.close()
            else:
                print('<echoing>', repr(data), 'to', conn)
                conn.send(data)  # Hope it won't block
        else:
            print('(closing) peer disconnect,', conn)
            sel.unregister(conn)
            conn.close()
    except Exception as e:
        print(e)
        print('(closing) peer disconnect,', conn)
        sel.unregister(conn)
        conn.close()

sock = socket.socket()
sock.bind(('0.0.0.0', 6710))
sock.listen(10)
sock.setblocking(False)
sel.register(sock, selectors.EVENT_READ, accept)

while True:
    events = sel.select()
    for key, mask in events:
        callback = key.data
        callback(key.fileobj, mask)
