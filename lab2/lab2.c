#include "./katwikOpsys/katwikOpsys.h"

volatile sig_atomic_t last_signal = 0;
void basic_handler(int sig) {
	//printf("?\n");
	last_signal = sig;
}

typedef struct childInfo_t {
	int childNum;
	int childCount;
	sigset_t* mask;
	sigset_t* oldMask;
} childInfo_t;

void childFunc(childInfo_t* childInfo) {
	srand(time(NULL) * getpid() + childInfo->childNum);
	myRandSleep(0.5, 3);

	DBGprintf("%d\n", childInfo->childNum);
	if (childInfo->childNum == childInfo->childCount) {
		kill_(0, SIGUSR1);
	}

	while (last_signal != SIGUSR1) {
		sigsuspend(childInfo->oldMask);
	}
	sigprocmask_(SIG_UNBLOCK, childInfo->mask, NULL);
}

void createChildren(void (*childCallback)(childInfo_t*), childInfo_t* childInfo, int n, bool vertically) {
	for (int i = 0; i < n; ++i) {
		// if we're in child, error checked
		if (!fork_()) {
			if (vertically) {
				createChildren(childCallback, childInfo, n - 1, vertically);
			}

			childInfo->childNum = (vertically ? childInfo->childCount - n : i) + 1;

			childCallback(childInfo);

			waitAllChildren();
			exit(EXIT_SUCCESS);
		}

		if (vertically) {
			break;
		}
	}
}

int main(int argc, char** argv) {
	char* usageDescription = "n (processes) vert (vert or horizontal)";
	usage_(argc >= 3);
	int n = atoi(argv[1]);
	bool vertically = atoi(argv[2]);
	usage_(n > 0);

	sigset_t mask = make_sigset_t(SIGINT), // (will have SIGINT and SIGUSR1)
		 oldMask = make_sigset_t(); // (will be empty)
	sigaddset_(&mask, SIGUSR1); // add SIGUSR1
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ignore SIGINT, SIGUSR1
	sethandler(basic_handler, SIGUSR1); // handle SIGUSR1

	// setup children
	childInfo_t childInfo = { n, n, &mask, &oldMask};
	createChildren(childFunc, &childInfo, n, vertically);

	while (last_signal != SIGUSR1) {
		sigsuspend(&oldMask);
	}
	sigprocmask_(SIG_UNBLOCK, &mask, NULL);

	waitAllChildren();
	DBGprintf("Parent exit\n");
	return EXIT_SUCCESS;
}
