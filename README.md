# ICFPC 2017

## 必要なもの

* Python3系

## セットアップ方法

Anaconda / Miniconda を使っている人

```
$ conda install attrs
```

使っていない人

```
$ pip install attrs
```

## AIを作る人がやること

1. 標準入出力仕様書の通りに入力がやってくるので，stdin/stdoutを通してそれを処理するAIを書く
2. `python communication/compare.py AIの実行ファイルのパス1 AIの実行ファイルのパス2` で40回対戦を行う
3. 勝率が6割を超えていたら，90%の確率でそっちのほうが強いと言える

## 標準入出力仕様書

setup -> pull -> push -> pull -> push -> pull -> push ...

### SetUp

グラフの情報が与えられる

```
N P S M R
S_1 ... S_S
M_1 ... M_M
R_{S,1} R_{T,1}
...
R_{S,R} R_{T,R}
```

N：全プレイヤー数
P：自分のプレイヤーID
S：土地の数
M：鉱山の数
R：川の数
`S_1` ... `S_S`：土地のID
`M_1` ... `M_M`：鉱山として設定されている土地のID
`R_{S,i}` `R_{T,i}`：i番目の川の両端の土地のID

### Pull

前回の自分の行動からの差分が与えられる

```
P_1 S_1 T_1
...
P_P S_P T_P
```

`P_i`：プレイヤーのID
`S_i T_i`：`P_i`が直前の行動で要求した川の両端の土地のID。`P_i`がパスした場合は両方とも`-1`

### Push

自分の行動を送信する

```
P S T
```

`P`：自分のプレイヤーID
`S T`：要求したい川の両端の土地のID
