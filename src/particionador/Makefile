CXX = g++
CXXFLAGS = -m64 -O2 -fPIC -fexceptions -DNDEBUG -DIL_STD -Wall -Wextra -std=c++11
SRCPRI = particionar.cpp 

SOURCES = $(SRCPRI)

OBJECTS = $(SOURCES:.cpp=.o)

all: $(OBJECTS) 
	$(CXX) -o particionar $(OBJECTS) $(CXXFLAGS) -lm

#
# produce the .o files from the .cc and .c files
#

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<
