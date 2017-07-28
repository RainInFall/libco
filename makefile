CROSS_PLATFORM =
CC = $(CROSS_PLATFORM)gcc
AR = $(CROSS_PLATFORM)ar
CXX = $(CROSS_PLATFORM)g++

INCLUD_PATH = -I ./include
INCLUD_PATH += -I ./src

SRCS = $(wildcard ./src/*.c) $(wildcard ./src/**/*.c)
TEST_SRCS = $(wildcard ./test/*.cpp) $(wildcard ./test/**/*.cpp)

OBJS = $(patsubst %.c,%.o,$(SRCS))
GCNOS = $(patsubst %.c,%.gcno,$(SRCS))
GCDAS = $(patsubst %.c,%.gcda,$(SRCS))
TEST_OBJS = $(patsubst %.cpp,%.o,$(TEST_SRCS))

C_FLAGS = -D__DEBUG -g -Wall --coverage $(INCLUD_PATH)
TEST_FLAGS = -g -Wall $(INCLUD_PATH)

TEST_LD_FLAGS = -L ./lib -l co -l uv -l gtest_main -l gtest -l pthread --coverage

LIB_STATIC = lib/libco.a

TEST_EXE = run_tests

LIB_PATH = lib

LIB_GTEST = $(LIB_PATH)/libgtest_main.a

LIB_UV = $(LIB_PATH)/libuv.a

all: $(LIB_STATIC)

$(LIB_STATIC): $(OBJS)
	$(AR) rcs $(LIB_STATIC) $(OBJS)

$(OBJS): %.o:%.c $(LIB_UV)
	$(CC) $(C_FLAGS) -c $< -o $@

clean:
	rm -rf $(GCDAS) $(GCNOS) $(OBJS) $(LIB_STATIC) $(TEST_EXE) $(TEST_OBJS)

run_test: test
	./$(TEST_EXE)
	lcov -c -o coverage/test.info -d src/
	genhtml coverage/test.info -o coverage

test: $(TEST_EXE)

$(TEST_EXE): $(TEST_OBJS) $(LIB_STATIC)
	$(CXX) -o $(TEST_EXE) $(TEST_OBJS) $(TEST_LD_FLAGS)

$(TEST_OBJS): %.o:%.cpp $(LIB_GTEST) $(LIB_UV)
	$(CXX) $(TEST_FLAGS) -c $< -o $@

$(LIB_GTEST): gtest/build.sh
	gtest/build.sh

$(LIB_UV): libuv/build.sh
	libuv/build.sh
