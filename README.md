# ICFPC 2017

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
