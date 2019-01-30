OPTFLAGS = -Ofast -march=native -mtune=native
# OPTFLAGS = -g3
CXXFLAGS = $(OPTFLAGS) -std=c++14 -I. -fopt-info
LDFLAGS =
OBJECTS = cg_generator


all : $(OBJECTS)


DEPS = libsemigroups/src/rws.cc libsemigroups/src/rwse.cc libsemigroups/src/report.cc libsemigroups/src/elements.cc
cg_generator: cg_generator.cpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) cg_generator.cpp $(DEPS) -o $@

clean:
	rm -vf $(OBJECTS)
