import argparse
import io
import json
import socket
from subprocess import PIPE, Popen

from model import *

NAME = 'Lawson Takayama Science Town'
HOST = 'punter.inf.ed.ac.uk'
# 9001 - 9240
PORT = None
SOLVER = None

def getLogger(name):
    # https://gist.github.com/arosh/d3582de637db8ec057b8
    import logging
    formatter = logging.Formatter('%(asctime)s %(message)s')
    handler = logging.StreamHandler()
    handler.setFormatter(formatter)
    logger = logging.getLogger(name)
    logger.addHandler(handler)
    logger.setLevel(logging.INFO)
    return logger

class Client:
    def __init__(self):
        # https://docs.python.jp/3/howto/sockets.html
        self.sock = socket.socket()
        self.logger = getLogger('socket')
        self.logger.info('[connect] start')
        self.sock.connect((HOST, PORT))
        self.logger.info('[connect] %s:%s' % (HOST, PORT))

    def send(self, msg):
        if not isinstance(msg, bytes):
            msg = bytes(msg, encoding='ascii')
        self.sock.sendall(msg)
        self.logger.info('[send] %s' % msg.decode('ascii'))

    def sendjson(self, msg):
        import json
        msg = json.dumps(msg, separators=(',', ':'))
        self.send('%d:%s' % (len(msg), msg))

    def recv(self):
        # データサイズは9桁以下なので，':'も考慮して10より大きい最小の2ベキを設定
        data = self.sock.recv(16)
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
            if len(chunk) == 0:
                raise RuntimeError('socket connection broken')
            chunks.append(chunk)
            total += len(chunk)
        msg = b''.join(chunks)
        self.logger.info('[recv] %d:%s' % (msglen, msg.decode('ascii').rstrip()))
        return msg

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
        rivers.append(RiverModel(
            source=river['source'], target=river['target']))
    model = SetupModel(
        n=recv['punters'],
        p=recv['punter'],
        sites=[x['id'] for x in recv['map']['sites']],
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

def pull(client):
    # (continue, model)
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
    if move.source == -1 and move.target == -1:
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


def setup_model_to_stdin(model):
    with io.StringIO() as f:
        print('%d %d %d %d %d' % (model.n, model.p, len(model.sites),
                                  len(model.mines), len(model.rivers)), file=f)
        print(' '.join(map(str, model.sites)), file=f)
        print(' '.join(map(str, model.mines)), file=f)
        for river in model.rivers:
            print('%d %d' % (river.source, river.target), file=f)
        return f.getvalue()

def pull_model_to_stdin(model):
    with io.StringIO() as f:
        for move in model.moves:
            print('%d %d %d' % (move.punter, move.source, move.target), file=f)
        return f.getvalue()

def parse_output(output):
    punter, source, target = map(int, output.split())
    return MoveModel(punter=punter, source=source, target=target)

def main():
    client = Client()
    proc = Popen(SOLVER, stdin=PIPE, stdout=PIPE)
    handshake(client)
    setup_model = setup(client)
    setup_stdin = setup_model_to_stdin(setup_model)
    proc.stdin.write(setup_stdin.encode('ascii'))
    proc.stdin.flush()
    while True:
        cont, pull_model = pull(client)
        if not cont:
            print(pull_model)
            proc.stdin.write('-1 -1 -1\n'.encode('ascii'))
            proc.terminate()
            break
        pull_stdin = pull_model_to_stdin(pull_model)
        proc.stdin.write(pull_stdin.encode('ascii'))
        proc.stdin.flush()
        output = proc.stdout.readline()
        push(client, parse_output(output))

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('PORT', type=int)
    parser.add_argument('SOLVER', type=str)
    args = parser.parse_args()
    PORT = args.PORT
    SOLVER = args.SOLVER
    main()
