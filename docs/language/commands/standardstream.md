# Command: Standard Stream
This command allows a string of bytes to be piped to standard out.

```fiber
ss {Channel} {Address} {Bytes}
```

## Valid Combinations
| Address | Bytes | Channel |
|:-:|:-:|:-:|
| ``out``| Register Address | Register Value |
| • | • | Uint |
| • | • | Hex |
| ``log`` | • | • |
| • | • | Uint |
| • | • | Hex |
| ``error`` | • | •|
| • | • | Uint |
| • | • | Hex |
