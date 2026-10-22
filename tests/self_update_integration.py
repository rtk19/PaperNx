#!/usr/bin/env python3
"""Exercise the real curl worker, installer and SDL UI against local HTTP/HTTPS."""
import hashlib
import http.server
import json
import os
from pathlib import Path
import ssl
import struct
import subprocess
import sys
import tempfile
import threading
import time

TARGET = 'v0.12.0-reader.10'

def nro(title='PaperNX', version='0.12.0-reader.10'):
    data = bytearray(0x7000)
    data[0x10:0x14] = b'NRO0'
    struct.pack_into('<I', data, 0x18, 0x2000)
    for index, (offset, size) in enumerate(((0, 0x1000), (0x1000, 0x800), (0x1800, 0x800))):
        struct.pack_into('<II', data, 0x20 + index * 8, offset, size)
    struct.pack_into('<4sI6Q', data, 0x2000, b'ASET', 0, 0, 0, 0x38, 0x4000, 0x4038, 0x50)
    data[0x2038:0x2038 + len(title)] = title.encode()
    version = version[:15] # NACP has a 16-byte, NUL-terminated display version.
    data[0x2038 + 0x3060:0x2038 + 0x3060 + len(version)] = version.encode()
    struct.pack_into('<Q', data, 0x6038, 0x50)
    return bytes(data)

class Server(http.server.ThreadingHTTPServer):
    daemon_threads = True
    scenario = 'ok'
    base = ''
    downgrade = ''

class Handler(http.server.BaseHTTPRequestHandler):
    def log_message(self, *_):
        pass

    def do_GET(self):
        mode, base = self.server.scenario, self.server.base
        payload = nro('OtherApp' if mode == 'badapp' else 'PaperNX', '0.12.0-reader.9' if mode == 'badversion' else '0.12.0-reader.10')
        digest = hashlib.sha256(payload).hexdigest()
        if self.path == '/api':
            time.sleep(.02)
            if mode == 'http':
                self.send_error(403)
                return
            assets = [{'name': 'PaperNX.nro', 'size': len(payload), 'browser_download_url': base + f'download/{TARGET}/PaperNX.nro'},
                      {'name': 'PaperNX.nro.sha256', 'browser_download_url': base + f'download/{TARGET}/PaperNX.nro.sha256'}]
            release = {'tag_name': TARGET if mode != 'current' else 'v0.12.0-reader.3', 'draft': False, 'prerelease': True,
                       'name': 'PaperNX — new reader', 'body': ('שלום עולם. Release notes with mixed Hebrew and English.\n' * 100),
                       'assets': [] if mode == 'missing' else assets}
            body = b'{bad response' if mode == 'malformed' else json.dumps([{'tag_name': 'v9.0.0', 'draft': True}, release]).encode()
        elif self.path.endswith('PaperNX.nro.sha256'):
            body = (digest + '  PaperNX.nro\n').encode()
        elif self.path.endswith('PaperNX.nro') or self.path == '/payload':
            if self.path != '/payload' and mode in ('redirect', 'downgrade'):
                self.send_response(302)
                self.send_header('Location', self.server.downgrade + 'payload' if mode == 'downgrade' else base + 'payload')
                self.end_headers()
                return
            body = payload
            if mode == 'corrupt':
                body = payload[:0x200] + b'!' + payload[0x201:]
            if mode == 'oversized':
                body += b'excess data'
        else:
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header('Content-Length', str(len(payload) if mode == 'truncated' and self.path.endswith('.nro') else len(body)))
        self.end_headers()
        try:
            if mode == 'truncated' and self.path.endswith('.nro'):
                self.wfile.write(body[:1000])
                self.close_connection = True
            elif mode == 'cancel' and self.path.endswith('.nro'):
                for start in range(0, len(body), 256):
                    self.wfile.write(body[start:start+256]); self.wfile.flush(); time.sleep(.02)
            else:
                self.wfile.write(body)
        except (BrokenPipeError, ConnectionResetError):
            pass

def serve(tls=None):
    server = Server(('127.0.0.1', 0), Handler)
    if tls:
        context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
        context.load_cert_chain(*tls)
        server.socket = context.wrap_socket(server.socket, server_side=True)
    server.base = ('https' if tls else 'http') + f'://127.0.0.1:{server.server_port}/'
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server

def main():
    executable, ui_executable = map(lambda p: str(Path(p).resolve()), sys.argv[1:3])
    with tempfile.TemporaryDirectory(prefix='papernx-updater-') as folder:
        root = Path(folder)
        server = serve()
        env = dict(os.environ, SDL_VIDEODRIVER=os.environ.get('SDL_VIDEODRIVER', 'dummy'), SDL_AUDIODRIVER='dummy')
        # Loopback never needs the session's external HTTP proxy.
        env['NO_PROXY'] = env['no_proxy'] = '127.0.0.1,localhost'
        cases = ['ok', 'current', 'missing', 'http', 'malformed', 'corrupt', 'badapp', 'badversion', 'truncated', 'oversized', 'cancel', 'write-error', 'redirect', 'recovery', 'tampered', 'replace-failure', 'missing-marker']
        for case in cases:
            server.scenario = case
            subprocess.run([executable, '--network', str(root / case), server.base, case], env=env, check=True, timeout=25)
        server.scenario = 'ok'
        subprocess.run([ui_executable, str(root / 'ui'), server.base], env=env, check=True, timeout=30)
        cert, key = root / 'cert.pem', root / 'key.pem'
        subprocess.run(['openssl', 'req', '-x509', '-newkey', 'rsa:2048', '-nodes', '-keyout', str(key), '-out', str(cert), '-days', '1', '-subj', '/CN=localhost', '-addext', 'subjectAltName=DNS:localhost,IP:127.0.0.1'], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL, check=True)
        https = serve((str(cert), str(key)))
        for case in ('ok', 'tls-reject', 'downgrade'):
            https.scenario = case
            https.downgrade = server.base
            subprocess.run([executable, '--network', str(root / ('tls-' + case)), https.base, case, '' if case == 'tls-reject' else str(cert)], env=env, check=True, timeout=25)
        https.shutdown();server.shutdown()
    print('PASS: 20 HTTP/HTTPS update scenarios, verified TLS/downgrade rejection, deferred install/recovery and SDL UI')

if __name__ == '__main__':
    main()
