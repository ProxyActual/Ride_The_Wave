 CXX ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra

.PHONY: all RobinhoodConnectorTest clean

all: RobinhoodConnectorTest

TEST_SRC := RobinhoodConnectorTest.cpp \
            RobinhoodConnector.cpp

RobinhoodConnectorTest: $(TEST_SRC)
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -f RobinhoodConnectorTest
