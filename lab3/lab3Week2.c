#include "./katwikOpsys/katwikOpsys.h"

volatile sig_atomic_t sigint_received = 0;
void sigint_handler(int sig) {
	unused_(sig);
	alarm(0); // cancel alarm
	sigint_received = 1;
}

volatile sig_atomic_t sigalrm_received = 0;
void sigalrm_handler(int sig) {
	unused_(sig);
	sigalrm_received = 1;
}

struct threadArgs_t;
typedef struct threadSpawnInfo_t {
	int n,
		m,
		t,
		*array;

	pthread_t* threads;
	struct threadArgs_t* threadArgs;
	pthread_mutex_t* mutexes;
} threadSpawnInfo_t;
typedef struct threadArgs_t {
	int threadNum;

	threadSpawnInfo_t* threadSpawnInfo;
} threadArgs_t;

void* threadFunc(void* voidArgs) {
	pthread_setcanceltype_(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	// NB: should NOT have async cancel and should instead
	// actually cleanup cancels using pthread_cleanup_push and ..._pop

	// for convenience:
	threadArgs_t* args = (threadArgs_t*) voidArgs;
	int n = args->threadSpawnInfo->n;
	int* array = args->threadSpawnInfo->array;
	int dir = 1 - 2 * myRand(0, 1);
	int num = args->threadNum;
	pthread_mutex_t* mutexes = args->threadSpawnInfo->mutexes;

	// init pos
	int pos = myRand(0, n - 1);
	while (true) {
		pthread_mutex_lock_(&mutexes[pos]);
		if (array[pos]) {
			pthread_mutex_unlock_(&mutexes[pos]);
			pos = myRand(0, n - 1);
		} else {
			break;
		}
	}
	array[pos] = dir;
	pthread_mutex_unlock_(&mutexes[pos]);
	DBGprintf("T%d init: pos:%d dir:%d\n", num, pos, dir);

	while (true) {
		myNanoSleep2(1, 0);
		int oldPos = pos;

		pos += dir;
		if (pos == -1) {
			pos = n - 1;
		} else if (pos == n) {
			pos = 0;
		}

		pthread_mutex_lock_(&mutexes[pos]);
		if (array[pos]) {
			pthread_mutex_unlock_(&mutexes[pos]);
			dir *= -1;
			pos = oldPos;
		} else {
			pthread_mutex_unlock_(&mutexes[pos]);

			pthread_mutex_lock_(&mutexes[oldPos]);
			array[oldPos] = 0;
			pthread_mutex_unlock_(&mutexes[oldPos]);

			pthread_mutex_lock_(&mutexes[pos]);
			array[pos] = dir;
			pthread_mutex_unlock_(&mutexes[pos]);
		}
	}
}

void spawnThreads(threadSpawnInfo_t* threadSpawnInfo) {
	int n = threadSpawnInfo->m; // ugly. fix me.

	// thread stuff
	threadSpawnInfo->threads = malloc_(n * sizeof(pthread_t));
	threadSpawnInfo->threadArgs = malloc_(n * sizeof(threadArgs_t));

	pthread_attr_t threadAttr = pthread_attr_make();

	// mutex stuff
	threadSpawnInfo->mutexes = malloc_(n * sizeof(pthread_mutex_t));
	for (int i = 0; i < n; ++i) {
		threadSpawnInfo->mutexes[i] = pthread_mutex_make();
	}

	srand(time(NULL));
	for (int i = 0; i < n; ++i) {
		threadArgs_t args = {
			.threadNum = i + 1,
			.threadSpawnInfo = threadSpawnInfo
		 };

		threadSpawnInfo->threadArgs[i] = args;
		pthread_create_(&threadSpawnInfo->threads[i], &threadAttr, &threadFunc, &threadSpawnInfo->threadArgs[i]);
	}

	pthread_attr_destroy_(&threadAttr); // ?
}

int main(int argc, char** argv) {
	// get args
	const char* usageDescription = "n (arraySize) m (threads) t (time)";
	usage_(argc == 4);
	int n = atoi(argv[1]);
	int m = atoi(argv[2]);
	int t = atoi(argv[3]);
	usage_(5 <= n && n <= 500);
	usage_(3 <= m && m <= 300);
	usage_(m <= n);
	usage_(50 <= t);

	// setup default signal masks...
	sigset_t mask = make_sigset_t(SIGALRM), // (will have SIGALRM and SIGINT)
		 oldMask = make_sigset_t(); // (will be empty)
	sigaddset_(&mask, SIGINT);
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ...so all threads ignore SIGALRM and SIGINT by default

	// spawn threads
	threadSpawnInfo_t threadSpawnInfo = {
		.n = n,
		.m = m,
		.t = t,

		.array = calloc_(n, sizeof(int)),

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
	alarm(t);
	pthread_mutex_t* mutexes = threadSpawnInfo.mutexes;
	while (!sigalrm_received && !sigint_received) {
		myNanoSleep2(1, 0);

		printf_("[");
		for (int i = 0; i < n; ++i) {
			pthread_mutex_lock_(&mutexes[i]);
			printf_("%d, ", threadSpawnInfo.array[i]);
			pthread_mutex_unlock_(&mutexes[i]);
		}
		printf_("]\n");
	}

	for (int i = 0; i < m; ++i) {
		pthread_cancel_(threadSpawnInfo.threads[i]);
		pthread_join_(threadSpawnInfo.threads[i], NULL);
	}

	// redundant...
	printf_("Final Array:\n[");
	for (int i = 0; i < n; ++i) {
		printf_("%d, ", threadSpawnInfo.array[i]);
	}
	printf_("]\n");

	free_(threadSpawnInfo.threads);
	free_(threadSpawnInfo.threadArgs);
	free_(threadSpawnInfo.mutexes);
	free_(threadSpawnInfo.array);
}
