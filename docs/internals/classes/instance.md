# Instance
A [function](./function.md) instance, is merely that. It refers to the original function to know it's execution commands, however the instance it's self describes the [registers](./register.md) and the location of the domain (a.k.a. local space/memory).  
A instance is also tied to one specific [worker](./worker.md)/thread. Thus when processing on this instance is paused, or a callback occurs it always executes within that first perscribed worker; this insures that data races do not occur within local memory.
