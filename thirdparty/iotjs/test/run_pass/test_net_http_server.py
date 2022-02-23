import http.server
from logging import exception
import socketserver
import time

PORT = 6710

class MyHTTPRequestHandler(http.server.BaseHTTPRequestHandler):

    def __init__(self, *args, directory=None, **kwargs):
        super().__init__(*args, **kwargs)

    def do_GET(self):
        """Serve a GET request."""
        if self.path == '/':
            self.send_response(http.HTTPStatus.OK)
            self.send_header("Content-type", 'text')
            self.end_headers()
            self.wfile.write(b"hello demo")
        elif self.path == '/version':
            ver = self.request_version
            self.send_response(http.HTTPStatus.OK)
            self.send_header("Content-type", 'text')
            self.end_headers()
            self.wfile.write(ver.encode())
        elif self.path == '/exit':
            print('/exit')
            self.request.send(b'Bad http\r\n\r\n')
            self.request.close()
        elif self.path == '/delay':
            time.sleep(2)
            self.send_error(
                http.HTTPStatus.BAD_REQUEST,
                "Bad request path")
        else:
            self.send_error(
                http.HTTPStatus.BAD_REQUEST,
                "Bad request path")
    
    def do_POST(self):
        """Serve a POST request."""
        if self.path == '/echo':
            self.send_response(http.HTTPStatus.OK)
            self.send_header("Content-type", 'text')
            self.end_headers()
            text = self.rfile.read(int(self.headers['content-length']))
            self.wfile.write(text)
        elif self.path == '/status_code':
            code = self.rfile.read(int(self.headers['content-length']))
            self.send_response(int(code))
            self.end_headers()
        else:
            self.send_error(
                http.HTTPStatus.BAD_REQUEST,
                "Bad request path")

    def do_HEAD(self):
        """Serve a HEAD request."""
        if self.path == '/':
            self.send_response(http.HTTPStatus.OK)
            self.end_headers()
        else:
            self.send_error(
                http.HTTPStatus.BAD_REQUEST,
                "Bad request path")

with socketserver.TCPServer(("0.0.0.0", PORT), MyHTTPRequestHandler) as httpd:
    print("serving at port", PORT)
    httpd.serve_forever()
