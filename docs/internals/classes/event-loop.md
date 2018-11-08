# Event Loop

An event loop is a expandable array (vector), who's data elements soly refer to specific instances that need to execute, and from what position.  
Each event loop has it's own lock (mutex), this ensures that multiple threads do not opperate on the same queue at the same time as it can produce race conditions, especially in the case of an [unallocated work pool](./worker.md).
