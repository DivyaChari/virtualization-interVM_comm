# file : Makefile
# desc : Builds "ne_upci_rw" 
#
# Example usage:
#  make                 (compile with "gcc(1)" optimization)
#  make V=1             (echo "gcc(1)" command-line)
#  make DEBUG=1         (compile with "gcc(1)" debugging info)
#  make CFLAGS=-DDEBUG  (define "DEBUG" in sources)

#shmem_wo_intr.c
PCIRWSRCS := uspace_guest.c shmem_wo_intr_wrapper.c shmem_wo_intr_wrapper.h 
PCIRWEXEC := uspace
CC := gcc

override CFLAGS += -I. -Wall -Wextra -fno-common 
ifdef DEBUG
EXTRA_CFLAGS += -g3
else
EXTRA_CFLAGS += -O2
endif

ECHO=@echo
ifeq ($V,1)
	Q =
else
	Q = @
endif

default : $(PCIRWEXEC)

$(PCIRWEXEC) : $(PCIRWSRCS)
	$(Q)$(ECHO) Building $(notdir $@)
	$(Q)$(CC) $(CFLAGS) $(EXTRA_CFLAGS) $(LDFLAGS) -o $@ $^

clean : 
	rm -rf $(PCIRWEXEC)

.PHONY : clean
