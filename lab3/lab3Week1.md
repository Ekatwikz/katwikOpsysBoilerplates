# Lab 3: Wolf and Pigs

Program creates an array of `N` integers, each from [1, 100], then prints them.  
Program creates `N` worker threads (aka "pigs").  
In a loop, each "pig" waits for a random time (from [10, 50] milliseconds)
then increments its respective number (aka "house").  

In a loop, the main thread (aka "wolf") waits for a random time (from [100, 500] milliseconds),  
then picks a random position `k` in the array, then "blows west" on it,  
ie: sets every number which is to the left of `Array[k]` AND less than `Array[k]` to 0.

When a "pig" notices that its "house" is zero, it sends SIGINT to the "wolf",  
signalling it to "kill" (ie: cancel the thread of) that "pig".

After `T` seconds have passed, the main thread cancels all threads
and prints the final state of the array.
Each element of the array must be protected by a separate mutex.

1. 4p) Create `N` threads.  
Each thread prints "Created".  
Main thread waits for all threads to finish.
2. 4p) Initialize the array with random numbers.  
Start N threads and implement their logic.  
Implement the "wolf"'s logic.  
Main thread stops and cancels all threads after `T` seconds.
3. 3p) Protect the array's elements with mutexes
4. 3p) (??) Main thread waits for the threads to finish,  
then prints the final state of the array
5. 3p) SIGINT is implemented  
Use `alarm()`s for the upper bound `T` (?)

###### (Week 1 Group)