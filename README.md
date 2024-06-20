# Operating systems - 2.project

## Overview

This project simulates a system where customers (Z processes) and office workers (U processes) interact in a service office. The office manages multiple service queues and is controlled using semaphores to synchronize access to shared resources. This program is written in C and uses POSIX shared memory and semaphores to coordinate between multiple processes.

## Project Structure

- **proj2.c**: The main C file containing the implementation of the project.
- **proj2.out**: The output file where the results of the program execution are written.

## Compilation

To compile the project, use the following command:

```
make
```

This will create an executable file named `proj2`.

## Execution

To run the program, use the following command:

```
./proj2 NZ NU TZ TU F
```

Where:
- `NZ` (int): Number of customers (Z processes).
- `NU` (int): Number of office workers (U processes).
- `TZ` (int): Maximum time (in milliseconds) a customer spends before deciding to either enter the office or go home if the office is closed.
- `TU` (int): Maximum time (in milliseconds) an office worker takes a break when no customers are in the queue.
- `F` (int): The time (in milliseconds) after which the office starts closing operations.

Example:
```
./proj2 10 5 1000 100 5000
```

## Arguments Validation

- The program checks that exactly 5 arguments are provided.
- Specific ranges for arguments are validated:
  - `NZ` must be ≥ 0.
  - `NU` must be > 0.
  - `TZ` must be in the range [0, 10000].
  - `TU` must be in the range [0, 100].
  - `F` must be in the range [0, 10000].

## Program Flow

1. **Initialization**:
   - Shared memory and semaphores are initialized.
   - The output file `proj2.out` is opened for writing.

2. **Process Creation**:
   - Office workers (`U` processes) are created and start executing their tasks.
   - Customers (`Z` processes) are created and start their interaction with the office.

3. **Office Operation**:
   - Office workers serve customers or take breaks if there are no customers.
   - Customers either enter the office for a service or go home if the office is closed.

4. **Closing Office**:
   - After a specified time (`F` milliseconds), the office starts closing.
   - Office workers stop serving new customers and prepare to go home.
   - Remaining customers in the queue are processed if possible.

5. **Cleanup**:
   - All processes finish their tasks and exit.
   - Shared resources (semaphores and shared memory) are cleaned up.
   - The output file is closed.

## Key Functions

### `checkarguments(int argc, char *argv[])`

Validates the command-line arguments provided to the program.

### `init(Shared_t *shared)`

Initializes semaphores and shared memory for synchronization.

### `destroy(Shared_t *shared)`

Closes and unlinks semaphores and cleans up shared memory.

### `u_process(int i, Shared_t *shared)`

Simulates the behavior of an office worker. The worker serves customers, takes breaks, and eventually goes home when the office is closed.

### `z_process(int i, Shared_t *shared)`

Simulates the behavior of a customer. The customer either waits for a service or goes home if the office is closed.

## Output

The program writes detailed logs of its operations to the `proj2.out` file, including:
- Start and end of customer and worker processes.
- Customer interactions with the office.
- Office worker activities, including serving customers and taking breaks.
- The office closing event.

## Notes

- The random behavior in the process interactions simulates real-life scenarios of an office environment.
