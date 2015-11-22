CPLEX_LIB       = /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic/libcplex.a /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic/libilocplex.a
LP_LIBS         = $(CPLEX_LIB)
INC_DIR		= -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include/ -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include/

CCC = g++

CCFLAGS = -m64 -O2 -fPIC -fexceptions -DNDEBUG -DIL_STD

LDFLAGS = $(LP_LIBS) -lc -lm

SRCPRI = particionar.cpp 

SOURCES = $(SRCPRI)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) 
	$(CCC) -o particionar $(OBJECTS) $(CCFLAGS) $(LDFLAGS) -lpthread -lm -lnsl

#
# produce the .o files from the .cc and .c files
#

%.o: %.cpp
	$(CCC) $(CCFLAGS) $(INC_DIR) -c $<
