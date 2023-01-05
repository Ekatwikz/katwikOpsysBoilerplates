#include "./katwikOpsys/katwikOpsys.h"

// max time? should be an argument tbh?
#define T 3

volatile sig_atomic_t sigint_received = 0;
void sigint_handler(int sig) {
	unused_(sig);
	sigint_received = 1;
}

volatile sig_atomic_t sigalrm_received = 0;
void sigalrm_handler(int sig) {
	unused_(sig);
	sigalrm_received = 1;
}

typedef struct threadArgs_t {
	int threadNum,
		seed,
		t;
	int* pigArray;
	pthread_mutex_t* mutex;
} threadArgs_t;
void* threadFunc(void* voidArgs) {
	pthread_setcanceltype_(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	threadArgs_t* args = (threadArgs_t*) voidArgs;

	DBGprintf("No.%d Created\n", args->threadNum);

	while (true) {
		myRandSleep(0.01, 0.05);
		int* myHouse = &args->pigArray[args->threadNum - 1];

		pthread_mutex_lock_(args->mutex);
		if (*myHouse) {
			++*myHouse;
			pthread_mutex_unlock_(args->mutex);
		} else {
			pthread_mutex_unlock_(args->mutex);
			kill(0, SIGINT);
			pause();
		}
	}
}

typedef struct threadSpawnInfo_t {
	int n,
		t;

	sigset_t* mask;
	int* pigArray;

	pthread_t* threads;
	threadArgs_t* threadArgs;
	pthread_mutex_t* mutexes;
} threadSpawnInfo_t;
void spawnThreads(threadSpawnInfo_t* threadSpawnInfo) {
	int n = threadSpawnInfo->n;

	// thread stuff
	threadSpawnInfo->threads = malloc_(n * sizeof(pthread_t));
	threadSpawnInfo->threadArgs = malloc_(n * sizeof(threadArgs_t));

	pthread_attr_t threadAttr = pthread_attr_make();

	threadSpawnInfo->mutexes = malloc_(n * sizeof(pthread_mutex_t));
	for (int i = 0; i < n; ++i) {
		threadSpawnInfo->mutexes[i] = pthread_mutex_make();
	}

	for (int i = 0; i < n; ++i) {
		threadArgs_t args = {
			.threadNum = i + 1,
			.t = threadSpawnInfo->t,

			.pigArray = threadSpawnInfo->pigArray,

			.seed = rand(),
			.mutex = &threadSpawnInfo->mutexes[i]
		 };

		threadSpawnInfo->threadArgs[i] = args;
		pthread_create_(&threadSpawnInfo->threads[i], &threadAttr, &threadFunc, &threadSpawnInfo->threadArgs[i]);
	}

	pthread_attr_destroy_(&threadAttr); // should I?
}

void doWolfStuff(threadSpawnInfo_t* threadSpawnInfo) {
	int n = threadSpawnInfo->n;
	int t = threadSpawnInfo->t;
	int* pigArray = threadSpawnInfo->pigArray;
	pthread_mutex_t* mutexes = threadSpawnInfo->mutexes;

	alarm(t);

	while (!sigalrm_received) {
		myRandSleep(0.1, 0.5);

		// eat homeless pigs
		if (sigint_received) {
			DBGprintf("Got SIGINT, checking who to eat\n");
			for (int i = 0; i < n; ++i) {
				pthread_mutex_lock_(&mutexes[i]);
				if (!pigArray[i]) {
					DBGprintf("Eating %d\n", i);
					pthread_cancel_(threadSpawnInfo->threads[i]);
					// might be useful to remember which threads have already been cancelled,
					// then skip those next time?
				}
				pthread_mutex_unlock_(&mutexes[i]);
			}

			sigint_received = 0;
		}

		int k; // house number
		do {
			k = myRand(0, n - 1);
		} while(!pigArray[k]);
		DBGprintf("Wolf blowing down [%d]:%d\n", k, pigArray[k]);

		for (int i = 0; i <= k; ++i) {
			pthread_mutex_lock_(&mutexes[i]);
			if (pigArray[i] < pigArray[k]) {
				pigArray[i] = 0;
			}
			pthread_mutex_unlock_(&mutexes[i]);
		}

		printf_("[");
		for (int i = 0; i < n; ++i) {
			pthread_mutex_lock_(&mutexes[i]);
			printf_("%d, ", pigArray[i]);
			pthread_mutex_unlock_(&mutexes[i]);
		}
		printf_("]\n");
	}

	DBGprintf("Wolf got alarm\n");
}

int main(int argc, char** argv) {
	// get args
	const char* usageDescription = "N (threads)";
	usage_(argc == 2);
	int n = atoi(argv[1]);
	usage_(n > 0);

	// setup default signal masks...
	sigset_t mask = make_sigset_t(SIGINT), // (will have SIGINT)
		 oldMask = make_sigset_t(); // (will be empty)
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ...so all threads ignore SIGINT by default

	// spawn threads
	srand(time(NULL));
	int* pigArray = malloc_(n * sizeof(int));
	printf_("[");
	for (int i = 0; i < n; ++i) {
		pigArray[i] = myRand(1, 100);
		printf_("%d, ", pigArray[i]);
	}
	printf_("]\n");
	threadSpawnInfo_t threadSpawnInfo = {
		.n = n,
		.t = T,

		.mask = &mask,
		.pigArray = pigArray,

		.threads = NULL,
		.threadArgs = NULL,
		.mutexes = NULL
	};
	spawnThreads(&threadSpawnInfo);

	// set main thread's sigmask back to old default and handle SIGINT
	pthread_sigmask_(SIG_SETMASK, &oldMask, NULL);
	sethandler(sigalrm_handler, SIGALRM);
	sethandler(sigint_handler, SIGINT);
	doWolfStuff(&threadSpawnInfo);

	for (int i = 0; i < n; ++i) {
		pthread_cancel_(threadSpawnInfo.threads[i]);
		pthread_join_(threadSpawnInfo.threads[i], NULL);
	}

	printf_("Final Array state:\n[");
	for (int i = 0; i < n; ++i) {
		printf_("%d, ", pigArray[i]);
	}
	printf_("]\n");

	free_(threadSpawnInfo.threads);
	free_(threadSpawnInfo.threadArgs);
	free_(threadSpawnInfo.mutexes);
	free_(threadSpawnInfo.pigArray);
}
