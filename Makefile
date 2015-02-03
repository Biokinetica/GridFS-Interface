#
# 'make depend' uses makedepend to automatically generate dependencies 
#               (dependencies are added to end of Makefile)
# 'make'        build library
# 'make clean'  removes all .o and executable files
#

# define the C++ compiler to use
GCC = g++

COPY = cp

AR = ar

ARFLAGS = -r -s

SHARED = -shared

SHAREDLIB = libmongointerface.so

INSTALL_PATH?=/usr/local

LIB?=/lib

INCLUDE?=/include

# define any compile-time flags
CXXFLAGS = -std=c++11 -fPIC -O2

# define any directories containing header files other than /usr/include
INCLUDES = -I/usr/include/cryptopp

# define the C++ source files
SRCS = $(wildcard *.cpp)

# define the C++ object files 
#
# This uses Suffix Replacement within a macro:
#   $(name:string1=string2)
#         For each word in 'name' replace 'string1' with 'string2'
# Below we are replacing the suffix .cpp of all words in the macro SRCS
# with the .o suffix
#
OBJS = $(SRCS:.cpp=.o)

# define the library file 
MAIN = libmongointerface.a

#
# The following part of the makefile is generic; it can be used to 
# build any executable just by changing the definitions above and by
# deleting dependencies appended to the file from 'make depend'
#

.PHONY: depend clean

all:	$(MAIN)
	@echo  Simple library libmongointerface.a has been compiled

$(MAIN):	 $(OBJS) 
	$(AR) $(ARFLAGS) $(MAIN) $(OBJS)
shared:		$(OBJS)
	$(GCC) $(SHARED) -o $(SHAREDLIB) $(OBJS)
install:	; $(COPY) $(SHAREDLIB) $(INSTALL_PATH)$(LIB) ; $(COPY) $(wildcard *.h) $(INSTALL_PATH)$(INCLUDE)

# this is a suffix replacement rule for building .o's from .cpp's
# it uses automatic variables $<: the name of the prerequisite of
# the rule(a .cpp file) and $@: the name of the target of the rule (a .o file) 
# (see the gnu make manual section about automatic variables)
.cpp.o:
	$(GCC) $(CXXFLAGS) $(INCLUDES) -c $<  -o $@

clean:
	$(RM) *.o *~ $(MAIN) $(SHAREDLIB)

depend: $(SRCS)
	makedepend $(INCLUDES) $^

# DO NOT DELETE THIS LINE -- make depend needs it
