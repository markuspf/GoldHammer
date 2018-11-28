OPTFLAGS = -Ofast -march=native -mtune=native
CXXFLAGS = $(OPTFLAGS) -std=c++14 -I.
LDFLAGS =
OBJECTS = cg_generator


all : $(OBJECTS)


cg_generator: cg_generator.cpp
		$(CXX) $(CXXFLAGS) $(LDFLAGS) cg_generator.cpp libsemigroups/src/{rws,rwse,report,elements}.cc -o $@

clean:
		rm -vf $(OBJECTS)
