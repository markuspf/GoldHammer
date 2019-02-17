OPTFLAGS = -Ofast -march=native -mtune=native
# OPTFLAGS = -g3
CXXFLAGS = $(OPTFLAGS) -std=c++14 -I. -fopt-info -Wall -Wextra -Wno-sign-compare -Wno-unused-result -Wno-unused-variable
LDFLAGS =
OBJECTS = cg_generator ss_generator ss_transform ss_cat


all : $(OBJECTS)


RWS_DEPS = libsemigroups/src/rws.cc libsemigroups/src/rwse.cc libsemigroups/src/report.cc libsemigroups/src/elements.cc
cg_generator: cg_generator.cpp StringSystem.hpp CayleyGraph.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) cg_generator.cpp $(RWS_DEPS) -o $@

ss_generator: ss_generator.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_generator.cpp $(RWS_DEPS) -o $@

ss_transform: ss_transform.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_transform.cpp -o $@

ss_cat: ss_cat.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_cat.cpp -o $@

clean:
	rm -vf $(OBJECTS)
