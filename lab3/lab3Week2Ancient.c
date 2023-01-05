// old King-Kong lab
#include "./katwikOpsys/katwikOpsys.h"

volatile sig_atomic_t last_signal = 0;
void basic_handler(int sig) {
	last_signal = sig;
}

volatile bool shouldReset = false;
void sigint_handler(int sig) {
	unused_(sig); // this line is purely just to suppress Wunused-parameter lol
	// since sigaction.sa_handler requires this function to take int.

	shouldReset = true;
}

// custom struct to pass stuff to the builder threads
typedef struct threadArgs_ {
	int* newYork;
	pthread_mutex_t* newYorkMutexes;
	int threadNum;
	unsigned int seed;
} threadArgs_t;

// builder thread routine
void* build(void* voidArgs) {
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);

	threadArgs_t* args = (threadArgs_t*) voidArgs;

	// sleep then say hi
	myRandSleep(0.1, 0.5);
	printf_("Builder %d starting\n", args->threadNum);

	// start building
	for (int i = args->threadNum - 1; true;) {
		pthread_mutex_lock_(&args->newYorkMutexes[i]);
		++args->newYork[i];
		pthread_mutex_unlock_(&args->newYorkMutexes[i]);

		myRandSleep(0.1, 0.5); // deferred == cancels in here
	}

	//return NULL; // unreachable; infinite loop, thread gets canceled
}

void setupThreads(int N, pthread_t* threads, threadArgs_t* args, int* newYork,
		pthread_mutex_t* newYorkMutexes, pthread_attr_t* attr) {
	for (int i = 0; i < N; ++i) {
		threadArgs_t arg = {
			.newYork = newYork,
			.threadNum = i + 1,
			.seed = rand(),
			.newYorkMutexes = newYorkMutexes
		};
		args[i] = arg;

		pthread_create_(&threads[i], attr, &build, &args[i]);
	}
}

int mainLoop(int N, int* newYork, pthread_mutex_t* newYorkMutexes) {
	int pos = 0; // we return the position we end up at after we finish/timeout

	for (bool shouldQuit = false; // we've got a nested loop, so we use this flag to
			// break the outer loop from the inner one
			pos < N - 1 && !shouldQuit;
			printf_("\n") // slightly tidier output lol
	    ) {
		printf_("Trying to jump from %d\n", pos);

		while (true) {
			// reset the array if we got SIGINT at some point,
			if (true == shouldReset) {
				for (int i = 0; i < N; ++i) {
					pthread_mutex_lock_(&newYorkMutexes[i]);
				}

				for (int i = 0; i < N; ++i) {
					newYork[i] = 0;
				}

				for (int i = 0; i < N; ++i) {
					pthread_mutex_unlock_(&newYorkMutexes[i]);
				}

				shouldReset = false;
			}

			// print the array,
			// (btw, here and when checking the jump condition below,
			// I lock the mutexes, I guess to try not to read intermediate
			// values, but it might just be pointless to do so if we're not
			// intending to write? idk. if they aren't necessary, they can
			// be easily removed without effect, unlike the ones above where
			// we do write to the array)
			for (int i = 0; i < N; ++i) {
				pthread_mutex_lock_(&newYorkMutexes[i]);
				printf_("%d ", newYork[i]);
				pthread_mutex_unlock_(&newYorkMutexes[i]);
			}
			printf_("\n");

			// stop if we got SIGALRM at some point,
			if (SIGALRM == last_signal) {
				printf_("\nTimed out. (cancelling)\n");
				shouldQuit = true;
				break;
			}

			// jump if we can,
			pthread_mutex_lock_(&newYorkMutexes[pos]);
			pthread_mutex_lock_(&newYorkMutexes[pos + 1]);
			if (newYork[pos] < newYork[pos + 1]) {
				pthread_mutex_unlock_(&newYorkMutexes[pos + 1]);
				pthread_mutex_unlock_(&newYorkMutexes[pos]);
				++pos;
				break;
			} else {
				pthread_mutex_unlock_(&newYorkMutexes[pos + 1]);
				pthread_mutex_unlock_(&newYorkMutexes[pos]);
			}

			sleep(1); // zzz
		}
	}

	if (pos + 1 == N) {
		printf_("Jumped to i=%d (finished)\n\n", pos);
	}

	return pos;
}

int main (int argc, char** argv) {
	const char* usageDescription = "N>2 T>=5";

	// get arguments
	usage_(argc == 3);
	int N = atoi(argv[1]);
	int T = atoi(argv[2]);
	usage_(N > 2 && T >= 5);

	if (T == 69) T = 1; // tmp thing to make testing easier

	// allocate array, threads, and thread arguments
	int* newYork = calloc_(N, sizeof(int));
	pthread_t* threads = malloc_(N * sizeof(pthread_t));
	threadArgs_t* args = malloc_(N * sizeof(threadArgs_t));

	// init mutexes
	pthread_mutex_t* newYorkMutexes = malloc_(N * sizeof(pthread_mutex_t));
	for (int i = 0; i < N; ++i) {
		newYorkMutexes[i] = pthread_mutex_make();
	}

	// setup threads' attribute
	pthread_attr_t attr = pthread_attr_make();
#if TEMP_REMOVE
	// unnecessary lol, makes the thread unjoinable
	pthread_attr_setdetachstate_(&attr, PTHREAD_CREATE_DETACHED);
#endif

	// setup default signal masks...
	sigset_t mask = make_sigset_t(SIGALRM), // (will have SIGALRM and SIGINT)
		 oldMask = make_sigset_t(); // (will be empty)
	sigaddset_(&mask, SIGINT);

	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ...so all threads ignore SIGALRM and SIGINT by default

	// make threads
	srand(time(NULL));
	setupThreads(N, threads, args, newYork, newYorkMutexes, &attr);

	// set main thread's sigmask back to old default and handle SIGALRM and SIGINT
	pthread_sigmask_(SIG_SETMASK, &oldMask, NULL);
	sethandler(basic_handler, SIGALRM);
	sethandler(sigint_handler, SIGINT);

	// set an alarm and start the main King Kong loop
	alarm(T);
	int pos = mainLoop(N, newYork, newYorkMutexes);

	// print final state
	printf_("Final state:\nKing Kong index: %d (counting from 0)\nNew York skyline: ", pos);
	for (int i = 0; i < N; ++i) {
		printf_("%d ", newYork[i]);
	}
	printf_("\n");

	// cancel and join threads
	for (int i = 0; i < N; ++i) {
		pthread_cancel_(threads[i]);
		pthread_join_(threads[i], NULL);
	}

	// clean stuff up:
	for (int i = 0; i < N; ++i) {
		pthread_mutex_destroy_(&newYorkMutexes[i]);
	}

	pthread_attr_destroy_(&attr);

	free_(newYorkMutexes);
	free_(newYork);
	free_(threads);
	free_(args);

	return EXIT_SUCCESS;
}
