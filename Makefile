CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra

COMMON = figlet.o ofigstream.o

all: hello test

hello: hello.o $(COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@

test: test.o $(COMMON)
	$(CXX) $(CXXFLAGS) $^ -o $@

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

hello.o:      hello.cpp figlet.h ofigstream.h
figlet.o:     figlet.cpp figlet.h
ofigstream.o: ofigstream.cpp ofigstream.h figlet.h
test.o:       test.cpp figlet.h ofigstream.h

run: hello
	./hello

check: test
	./test

clean:
	rm -f *.o hello test

.PHONY: all run check clean
