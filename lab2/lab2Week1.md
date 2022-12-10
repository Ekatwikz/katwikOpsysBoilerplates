# Lab 2: Counting Signals

Program takes one number `n` as its argument.  
Parent process creates `n` child processes.  
Each child process selects a random value `k` from [0, 10].  
Every second `s`, as long as `s` < `k`, each child process sends `SIGUSR1`
to all child processes (including itself).  
All child processes count the total incoming signals as `i`.  
After 10 seconds all child processes stop and return both(!?) `k` and `i`
values as their exit status.  
Parent process waits for its children, collects their statuses and writes
the results (`PID`, `k`, `i`) sorted by `k` in ascending order to `out.txt`
using low-level POSIX IO.  

1. 4p)
Parent process starts child processes.  
Child processes print their `PID` and `k` values to `stdout`, then exit.  
Parent awaits children.
2. 5p)
Child processes send, receive and count signals as described in the task.  
On expiration, child processes print their `k` and `i` values to `stdout`.
3. 4p)
Parent process receives statuses from expired child processes and prints them
to `stdout` _without sorting_.
4. 4p)
_Results are sorted_ and written to `out.txt`.

###### (Week 1 Group)

