import socket

HOST = 'punter.inf.ed.ac.uk'
# 9001 - 9240
PORT = 9001

# https://docs.python.jp/3/howto/sockets.html
class Client:
    def __init__(self):
        self.sock = socket.socket()

    def connect(self, host, port):
        self.sock.connect((host, port))

    def send(self, msg):
        if not isinstance(msg, bytes):
            msg = bytes(msg)
        self.sock.sendall(msg)

    def recv(self):
        # データサイズは9桁以下なので，':'も考慮して10より大きい最小の2ベキを設定
        data = self.sock.recv(16)
        # 最初の':'で区切る
        split = data.split(':', maxsplit=1)
        # データサイズ
        msglen = int(split[0])
        chunk = split[1]
        chunks = []
        chunks.append(chunk)
        total = len(chunk)
        while total < msglen:
            chunk = self.sock.recv(min(4048, msglen - total))
            if len(chunk) == 0:
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            total += len(chunk)
        return b''.join(chunks)

    def close(self):
        self.close()
