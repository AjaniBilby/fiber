# Command: if
A standard flow control where the adjacent block will only execute when the supplied value is equal to ``1``. There is also an optional ``else`` condition with will execute when the value does not equal one; if the ``else`` command is not imediatly after the closing bracket then it will not be detected.

```fiber
if {Register}
{
	{Code1}
}
```

```fiber
if {Register}
{
	{Code1}
}
else
{
	{Code2}
}
```

## Valid combinations
| Register | Code1 | ?Code2 |
|:-:|:-:|:-:|
| Register value | Fiber code | Fiber code |
