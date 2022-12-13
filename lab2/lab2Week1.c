#include "./katwikOpsys/katwikOpsys.h"

#define RANDOM_LIMIT 10

volatile sig_atomic_t sigCount = 0;
void basic_handler(int sig) {
	unused_(sig);
	++sigCount;
}

volatile sig_atomic_t alarmReceived = 0;
volatile sig_atomic_t alarmCount = 0;
void alarm_handler(int sig) {
	unused_(sig);
	alarmReceived = 1;
	++alarmCount;
}

typedef struct childInfo_t {
	int childNum;
	int childCount;
	sigset_t* mask;
	sigset_t* oldMask;
	int exit;
} childInfo_t;

void childFunc(childInfo_t* childInfo) {
	srand(time(NULL) * getpid() + childInfo->childNum);
	int randK = myRand(1, RANDOM_LIMIT);

	DBGprintf("k:%d\n", randK);

	// very unsure about this part,
	// pretty sus
	sigprocmask_(SIG_UNBLOCK, childInfo->mask, childInfo->oldMask);
	for (int i = 0; alarmCount < RANDOM_LIMIT && i < randK; ++i) {
		alarm(1);
		while (!alarmReceived) {
			sigsuspend(childInfo->mask);
		}
		alarmReceived = 0;

		printf("%d, Sending\n", randK);
		kill_(0, SIGUSR1);
	}

	while (alarmCount < RANDOM_LIMIT) {
		alarm(1);
		sigsuspend(childInfo->mask);
	}

	childInfo->exit = randK << 4;
	childInfo->exit += sigCount;

	printf("k:%d sigCount:%d\n", randK, sigCount);
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

// we need to sort based off of just part of the return?
// idk tbh
bool MY_NON_NULL(1, 2)
	compFirst4Bits(const LIST_TYPE* const a, const LIST_TYPE* const b) {
		return (WEXITSTATUS(*a) >> 4) < (WEXITSTATUS(*b) >> 4);
	}

int main(int argc, char** argv) {
	char* usageDescription = "n (children)";
	usage_(argc >= 2);
	int n = atoi(argv[1]);
	usage_(n > 0);

	sigset_t mask = make_sigset_t(SIGUSR1), // (will have SIGUSR1)
		 oldMask = make_sigset_t(); // (will be empty)
	sigprocmask_(SIG_BLOCK, &mask, &oldMask); // ignore SIGUSR1
	sethandler(basic_handler, SIGUSR1); // handle SIGUSR1
	sethandler(alarm_handler, SIGALRM); // handle SIGALRM

	// setup children
	childInfo_t* childInfos = malloc(sizeof(childInfo_t) * n);
	for (int i = 0; i < n; ++i) {
		childInfo_t currChildInfo = { n, n, &mask, &oldMask, 0 };
		childInfos[i] = currChildInfo;
	}
	createChildren(childFunc, childInfos, n, false);
	free_(childInfos);

	MyList* vals = newMyList();
	for (pid_t pid; true;) {
		int wstatus = 0;

		// error check except ECHILD:
		ERR_NEG1_(pid = waitpid(-1, &wstatus, 0) , ECHILD);
		if (errno == ECHILD) {
			break;
		} else {
			DBGprintf("Child returned: %d:%d\n",
				WEXITSTATUS(wstatus) >> 4, WEXITSTATUS(wstatus) & 0xF);
			insertValLast(vals, wstatus);
		}
	}

#define MAX_STR 30
	int outputFD = open_("out.txt", O_WRONLY | O_CREAT | O_TRUNC | O_APPEND, 0664);
	sortMyList(vals, compFirst4Bits);
	for (int i = 0; i < n; ++i) {
		int wstatus = popFirstVal(vals);
		char outStr[MAX_STR + 1] = { 0 };
		snprintf(outStr, MAX_STR, "k:%d i:%d\n",
			WEXITSTATUS(wstatus) >> 4, WEXITSTATUS(wstatus) & 0xF);
		write_(outputFD, outStr, strlen(outStr));
	}

	close_(outputFD);
	deleteMyList(vals);
	DBGprintf("Parent exit\n");
	return EXIT_SUCCESS;
}
