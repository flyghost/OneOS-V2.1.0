#test_uselect.py
import usocket as socket, uselect as select

def test(peer_addr):
    s = socket.socket()
    poller = select.poll()
    poller.register(s)

    # test poll before connect
    p = poller.poll(0)
    print(len(p), p[0][-1])#1 0

    s.connect(peer_addr)

    # test poll during connection
    print(len(poller.poll(0)))#1

    # test poll after connection is established
    p = poller.poll(1000)
    print(len(p), p[0][-1])#1 4

    s.close()

test(socket.getaddrinfo("www.micropython.org", 80)[0][-1])