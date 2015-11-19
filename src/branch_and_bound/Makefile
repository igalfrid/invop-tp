CPLEX_LIB       = /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic/libcplex.a /opt/ibm/ILOG/CPLEX_Studio126/cplex/lib/x86-64_linux/static_pic/libilocplex.a
LP_LIBS         = $(CPLEX_LIB)
INC_DIR		= -I/opt/ibm/ILOG/CPLEX_Studio126/cplex/include/ -I/opt/ibm/ILOG/CPLEX_Studio126/concert/include/

CXX = g++

CXXFLAGS = -m64 -O2 -fPIC -fexceptions -DNDEBUG -DIL_STD -std=c++11 -Wall -Wextra -pedantic

LDFLAGS = $(LP_LIBS) -lc -lm

DEPS = Grafo.h Arista.h Particion.h

SRCPRI = main.cpp Grafo.cpp Arista.cpp Particion.cpp

OUTPUT = prog

SOURCES = $(SRCPRI)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) 
	$(CXX) -o $(OUTPUT) $(OBJECTS) $(CXXFLAGS) $(LDFLAGS) -lpthread -lm -lnsl

#
# produce the .o files from the .cc and .c files
#

%.o: %.cpp $(DEPS)
	$(CXX) $(CXXFLAGS) $(INC_DIR) -c $<

clean:
	rm -f *.o
