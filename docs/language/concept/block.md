# Concept: Block
A block is a section of code of which is surrounded by ``{`` and ``}``; it is critical that these symboles be on their own dedicated lines.  
Depending on what prepends a block their behaviour may vary.  
For instance if you attempt to use [continue](./../commands/continue.md) or [break](./../commands/break.md) within the block used by a [function definition](./../commands/function.md), then the code will not compile as the function it's self will not store those definition boundries.  
There are also other exceptions such as ``if``, as well as others with alter the behaviour of a block; however if there is no behaviour altering command prepending a block then [continue](./../commands/continue.md) and [break](./../commands/break.md) will behave as normal.
