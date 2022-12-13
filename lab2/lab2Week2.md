# Lab 2: Counting Signals

Program takes `n` (`n` < 8) arguments. Each argument must be either a 1 or a 2.  
Eg: `./prog 1 1 2 1 2 1`  
Parent process creates `n` child processes.  
Each child process number i receives the i<sup>th</sup> argument.  
If its argument is 1, its `signal = SIGUSR1`. It is "in signal group 1".    
If its argument is 2, its `signal = SIGUSR2`. It is "in signal group 2".  
Each child process number i also generates a random integer c<sub>i</sub> from [1, i].  
Each child process number i also generates a random number t<sub>i</sub> from [0.01, 0.03].  
Each child process then sends c<sub>i</sub> `signal`s to parent process, at t<sub>i</sub> intervals, then exits with status c<sub>i</sub>.  
Parent process uses two counters to separately count the received `SIGUSR1` and `SIGUSR2` signals.  
Parent process also uses two counters to separately sum the c<sub>i</sub> exit codes for each "signal group".  
All four counters are written to `out.txt` using low-level POSIX IO.  

1. 4p)
Parent process starts `n` child processes.  
Child processes print their `PID`, i,  c<sub>i</sub>, t<sub>i</sub>, and their "signal group".  
They then exit with code c<sub>i</sub>.  
Parent process awaits all children.
2. 5p)
Child processes send signals as required.  
Parent process prints "*" to `stdout` on each received signal.  
3. 5p)
Parent process receives the c<sub>i</sub> exit statuses and sums them in _one_ counter.  
Parent process counts the received `SIGUSR1` and `SIGUSR2` signals in _two_ counters.  
All _three_ counters are printed to `stdout`.
4. 3p)
Parent process now receives and sums the c<sub>i</sub> exit codes _separately_ for each "signal group".  
All _four_  counters are written to `out.txt`.

###### (Week 2 Group)
