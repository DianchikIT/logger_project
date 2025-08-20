CXX := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -fPIC -finput-charset=UTF-8
LDFLAGS := -shared

LIB_NAME := liblogger.so
LIB_SRC := src/logger.cpp
LIB_HEADER := include/logger.h

APP_NAME := logger_app
APP_SRC := src/main.cpp

TEST_NAME := test_logger
TEST_SRC := tests/test_logger.cpp

.PHONY: all clean

all: $(LIB_NAME) $(APP_NAME)

$(LIB_NAME): $(LIB_SRC) $(LIB_HEADER)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -Iinclude -o $@ $(LIB_SRC)

$(APP_NAME): $(APP_SRC) $(LIB_NAME)
	$(CXX) $(CXXFLAGS) -Iinclude -L. -o $@ $(APP_SRC) -llogger -pthread

$(TEST_NAME): $(TEST_SRC) $(LIB_NAME)
	$(CXX) $(CXXFLAGS) -Iinclude -L. -o $@ $(TEST_SRC) -llogger -pthread

run-test: $(TEST_NAME)
	LD_LIBRARY_PATH=. ./$<

clean:
	rm -f $(LIB_NAME) $(APP_NAME) $(TEST_NAME)
