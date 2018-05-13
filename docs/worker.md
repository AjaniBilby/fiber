# Worker
A worker's purpose is to complete all perscribed tasks.  
When a task is given to a worker it will mark it as inprogress and use the information stored within the task to direct it to it's inital state.  
Once this has been done the worker will run until it hits an end point, then it marks the task as done and looks for the next task.

## Task Stack
Held privatly within the interpeter is a list of tasks called a ``stack``.

## Task Dispatching
When a task is issued for dispatch it fills the first avaiable space within the stack.  
A task slot is defined as available when ``completed == true``.

## Task Finding
When a worker is searching for a task it searches from it's previous task through the list (acending) till it reaches the point it was already at. This allows better equality between tasks, making sure that tasks are less likely to be put off, without requiring large task management infastructure.  
A task is deemed available for work if ``active == false`` and ```(target == true && to == this.id) || target == false```.  
Once a task is found the first thing to be done is to set active to ``true`` to ensure another task does don't snatch it (this would only occur if ``target == false``) and make processing on the task occur twice.

## Task Data Structure
| Name | Source Name | Type | Purpose |
|:--|:--|--:|:--|
| **Task Description** |
| Code Position | codePos | ``unsigned int`` | The starting point for the interpreter to work |
| Memory Position | memPos | ``unsigned char*`` | The starting value for ``rA`` within the interpreter |
| **Work Specification** |
| Is Targeted | target | ``bool`` | Is this task perscribed to a certain worker? |
| Target | to | ``int`` | Which worker is the task perscribed to? (Also used for which worker is currently processing) |
| **Status** |
| Completed | complete | ``bool`` | Is this task done? If so, it can be overwritten with new task information |
| Active | active | ``bool`` | Is a worker currenly processing this task? |