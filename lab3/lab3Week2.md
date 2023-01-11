# Lab 3: Hunting Tulkuns in Pandora

Program takes 3 arguments: `N` (in [5, 500]), `M` (in [3, 300], and <= `N`) and `T` (>= 50).  
Program creates and array of length `N`.  
Program creates `M` threads, each corresponding to a "Tulkun".

Each "Tulkun" picks a location in the array, and places its "mark" (some non-zero value) there.  
Each position in the array can only be "marked" by one "Tulkun" at a time.

Each "Tulkun" picks a random direction (-1 or 1) and "moves" its "mark" in that direction every second.  
If the position it's trying to move to is already "marked", it doesn't move and instead switches its direction.  
The "Tulkun"s move through the array circularly; ie: moving forward at the end goes to the beginning and moving backwards from the beginning goes to the end.

Every second, the main thread prints the state of the array.  
After `T` seconds, or when SIGINT is received, the program cancels all the threads, waits for them to finish and prints the final state of the array.  
All threads will synchronize access to the array using mutexes.

1. 4p) Program creates `M` threads.  
Each thread sleeps then prints a "*".  
Program terminates once all threads finish.
2. 4p) Program creates an array of length `N`.  
"Tulkun" logic is implemented.  
Worker threads get cancelled and program ends after all threads finish.
3. 4p) Mutexes and thread cancellation handlers are implemented.
4. 5p) SIGINT is handled.

###### (Week 2 Group)