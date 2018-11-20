# Command: Initilize
Creates a new function instance, and binds a return action.
```
init {Name} {Register}
{
  {Preamble}
}
{
  {Code}
}
```

## Use
``Preamble`` is code that will linearly execute before the new instance can start. This is designed to allow injection of function arguments before another [worker](./../../internals/classes/worker.md) attempts to execute the instance.  

``Code`` will execute linearly once the function instance returns a value.  

``Register`` will point to the starting address of the local space of the new instance.

## Valid combinations
| Name | Register | Preamble | Code |
|:-:|:-:|:-:|:-:|
| Any single word | Register Address | Fiber code | Fiber code |
