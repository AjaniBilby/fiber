# Command: Standard Stream
This command allows a string of bytes to be piped to standard out.

```fiber
ss {Address} {Bytes} {Channel}
```

## Valid Combinations
| Address | Bytes | Channel |
|:-:|:-:|:-:|
| Register Address | Register Value | ``out``|
| • | • | ``log`` |
| • | • | ``error`` |
