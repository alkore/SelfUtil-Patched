CXX := clang++
CXXFLAGS := -std=c++17 -O2 -Wall -Wextra
TARGET := selfutil
SRC := selfutil.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(TARGET)
