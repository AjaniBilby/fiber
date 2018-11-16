# Command: Function
This command allows the definitions of functions. It is important to note that these definitions are extracted on interpretation, so no linear order for function definition is required. However what is requried is a complete [block](./../concept/block.md) immediatly after this command.

```fiber
func {name} {domain}
```

## Possible combinations
``name`` can be any non-broken string to refer to the function with.
``domain`` must be a ``uint`` or ``hex`` value to define the local space required by a function.
