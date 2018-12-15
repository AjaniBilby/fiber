#pragma once

#define DEBUG_SHOW_THREAD_ACTIVITY false

// A string describing the application version
#define FIBER_VERSION "v0.0.0a"

// The number of works to be initilized by default
size_t THREAD_COUNT = 4;

// Should a thread complete it's own tasks first, or attempt to claim unclaimed work?
// true = own work first
// false = claim unclaimed work
#define THREAD_OWN_TASK_PRIORITY true

// Should all interpreted langauge level allocates wipe any existing data?
// true = sets all allocated bytes to 0 on allocation
// false = just allocates the bytes
#define MemorySafe false
