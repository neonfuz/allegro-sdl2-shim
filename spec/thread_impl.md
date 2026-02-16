# Threading Module Implementation Plan

## Overview

This document details the implementation of the Allegro 5 Threading Module using SDL2 as the backend. The threading module provides mutex locks, condition variables, and thread creation/manangement capabilities.

## Type Mappings

| Allegro Type | SDL2 Equivalent | Description |
|--------------|-----------------|-------------|
| `ALLEGRO_THREAD` | `SDL_Thread*` | Thread handle |
| `ALLEGRO_MUTEX` | `SDL_mutex*` | Mutex for thread synchronization |
| `ALLEGRO_COND` | `SDL_cond*` | Condition variable for thread signaling |

---

## Thread Functions

### al_create_thread

**Function Signature:**
```c
ALLEGRO_THREAD* al_create_thread(void* (*proc)(ALLEGRO_THREAD* thread, void* arg), void* arg);
```

**SDL2 Equivalent:** SDL_CreateThread with custom wrapper

**Implementation Details:**
- SDL_CreateThread takes `int (*func)(void*)` but Allegro uses `void* (*proc)(ALLEGRO_THREAD*, void*)`
- Create a wrapper function that bridges the SDL signature to Allegro signature
- The wrapper receives the Allegro thread proc and arg, calls it, and returns result
- Store the user-provided proc and arg in a struct associated with the SDL_Thread
- Return the SDL_Thread* cast to ALLEGRO_THREAD*

---

### al_create_thread_with_stacksize

**Function Signature:**
```c
ALLEGRO_THREAD* al_create_thread_with_stacksize(void* (*proc)(ALLEGRO_THREAD* thread, void* arg), void* arg, size_t stacksize);
```

**SDL2 Equivalent:** SDL_CreateThreadWithStackSize (SDL 2.0.18+)

**Implementation Details:**
- Similar to al_create_thread but passes stacksize parameter
- Requires SDL 2.0.18 or later for SDL_CreateThreadWithStackSize
- If older SDL version, fall back to al_create_thread with warning

---

### al_start_thread

**Function Signature:**
```c
void al_start_thread(ALLEGRO_THREAD* outer);
```

**SDL2 Equivalent:** N/A (threads start immediately in SDL)

**Implementation Details:**
- In SDL, threads start running immediately upon creation
- Allegro separates creation from starting
- For compatibility, create thread in suspended state if possible, or document that SDL behavior differs
- Most common approach: threads start immediately; this function can be a no-op

---

### al_join_thread

**Function Signature:**
```c
void al_join_thread(ALLEGRO_THREAD* outer, void** ret_value);
```

**SDL2 Equivalent:** SDL_WaitThread

**Implementation Details:**
- SDL_WaitThread takes double pointer for return value
- Pass ret_value directly to SDL_WaitThread
- After wait, the thread is cleaned up by SDL

---

### al_set_thread_should_stop

**Function Signature:**
```c
void al_set_thread_should_stop(ALLEGRO_THREAD* outer);
```

**SDL2 Equivalent:** Custom flag in thread struct

**Implementation Details:**
- SDL does not have built-in stop request mechanism
- Create a custom atomic boolean or mutex-protected flag per thread
- Store this flag in the thread's user data structure
- Use SDL_AtomicCAS or a simple mutex for thread-safe flag access

---

### al_get_thread_should_stop

**Function Signature:**
```c
bool al_get_thread_should_stop(ALLEGRO_THREAD* outer);
```

**SDL2 Equivalent:** Read custom flag

**Implementation Details:**
- Read the atomic flag set by al_set_thread_should_stop
- Return true if flag is set, false otherwise
- Thread procedures should periodically check this and exit cleanly

---

### al_destroy_thread

**Function Signature:**
```c
void al_destroy_thread(ALLEGRO_THREAD* thread);
```

**SDL2 Equivalent:** SDL_WaitThread + SDL_DetachThread (or just SDL_WaitThread)

**Implementation Details:**
- If thread is still running, call SDL_WaitThread to join it
- Free any associated user data (proc, arg, should_stop flag)
- In SDL 2.0.16+, can use SDL_DetachThread for automatic cleanup
- For older SDL, just call SDL_WaitThread which also cleans up

---

### al_run_detached_thread

**Function Signature:**
```c
void al_run_detached_thread(void* (*proc)(void* arg), void* arg);
```

**SDL2 Equivalent:** SDL_CreateThread + SDL_DetachThread

**Implementation Details:**
- Create thread using SDL_CreateThread
- Immediately call SDL_DetachThread (or SDL 2.0.16+ equivalent)
- Thread runs independently and cleans up automatically when finished

---

## Mutex Functions

### al_create_mutex

**Function Signature:**
```c
ALLEGRO_MUTEX* al_create_mutex(void);
```

**SDL2 Equivalent:** SDL_CreateMutex

**Implementation Details:**
- Simply call SDL_CreateMutex()
- SDL_CreateMutex returns NULL on failure, Allegro returns NULL on failure
- Return the SDL_mutex* cast to ALLEGRO_MUTEX*

---

### al_create_mutex_recursive

**Function Signature:**
```c
ALLEGRO_MUTEX* al_create_mutex_recursive(void);
```

**SDL2 Equivalent:** SDL_CreateMutex (recursive behavior handled differently)

**Implementation Details:**
- SDL mutexes are not recursive by default
- For recursive mutex behavior, wrap SDL_mutex with a count
- Create a custom struct containing:
  - SDL_mutex* (the actual mutex)
  - SDL_ThreadID (owner thread ID)
  - int (lock count)
- On lock: if current thread owns it, increment count; otherwise lock mutex
- On unlock: decrement count; if zero, unlock mutex
- Alternative: use SDL_CreateMutex and document that recursive behavior requires manual tracking

---

### al_lock_mutex

**Function Signature:**
```c
void al_lock_mutex(ALLEGRO_MUTEX* mutex);
```

**SDL2 Equivalent:** SDL_LockMutex

**Implementation Details:**
- Simply call SDL_LockMutex on the underlying SDL_mutex
- SDL_LockMutex handles both normal and recursive (if implemented as such) mutexes

---

### al_unlock_mutex

**Function Signature:**
```c
void al_unlock_mutex(ALLEGRO_MUTEX* mutex);
```

**SDL2 Equivalent:** SDL_UnlockMutex

**Implementation Details:**
- Simply call SDL_UnlockMutex on the underlying SDL_mutex
- Must match the lock count for recursive mutexes

---

### al_destroy_mutex

**Function Signature:**
```c
void al_destroy_mutex(ALLEGRO_MUTEX* mutex);
```

**SDL2 Equivalent:** SDL_DestroyMutex

**Implementation Details:**
- Call SDL_DestroyMutex on the underlying SDL_mutex
- If using custom recursive wrapper, free the wrapper struct
- Set pointer to NULL if needed for safety

---

## Condition Variable Functions

### al_create_cond

**Function Signature:**
```c
ALLEGRO_COND* al_create_cond(void);
```

**SDL2 Equivalent:** SDL_CreateCond

**Implementation Details:**
- Simply call SDL_CreateCond()
- SDL_CreateCond returns NULL on failure, Allegro returns NULL on failure
- Return the SDL_cond* cast to ALLEGRO_COND*

---

### al_destroy_cond

**Function Signature:**
```c
void al_destroy_cond(ALLEGRO_COND* cond);
```

**SDL2 Equivalent:** SDL_DestroyCond

**Implementation Details:**
- Simply call SDL_DestroyCond on the underlying SDL_cond*

---

### al_wait_cond

**Function Signature:**
```c
void al_wait_cond(ALLEGRO_COND* cond, ALLEGRO_MUTEX* mutex);
```

**SDL2 Equivalent:** SDL_CondWait

**Implementation Details:**
- SDL_CondWait automatically unlocks the mutex and waits atomically
- When signaled, it re-locks the mutex before returning
- This matches Allegro's behavior exactly
- Simply call SDL_CondWait(cond, mutex)

---

### al_wait_cond_until

**Function Signature:**
```c
int al_wait_cond_until(ALLEGRO_COND* cond, ALLEGRO_MUTEX* mutex, const ALLEGRO_TIMEOUT* timeout);
```

**SDL2 Equivalent:** SDL_CondWaitTimeout

**Implementation Details:**
- SDL_CondWaitTimeout takes timeout in milliseconds
- Need to convert ALLEGRO_TIMEOUT (which uses seconds as double) to milliseconds
- Extract timeout value from ALLEGRO_TIMEOUT structure
- SDL_CondWaitTimeout returns 0 on signal, SDL_MUTEX_TIMEDOUT on timeout, or -1 on error
- Return 0 for signaled (success), non-zero for timeout (failure in Allegro terms)

**Timeout Conversion:**
```c
// ALLEGRO_TIMEOUT contains internal data; use al_init_timeout to populate
// Then extract milliseconds: timeout_ms = (uint32_t)(timeout_seconds * 1000)
```

---

### al_broadcast_cond

**Function Signature:**
```c
void al_broadcast_cond(ALLEGRO_COND* cond);
```

**SDL2 Equivalent:** SDL_CondBroadcast

**Implementation Details:**
- Simply call SDL_CondBroadcast(cond)
- Wakes all threads waiting on the condition variable

---

### al_signal_cond

**Function Signature:**
```c
void al_signal_cond(ALLEGRO_COND* cond);
```

**SDL2 Equivalent:** SDL_CondSignal

**Implementation Details:**
- Simply call SDL_CondSignal(cond)
- Wakes one thread waiting on the condition variable (if any)

---

## Additional Considerations

### Thread Local Storage

Allegro may provide thread-local storage capabilities. SDL2 does not have built-in TLS, but can be emulated using:
- Platform-specific TLS (pthread on POSIX, TlsAlloc on Windows)
- A thread-local wrapper using pthread_getspecific/pthread_setspecific

### Thread Priority

SDL2 does not directly support thread priorities. If needed:
- Use platform-specific APIs after thread creation
- Document that priority is not supported in SDL2 backend

### Thread Naming

SDL2 does not support naming threads. This can be:
- Ignored (no-op implementation)
- Implemented platform-specifically (prctl on Linux, SetThreadName on Windows)

---

## Implementation Order

1. **Phase 1: Basic Mutex** - al_create_mutex, al_lock_mutex, al_unlock_mutex, al_destroy_mutex
2. **Phase 2: Condition Variables** - al_create_cond, al_destroy_cond, al_wait_cond, al_signal_cond, al_broadcast_cond
3. **Phase 3: Timed Waits** - al_wait_cond_until (requires timeout conversion)
4. **Phase 4: Recursive Mutex** - al_create_mutex_recursive
5. **Phase 5: Thread Management** - al_create_thread, al_join_thread, al_destroy_thread
6. **Phase 6: Thread Control** - al_start_thread, al_set_thread_should_stop, al_get_thread_should_stop
7. **Phase 7: Detached Threads** - al_run_detached_thread, al_create_thread_with_stacksize

---

## Error Handling

| SDL2 Function | Failure Return | Allegro Equivalent |
|---------------|----------------|---------------------|
| SDL_CreateMutex | NULL | Return NULL |
| SDL_CreateCond | NULL | Return NULL |
| SDL_CreateThread | NULL | Return NULL |
| SDL_LockMutex | -1 (error) | Possibly assert or log |
| SDL_WaitThread | void | void |

---

## Header Dependencies

The implementation requires:
```c
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
```

And from Allegro headers:
```c
#include "allegro5/altime.h"  // For ALLEGRO_TIMEOUT
```

---

## Testing Checklist

- [ ] Test mutex lock/unlock from single thread
- [ ] Test mutex lock from multiple threads (verify blocking)
- [ ] Test recursive mutex lock count
- [ ] Test condition variable signal (wakes one thread)
- [ ] Test condition variable broadcast (wakes all threads)
- [ ] Test timed wait (timeout expires)
- [ ] Test thread creation and join
- [ ] Test thread stop flag propagation
- [ ] Test detached thread cleanup

---

## New Details from SDL Source Analysis

### SDL2 Threading APIs

The repository contains **SDL2** with threading support:

1. **Thread Implementation**: Located in `src/thread/`
   - Platform-specific implementations: `thread/pthread/`, `thread/windows/`
   - Key files: `SDL_systhread.c`, `SDL_sysmutex.c`, `SDL_syscond.c`

2. **SDL2 Thread Functions**:
   - `SDL_CreateThread()` - Create new thread
   - `SDL_WaitThread()` - Wait for thread completion
   - `SDL_GetThreadID()` / `SDL_GetCurrentThreadID()` - Get thread IDs
   - `SDL_ThreadPriority()` - Set thread priority (SDL 2.0.2+)

3. **SDL2 Synchronization Primitives**:
   - `SDL_CreateMutex()` / `SDL_LockMutex()` / `SDL_UnlockMutex()` - Mutexes
   - `SDL_CreateSemaphore()` - Semaphores
   - `SDL_CreateCondition()` - Condition variables

4. **Atomic Operations**: Located in `src/atomic/`
   - Low-level atomic operations and spinlocks
   - `SDL_AtomicCAS()` - Compare-and-swap

### Thread Implementation Details

From `src/thread/pthread/`:
- `SDL_systhread.c` - Thread creation (pthread_create wrapper)
- `SDL_sysmutex.c` - Mutex implementation (pthread_mutex_*)
- `SDL_syscond.c` - Condition variables (pthread_cond_*)

From `src/thread/windows/`:
- Windows API implementations (CriticalSections, etc.)

### SDL2 Implementation Notes

1. **Thread Naming**: SDL2 doesn't support naming threads natively
2. **Detached Threads**: Use `SDL_DetachThread()` (SDL 2.0.16+)
3. **Stack Size**: Use `SDL_CreateThreadWithStackSize()` (SDL 2.0.18+)
4. **Recursive Mutex**: SDL2 mutexes are not recursive by default; implement custom wrapper if needed

### Dependencies Update

| Header | Notes |
|--------|-------|
| SDL2/SDL_thread.h | Thread management |
| SDL2/SDL_mutex.h | Synchronization primitives |
| SDL2/SDL_atomic.h | Atomic operations |
| SDL2/SDL_timer.h | Timer for timeouts |
