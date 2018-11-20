# Internal: Class: Worker

A worker runs on an event loop, allowing it to process multiple different function instances which all have individual async behaviour.  
However for a worker to originally receive any work they must aquire it from the shared work pool (a.k.a. unclaimed work). The unclaimed work pool has the same behaviour as a worker's pool, however it will be accessed by multiple threads.  
It also must be noted that a worker may have tasks added to their pool from other tasks; this is due to their task calling back/returning to another worker's task.  

These pools can also be refered to as an [event loop](./event-loop.md) due to their functional behaviour when owned by a worker.
