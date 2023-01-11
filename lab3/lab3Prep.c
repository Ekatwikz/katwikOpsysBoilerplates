#include "./katwikOpsys/katwikOpsys.h"

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
	pthread_mutex_t* mutex;
} threadArgs_t;
void* threadFunc(void* voidArgs) {
	pthread_setcanceltype_(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	//pthread_setcanceltype_(PTHREAD_CANCEL_DEFERRED, NULL);
	threadArgs_t* args = (threadArgs_t*) voidArgs;

	for (int i = 0; i < args->t; ++i) {
		DBGprintf("%d waitin\n", args->threadNum);
		pthread_mutex_lock(args->mutex);
		DBGprintf("%d goteem\n", args->threadNum);
		myRandSleep(0.5, 1.5);
		pthread_mutex_unlock_(args->mutex);
		DBGprintf("%d released\n", args->threadNum);
		myNanoSleep(0.1);
	}

	DBGprintf("%d done\n", args->threadNum);

	return NULL;
}

typedef struct threadSpawnInfo_t {
	int n,
		t;
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
	//pthread_attr_setdetachstate_(&threadAttr, PTHREAD_CREATE_DETACHED);

	// mutex stuff
#define MUTEX_COUNT 1
	threadSpawnInfo->mutexes = malloc_(MUTEX_COUNT * sizeof(pthread_mutex_t));
	for (int i = 0; i < MUTEX_COUNT; ++i) {
		threadSpawnInfo->mutexes[i] = pthread_mutex_make();
	}

	srand(time(NULL));
	for (int i = 0; i < n; ++i) {
		threadArgs_t args = { 
			.threadNum = i + 1,
			.t = threadSpawnInfo->t,
			.seed = rand(),
			.mutex = threadSpawnInfo->mutexes // for now
		};

		threadSpawnInfo->threadArgs[i] = args;
		pthread_create_(&threadSpawnInfo->threads[i], &threadAttr, &threadFunc, &threadSpawnInfo->threadArgs[i]);
	}

	pthread_attr_destroy_(&threadAttr); // ?
}

int main(int argc, char** argv) {
	//close_(-1); // uncomment to prove that underscore_ error handling works

	// get args
	const char* usageDescription = "n (threads) t (time)";
	usage_(argc == 3);
	int n = atoi(argv[1]);
	int t = atoi(argv[2]);
	usage_(n > 0);
	usage_(t > 0);

	// setup default signal masks...
	sigset_t mask = make_sigset_t(SIGALRM), // (will have SIGALRM and SIGINT)
		 oldMask = make_sigset_t(); // (will be empty)
	sigaddset_(&mask, SIGINT);
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ...so all threads ignore SIGALRM and SIGINT by default

	// spawn threads
	threadSpawnInfo_t threadSpawnInfo = {
		.n = n,
		.t = t,
		.threads = NULL,
		.threadArgs = NULL,
		.mutexes = NULL
	};
	spawnThreads(&threadSpawnInfo);

	// set main thread's sigmask back to old default and handle SIGALRM and SIGINT
	pthread_sigmask_(SIG_SETMASK, &oldMask, NULL);
	sethandler(sigalrm_handler, SIGALRM);
	sethandler(sigint_handler, SIGINT);

	// set an alarm
	alarm(3);

	while (!sigalrm_received) {
		sleep(1); // not so good
	}

	for (int i = 0; i < n; ++i) {
		pthread_cancel_(threadSpawnInfo.threads[i]);
		pthread_join_(threadSpawnInfo.threads[i], NULL);
		DBGprintf("Joined %d\n", i + 1);
	}

	free_(threadSpawnInfo.threads);
	free_(threadSpawnInfo.threadArgs);
	free_(threadSpawnInfo.mutexes);
}
