## Postal Office Process Synchronization (C, Semaphores, Shared Memory)

**Description:**  
Implementation of a multi-process synchronization problem inspired by the "barbershop problem" from *The Little Book of Semaphores*.  
In this simulation, we model a post office with three types of processes: the main process, postal clerks, and customers.

**Key Features:**
- Customers request one of three services: letters, parcels, or money services.
- Customers join the appropriate queue based on their selected service.
- Clerks randomly select a non-empty queue to serve customers.
- If no customers are available, clerks take short breaks.
- After the office closes, clerks finish serving all remaining customers before exiting.
- Late-arriving customers are turned away.

**Technical Highlights:**
- Process synchronization implemented using **POSIX semaphores**.
- **Shared memory** used for counters, queues, and office state management.
- Correct handling of concurrent access with mutual exclusion.
- Graceful error handling for invalid input, semaphore failures, and memory allocation.
- Deterministic and synchronized output written to a file (`proj2.out`), including action numbering.
- Fully compliant with POSIX standards and compiled with `-std=gnu99 -Wall -Wextra -Werror -pedantic`.

**Key System Calls and Libraries:**
- `fork()`, `sem_init()`, `sem_wait()`, `sem_post()`, `mmap()`, `munmap()`, `usleep()`, `wait()`
- `<semaphore.h>`, `<sys/mman.h>`, `<unistd.h>`, `<stdio.h>`, `<stdlib.h>`, `<stdbool.h>`, `<time.h>`

**Compilation and Execution:**
- Built using a provided `Makefile`.
- Fully tested on Linux systems (e.g., Merlin server).
- Example usage:  
  ```bash
  ./proj2 NZ NU TZ TU F
  ```
  where:
  - `NZ` = number of customers
  - `NU` = number of clerks
  - `TZ` = maximum customer arrival delay (ms)
  - `TU` = maximum clerk break time (ms)
  - `F` = time after which the post office closes (ms)
