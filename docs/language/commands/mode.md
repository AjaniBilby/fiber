# Command: Mode
This defines the behavior for a given register. This command will define it's size (number of bytes) as well as primative type (uint, int, float).  
This definition will then change behaviour for other commands such as [math](./math.md), [compare](./compare.md), [bitwise](./bitwise.md) (mainly uses the size).

```fiber
mode {Register} {Mode}
```

## Valid combinations
| Register | Mode |
|:-:|:-|
| Register Value | int8 |
| • | uint8 |
| • | int16 |
| • | uint16 |
| • | int32 |
| • | uint32 |
| • | int64 |
| • | uint64 |
| • | float32 |
| • | float64 |

