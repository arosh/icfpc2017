import attr

@attr.s
class SetupModel:
    n = attr.ib()
    p = attr.ib()
    sites = attr.ib()
    rivers = attr.ib()
    mines = attr.ib()

@attr.s
class RiverModel:
    source = attr.ib()
    target = attr.ib()

@attr.s
class GameplayModel:
    moves = attr.ib()

@attr.s
class MoveModel:
    punter = attr.ib()
    source = attr.ib()
    target = attr.ib()

@attr.s
class StopModel:
    moves = attr.ib()
    scores = attr.ib()

@attr.s
class ScoreModel:
    punter = attr.ib()
    score = attr.ib()
