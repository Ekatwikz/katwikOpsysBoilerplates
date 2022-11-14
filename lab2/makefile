# base filenames go here
# separate with spaces
DEFAULTFILES:=lab2

# folder to submit to
SUBMISSIONPATH:=..

# 1 for ez compile,
# 0 to see a few more warnings,
# -1 if you you're a perfectionist
FEELINGLAZY:=-1

# 1 for multi-process programs
# 0 for single process programs
ERR_MULTIPROCESS:=0

# 1 to auto clear msg queues
# 0 to not
# probably only useful on lab6 (Ops2 Lab2) tho
AUTOCLEANMSGQUEUES:=0
IPCMSGQPATH:=/dev/mqueue

# 1 if randomizers should use RAND_R
# 0 if not
USE_RAND_R:=0

# 1 to clean whenever you make
# 0 to not
CLEANONMAKE:=1

# 1 to clear whenever you make
# 0 to not
CLEARONMAKE:=1

# colors lol
USECOLOR:=1

# other options for the headers:
MUNDANE_MESSAGES:=0
EXTENSIONS_ALLOWED:=1

# headers and paths to use
KATLIBS:=errorHelpers easyCheck littleHelpers
KATHDRPATH=katwikOpsys
KATOBJPATH=katwikOpsys/obj
KATSRCPATH=katwikOpsys/src

ifeq ($(CLEANONMAKE), 0)
	SHOULDCLEAN =
else
	SHOULDCLEAN = clean
endif

ifeq ($(CLEARONMAKE), 0)
	SHOULDCLEAR =
else
	SHOULDCLEAR = clear
endif

ifeq ($(USECOLOR), 1)
	BLACK		:= $(shell tput -Txterm setaf 0)
	RED		:= $(shell tput -Txterm setaf 1)
	GREEN		:= $(shell tput -Txterm setaf 2)
	YELLOW		:= $(shell tput -Txterm setaf 3)
	LIGHTPURPLE	:= $(shell tput -Txterm setaf 4)
	PURPLE		:= $(shell tput -Txterm setaf 5)
	BLUE		:= $(shell tput -Txterm setaf 6)
	WHITE		:= $(shell tput -Txterm setaf 7)
	RESET		:= $(shell tput -Txterm sgr0)
else
	BLACK		:= ""
	RED		:= ""
	GREEN		:= ""
	YELLOW		:= ""
	LIGHTPURPLE	:= ""
	PURPLE		:= ""
	BLUE		:= ""
	WHITE		:= ""
	RESET		:= ""
endif

WARNINGS:=all extra
ifeq ($(FEELINGLAZY), 0)
else ifeq ($(FEELINGLAZY), -1)
	WARNINGS+=pedantic error
else # ifeq ($(FEELINGLAZY), 1) # default
	WARNINGS+=no-unused-parameter no-unused-variable no-unused-function no-cpp
endif

SYSLIBS:=pthread rt
DEBUGFLAGS:=-g3 -O0
FFLAGS:=sanitize=address,undefined track-macro-expansion=0 no-omit-frame-pointer
AUTOCLEANKATOBJS:=0

PREDEFINES+=-D '_GNU_SOURCE=1'
PREDEFINES+=-D '_POSIX_C_SOURCE=200809L'
PREDEFINES+=-D 'MUNDANE_MESSAGES=$(MUNDANE_MESSAGES)'
PREDEFINES+=-D 'ERR_MULTIPROCESS=$(ERR_MULTIPROCESS)'
PREDEFINES+=-D 'USE_RAND_R=$(USE_RAND_R)'
PREDEFINES+=-D 'EXTENSIONS_ALLOWED=$(EXTENSIONS_ALLOWED)'

KATOBJS=$(KATLIBS:%=$(KATOBJPATH)/%.o)
COMPILEMSG="$(BLUE)=== $(GREEN)$<$(BLUE) -> $(GREEN)$@ $(BLUE)===$(RESET)\n"

CC=gcc
CFLAGS=$(PREPROCESS) $(ASSEMBLE) $(WARNINGS:%=-W%) $(FFLAGS:%=-f%) $(DEBUGFLAGS)
LDFLAGS=$(SYSLIBS:%=-l%)

.PHONY: clean clear all zip submit

all: $(SHOULDCLEAN) $(SHOULDCLEAR) $(DEFAULTFILES)

$(KATOBJPATH)/%.o: $(KATSRCPATH)/%.c $(KATHDRPATH)/%.h
	@mkdir -pv $(KATOBJPATH)
	@printf $(COMPILEMSG)
	@$(CC) $(KATHDRPATH:%=-I %) $(CFLAGS) $(PREDEFINES) -c $< -o $@ $(LDFLAGS)

$(DEFAULTFILES): $(DEFAULTFILES:%=%.c) $(KATOBJS)
%: %.c $(KATOBJS)
	@printf $(COMPILEMSG)
	-@$(CC) $(KATHDRPATH:%=-I %) $(CFLAGS) $(PREDEFINES) $(KATOBJS) $< -o $@ $(LDFLAGS)

clean:
ifeq ($(AUTOCLEANMSGQUEUES), 1)
	@printf "$(BLUE)Trying to remove * (message queues) from $(GREEN)$(IPCMSGQPATH)$(RESET)\n"
	-@cd $(IPCMSGQPATH); rm -fv *
endif
ifeq ($(AUTOCLEANKATOBJS), 1)
	@printf "$(BLUE)Trying to remove $(GREEN)*.o$(RESET) from $(GREEN)$(KATOBJPATH)$(RESET)\n"
	-@cd $(KATOBJPATH); rm -fv *.o
endif
	@printf "$(BLUE)Trying to remove $(GREEN)$(DEFAULTFILES)$(GREEN)$(RESET)\n"
	-@rm -fv $(DEFAULTFILES)

clear:
	@clear

zip: $(DEFAULTFILES:%=%.c)
	-@rm -v $(USER).tar.bz2
	@mkdir -pv __submission_tmp__/katwikOpsys/src
	-@cp -v makefile $(DEFAULTFILES:%=%.c) $(DEFAULTFILES)  __submission_tmp__
	-@cp -v $(KATHDRPATH)/*.h __submission_tmp__/katwikOpsys
	-@cp -v $(KATSRCPATH)/*.c __submission_tmp__/katwikOpsys/src

	@$(SHOULDCLEAR)
	@printf "$(BLUE)=== Creating $(GREEN)$(USER).tar.bz2$(BLUE) ===$(RESET)\n"
	@tar --create --bzip2 --file=$(USER).tar.bz2 -C __submission_tmp__ .
	@rm -rf __submission_tmp__
	@tar --list --bzip2 --file=$(USER).tar.bz2

submit: zip
	@printf "$(BLUE)=== Submitting $(GREEN)$(USER).tar.bz2$(BLUE) to $(GREEN)$(SUBMISSIONPATH)$(BLUE) ===$(RESET)\n"
	@cp $(USER).tar.bz2 $(SUBMISSIONPATH)
	@ls -l $(SUBMISSIONPATH)/$(USER).tar.bz2
	@stat $(SUBMISSIONPATH)/$(USER).tar.bz2