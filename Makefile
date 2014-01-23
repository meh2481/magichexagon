# Entry point for magichexagon makefiles
# Based loosely off FreeImage makefiles - thanks, doods
# Default to 'make -f Makefile.unix' for Linux and for unknown OS. 
#
OS = $(shell uname -s)
MAKEFILE = unix

ifeq ($(OS), Darwin)
    MAKEFILE = osx
endif
ifneq (,$(findstring MINGW,$(OS)))
    MAKEFILE = mingw
endif

default:
	$(MAKE) -f Makefile.$(MAKEFILE) 

all:
	$(MAKE) -f Makefile.$(MAKEFILE) all 

clean:
	$(MAKE) -f Makefile.$(MAKEFILE) clean 

release:
	make "BUILD=release"

