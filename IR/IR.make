#  Makefile "IR.make", by KWR for the "Intrinsic Ratings" project
#
#  USAGE---enter at command prompt in directory:  make -f IR.make


# Unix Macros---choose which compiler and options here!

CC     = g++ -O5     #optional change to: g++ -Wall
# CC     = g++ -fhandle-exceptions -frtti -O5
# CC     = CC -fast  # name of the Sun C++ compiler---NOT avail. on timberlake
                     # if you have a command-line installation at home (as
                     # opposed to an IDE like Eclipse), put your compiler here.

#FLAGS	= -I/util/gsl/include -D_LINK_ME_
FLAGS	= -I/util/gsl/include -I/util/cminpack/include -L/util/gsl/lib -L/util/cminpack/lib
         # last is for g++ on castor/timberlake/etc.
LIBS	= -lgsl -lgslcblas -lcminpack -lm
OBJ    = .o
RM     = rm -fr


# SYNTAX: Dependencies must be on same line as ":".  Actions must
# follow on succeeding line(s), and have at least one TAB indent.
# Hence, be careful mouse-copying makefile text if tabs convert to spaces.

all:	IR

# Invoke this via "make -f IR.make clean".  No "-" before "clean"!
clean:
	$(RM) *$(OBJ)    # ISR/*$(OBJ)---etc. for any subdirectories

# Below I hard-coded ".o" in place of the variable "$(OBJ)" to save room.

PerfData.o: PerfData.cpp PerfData.h  
	$(CC) $(FLAGS) -c PerfData.cpp

Trial.o: Trial.cpp Trial.h
	$(CC) $(FLAGS) -c Trial.cpp

Minimizer.o: Minimizer.cpp Minimizer.h
	$(CC) $(FLAGS) -c Minimizer.cpp

Ensemble.o: Ensemble.cpp Ensemble.h
	$(CC) $(FLAGS) -c Ensemble.cpp

Demos.o: Demos.cpp Demos.h
	$(CC) $(FLAGS) -c Demos.cpp

IRmain.o: IRmain.cpp
	$(CC) $(FLAGS) -c IRmain.cpp

IR: IRmain.o Demos.o Ensemble.o Minimizer.o Trial.o PerfData.o 
	$(CC) $(FLAGS) -o IRx IRmain.o Demos.o Ensemble.o Minimizer.o Trial.o PerfData.o $(LIBS)

# The .o file with main should come before all the other object files in the
# final linking stage.

