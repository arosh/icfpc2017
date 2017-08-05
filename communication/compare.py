import argparse
import io
import json
from subprocess import PIPE, Popen
import itertools
from tqdm import tqdm
import random
import better_exceptions

from model import *
from online import getLogger, setup_model_to_stdin, pull_model_to_stdin, parse_output

class Server:
    def __init__(self, n):
        filename = 'maps/sample.json'
        with open(filename) as f:
            self.graph = json.load(f)
        self.punters = n
        self.punter = 0
        self.lastMove = []
        for i in range(self.punters):
            self.lastMove.append(MoveModel(i, -1, -1))
        self.history = []

    def next(self):
        self.punter += 1
        self.punter %= self.punters

    def pushMove(self, move):
        self.lastMove[self.punter] = move
        if not (move.source == -1 and move.target == -1):
            for x in self.history:
                a, b = min(x.source, x.target), max(x.source, x.target)
                c, d = min(move.source, move.target), max(move.source, move.target)
                assert not (a == c and b == d)
        self.history.append(move)

def setup(server):
    recv = {
        'punters': server.punters,
        'punter': server.punter,
        'map': server.graph,
    }
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
    server.next()
    return model

def pull(server):
    moves = []
    for move in server.lastMove:
        moves.append(move)
    model = GameplayModel(moves=moves)
    return (True, model)

def push(server, move):
    server.pushMove(move)
    server.next()

def evaluate(server):
    o = []
    setup_stdin = setup_model_to_stdin(setup(server))
    o.append(setup_stdin)
    model = GameplayModel(moves=server.history)
    pull_stdin = pull_model_to_stdin(model)
    o.append(pull_stdin)
    output = ''.join(o) + '\n'
    proc = Popen('./calcScore/calcScore', stdin=PIPE, stdout=PIPE)
    proc.stdin.write(output.encode('ascii'))
    proc.stdin.flush()
    scores = proc.stdout.readline()
    proc.terminate()
    scores = list(map(int, scores.split()))
    if len(scores) == 0:
        print(output)
        assert len(scores) > 0
    return scores

def run(solvers):
    server = Server(len(solvers))
    procs = []
    for i in range(len(solvers)):
        procs.append(Popen(solvers[i], stdin=PIPE, stdout=PIPE))
    for proc in procs:
        setup_model = setup(server)
        setup_stdin = setup_model_to_stdin(setup_model)
        proc.stdin.write(setup_stdin.encode('ascii'))
        proc.stdin.flush()

    for proc in itertools.islice(itertools.cycle(procs), len(setup_model.rivers)):
        _, pull_model = pull(server)
        pull_stdin = pull_model_to_stdin(pull_model)
        proc.stdin.write(pull_stdin.encode('ascii'))
        proc.stdin.flush()
        output = proc.stdout.readline()
        push(server, parse_output(output))

    for proc in procs:
        proc.stdin.write('-1 -1 -1\n'.encode('ascii'))
        proc.terminate()

    return evaluate(server)

def main(solvers):
    winnings = [0] * len(solvers)
    shuffle = []
    for i, name in enumerate(solvers):
        shuffle.append((i, name))
    for i in tqdm(range(60)):
        random.shuffle(shuffle)
        names = [x[1] for x in shuffle]
        scores = run(names)
        winnings[shuffle[scores.index(max(scores))][0]] += 1
    for i in range(len(solvers)):
        print(solvers[i], winnings[i] / 60)

if __name__ == '__main__':
    random.seed(0)
    parser = argparse.ArgumentParser()
    parser.add_argument('solvers', nargs='+')
    args = parser.parse_args()
    main(args.solvers)
