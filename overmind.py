#!/usr/bin/env python

import os
from BaseHTTPServer import BaseHTTPRequestHandler, HTTPServer
from SocketServer import ThreadingMixIn

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        if self.path == '/':
            if os.path.isfile('index.html'):
                self._send_response()
                with open('index.html') as index:
                    self.wfile.write(index.read())

        elif self.path == '/sleep':
            self._send_response(200, 'text/plain')
            self._sleep()
            
        else:
            self._send_response(404, 'text/plain')

    def _send_response(self, code = 200, content_type = 'text/html'):
        self.send_response(code)
        self.send_header('Content-Type', content_type)
        self.end_headers()

    def _sleep(self):
        os.system("rundll32.exe powrprof.dll,SetSuspendState 0,1,0")

class Server(ThreadingMixIn, HTTPServer):
    "Empty"

server = Server(('0.0.0.0', 13456), Handler)
server.serve_forever()
