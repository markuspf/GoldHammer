OPTFLAGS = -Ofast -march=native -mtune=native
# OPTFLAGS = -g3
CXXFLAGS = $(OPTFLAGS) -std=c++14 -I. -fopt-info
LDFLAGS =
OBJECTS = cg_generator ss_generator ss_transform ss_cat


all : $(OBJECTS)


DEPS = StringSystem.hpp libsemigroups/src/rws.cc libsemigroups/src/rwse.cc libsemigroups/src/report.cc libsemigroups/src/elements.cc
cg_generator: cg_generator.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) cg_generator.cpp $(DEPS) -o $@

ss_generator: ss_generator.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_generator.cpp -o $@

ss_transform: ss_transform.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_transform.cpp -o $@

ss_cat: ss_cat.cpp StringSystem.hpp Makefile
	$(CXX) $(CXXFLAGS) $(LDFLAGS) ss_cat.cpp -o $@

clean:
	rm -vf $(OBJECTS)
