# Command: Compare
Compares two values storing the result in ``result``
```fiber
cmpr {A} {method} {B} {Result}
```

## Valid Combinations
| A | Method | B | Result |
|:-:|:-:|:-:|:-:|
| Register Value | ``=``, ``<``, or ``>`` | Register Value | Register Value
| • | • | Register Address | •
| • | • | Int | •
| • | • | UInt | •
| • | • | Float | •
| Register Address | • | Register Address | •
