# Internal: Command: IF
How the if statment is coverted over time (in a human representation)

---

```fiber
if &rA
{
	...
}
else
{
	...
}
```

```fiber
if &rA
jump {numCmdToA}
	...
jump {numCmdToB}
blank
blank
	# A
	...
blank
# B
```

---
```fiber
if &rA
{
	...
}
```
```fiber
if &rA
jump {numCmdToA}
	...
blank
# A
```
