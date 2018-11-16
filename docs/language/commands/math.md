# Command: Math
Uses A as both result location and the first opperand
```fiber
set {A} {Opperation} {B}
```

## Valid Combinations
| A | Opperation | B |
|:-:|:-:|:-:|
| Register Value | ``+``, ``-``, ``*``, ``/``, or ``%`` | Register Value
| - | - | Register Address
| - | - | Int
| - | - | Uint
| - | - | Hex
| Register Address | ``+``, or ``-`` | Int
| - | - | Uint

