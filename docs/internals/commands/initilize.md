# Internal: Command: Initilize
How initilize is converted over time (in a human representation).
```fiber
init name @rA
{
	...
}
{
	...
}
```
Converts to
```fiber
init name @rA
jump {numCmdsToA}
	...
jump {numCmdsToB}
stop
	# A
	...
stop
# B
```

When the base instance attempts to execute the preamble it will skip over the first jump and execute through the code; then hit the converted jump of which will cause the linear execution to skip over the post-execution code (marked as comment ``B``) and continue executing.
When the function finshes executing it will execute the line directly after the initilization point which will jump the linear execution to the post-execution code (marked as comment ``A``), once the post execution code is completed it will hit the converted stop command.  
**Note:** that ``numLinesToA`` and ``numLinesToB`` will be a static value determined by the compiler.
