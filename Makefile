CXX      = clang++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iselfutil_patched/compat

SRC = $(wildcard selfutil_patched/*.cpp)
OUT = selfutil

all:
	$(CXX) $(CXXFLAGS) $(SRC) -o $(OUT)

clean:
	rm -f $(OUT)
