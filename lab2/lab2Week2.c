#include "./katwikOpsys/katwikOpsys.h"

// NB: should probably find a better way than
// printing from within signal handlers
volatile sig_atomic_t sigusr1_count = 0;
void sigusr1_handler(int sig) {
	unused_(sig);
	printf("*\n");
	++sigusr1_count;
}

volatile sig_atomic_t sigusr2_count = 0;
void sigusr2_handler(int sig) {
	unused_(sig);
	printf("*\n");
	++sigusr2_count;
}

typedef struct childInfo_t {
	int childNum;
	int childCount;
	sigset_t* mask;
	sigset_t* oldMask;
	int arg;
	int exit;
} childInfo_t;

void childFunc(childInfo_t* childInfo) {
	srand(time(NULL) * getpid() + childInfo->childNum);

	int sigCount = myRand(1, childInfo->childNum);
	double sleepTime = myRandDouble(0.01, 0.03);
	int sig;
	if (childInfo->arg == 1) {
		sig = SIGUSR1;
	} else if (childInfo->arg == 2) {
		sig = SIGUSR2;
	}

	DBGprintf("i:%d, sigCount:%d, sigVersion:%d, sleepTime:%lf\n",
		childInfo->childNum, sigCount, childInfo->arg, sleepTime);
	for (int i = 0; i < sigCount; ++i) {
		myNanoSleep(sleepTime);
		DBGprintf("Sending SIGUSR%d\n", childInfo->arg);
		kill_(0, sig);
	}

	childInfo->exit = sigCount;

	// "cheat" and use highest bit of exit status
	// to say which "signal group" we are
	if (childInfo->arg == 1) {
		childInfo->exit |= 0x80;
	}

	DBGprintf("Child exit\n");
}

void createChildren(void (*childCallback)(childInfo_t*), childInfo_t* childInfos, int n, bool vertically) {
	for (int i = 0; i < n; ++i) {
		childInfo_t* childInfo = &childInfos[i];

		// if we're in child, error checked
		if (!fork_()) {
			if (vertically) {
				createChildren(childCallback, childInfo, n - 1, vertically);
			}

			childInfo->childNum = (vertically ? childInfo->childCount - n : i) + 1;

			childCallback(childInfo);

			waitAllChildren();
			exit(childInfo->exit);
		}

		if (vertically) {
			break;
		}
	}
}

int main(int argc, char** argv) {
	const char* usageDescription = "signalGroup...";
	usage_(2 <= argc && argc <= 8);
	int n = argc - 1;

	sigset_t mask = make_sigset_t(SIGUSR1), // (will have SIGUSR1)
		 oldMask = make_sigset_t(); // (will be empty)
	sigaddset_(&mask, SIGUSR2); // add SIGUSR2
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ignore SIGUSR1 and SIGUSR2
	sethandler(sigusr1_handler, SIGUSR1); // handle SIGUSR1
	sethandler(sigusr2_handler, SIGUSR2); // handle SIGUSR2

	// setup children
	childInfo_t* childInfos = malloc_(sizeof(childInfo_t) * n);
	for (int i = 0; i < n; ++i) {
		int sigVersion = atoi(argv[i + 1]);
		usage_(sigVersion == 1 || sigVersion == 2);
		childInfo_t currChildInfo = { 0, n, &mask, &oldMask, sigVersion, 0 };
		childInfos[i] = currChildInfo;
	}
	createChildren(childFunc, childInfos, n, false);
	free_(childInfos);

	// wait for stuff to happen in parent
	// there's an occasional, atrocious race condition around here,
	// I still haven't figured it out
	sigprocmask(SIG_UNBLOCK, &mask, NULL);
	int sigCountSum1 = 0;
	int sigCountSum2 = 0;
	while (true) {
		int wstatus = 0;
		waitpid(-1, &wstatus, 0);

		if (errno == ECHILD) {
			break;
		} else if (WIFEXITED(wstatus)) {
			int exitStat = WEXITSTATUS(wstatus);
			// child return handling
			if (exitStat & 0x80) {
				sigCountSum1 += exitStat & 0x7F;
			} else {
				sigCountSum2 += exitStat;// & 0xFF;
			}
		} else {
			ERR("??");
		}
	}

	#define MAX_STR 200
	int outputFD = open_("out.txt", O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0664);
	char outStr[MAX_STR + 1] = { 0 };
	snprintf_(outStr, MAX_STR,
		"Final info: Total c_i:%d, Total c_i for group 1:%d, Total c_i for group 2:%d sig1Count:%d sig2Count:%d \n",
		sigCountSum1 + sigCountSum2, sigCountSum1, sigCountSum2, sigusr1_count, sigusr2_count);
	DBGprintf("%s", outStr);
	write_(outputFD, outStr, strlen(outStr));
	close_(outputFD);

	DBGprintf("Parent exit\n");
	return EXIT_SUCCESS;
}
