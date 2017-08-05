import socket
import json
import attr
from model import *

HOST = 'punter.inf.ed.ac.uk'
# 9001 - 9240
PORT = 9001
NAME = 'Lawson Takayama Science Town'

# https://docs.python.jp/3/howto/sockets.html
class Client:
    def __init__(self):
        self.sock = socket.socket()
        self.sock.connect((HOST, PORT))

        # https://gist.github.com/arosh/d3582de637db8ec057b8
        import logging
        formatter = logging.Formatter('%(asctime)s %(message)s')
        handler = logging.StreamHandler()
        handler.setFormatter(formatter)
        self.logger = logging.getLogger('client')
        self.logger.addHandler(handler)
        self.logger.setLevel(logging.DEBUG)
        self.logger.info('[connect] %s:%s' % (HOST, PORT))

    def send(self, msg):
        if not isinstance(msg, bytes):
            msg = bytes(msg, encoding='ascii')
        self.sock.sendall(msg)
        self.logger.info('[send] %s' % str(msg))

    def sendjson(self, msg):
        import json
        msg= json.dumps(msg, separators=(',', ':'))
        self.send('%d:%s' % (len(msg), msg))

    def recv(self):
        # データサイズは9桁以下なので，':'も考慮して10より大きい最小の2ベキを設定
        data = self.sock.recv(16)
        self.logger.info('[recv] %s' % str(data))
        # 最初の':'で区切る
        split = data.decode('ascii').split(':', maxsplit=1)
        # データサイズ
        msglen = int(split[0])
        chunk = split[1].encode('ascii')
        chunks = []
        chunks.append(chunk)
        total = len(chunk)
        while total < msglen:
            chunk = self.sock.recv(min(4048, msglen - total))
            self.logger.info('[recv] %s' % str(chunk))
            if len(chunk) == 0:
                raise RuntimeError("socket connection broken")
            chunks.append(chunk)
            total += len(chunk)
        return b''.join(chunks)

    def recvjson(self):
        recv = self.recv()
        return json.loads(recv.decode('ascii'))

    def close(self):
        self.close()

def handshake(client):
    client.sendjson({'me': NAME})
    recv = client.recvjson()
    assert recv['you'] == NAME

def setup(client):
    recv = client.recvjson()
    p = recv['punter']
    client.sendjson({'ready': p})
    rivers = []
    for river in recv['map']['rivers']:
        rivers.append(RiverModel(source=river['source'], target=river['target']))
    model = SetupModel(
            n=recv['punters'],
            p=recv['punter'],
            sites=recv['map']['sites'],
            rivers=rivers,
            mines=recv['map']['mines'])
    return model

def parse_move(move):
    if 'pass' in move:
        model = MoveModel(
            punter=move['pass']['punter'],
            source=-1,
            target=-1)
    else:
        model = MoveModel(
            punter=move['claim']['punter'],
            source=move['claim']['source'],
            target=move['claim']['target'])
    return model

# (continue, model)
def pull(client):
    recv = client.recvjson()
    # scoring
    if 'stop' in recv:
        moves = []
        for move in recv['stop']['moves']:
            moves.append(parse_move(move))
        model = StopModel(moves=moves, scores=recv['stop']['scores'])
        return (False, model)

    # gameplay
    moves = []
    for move in recv['move']['moves']:
        moves.append(parse_move(move))
    model = GameplayModel(moves=moves)
    return (True, model)

def push(client, move):
    if move.is_pass:
        send = {
            'pass': {
                'punter': move.punter,
            },
        }
    else:
        send = {
            'claim': {
                'punter': move.punter,
                'source': move.source,
                'target': move.target,
            },
        }
    client.sendjson(send)

def main():
    client = Client()
    handshake(client)
    setup(client)
    pull(client)

if __name__ == '__main__':
    main()
