
EXT=

# Try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS), Windows_NT)
	ifeq ($(shell uname -s),) # not in a bash-like shell
		CLEANUP = del /F /Q
		MKDIR = mkdir
	else # in a bash-like shell, like msys
		CLEANUP = rm -f
		MKDIR = mkdir -p
	endif
	EXT =.exe
else
	CLEANUP = rm -rf
	MKDIR = mkdir -p
endif

CC=gcc
# ifeq ($(shell uname -s), Darwin)
# CC=clang
# endif

CFLAGS =  -D_FILE_OFFSET_BITS=64
CFLAGS += -D_LARGEFILE64_SOURCE
CFLAGS += -ggdb
CFLAGS += -fpack-struct=1
CFLAGS += -fPIC
CFLAGS += -Wno-address-of-packed-member

SRC_DIR=src
OBJ_DIR=obj
TEST_DIR=tests

UNITY_ROOT=tests/unity
TEST_INCLUDES=-I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src -I$(SRC_DIR)
TEST_DEPENDENCIES = $(UNITY_ROOT)/src/unity.c $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c
TEST_CASES := $(wildcard $(TEST_DIR)/test_*.c)
# TEST_RUNNERS := $(wildcard $(TEST_DIR)/test_runners/*.c)
TEST_RUNNER = $(TEST_DIR)/run_tests$(EXT)
TESTS = $(TEST_DEPENDENCIES) $(TEST_CASES) $(TEST_RUNNERS) $(TEST_DIR)/all_tests.c

SOURCES := $(wildcard $(SRC_DIR)/*.c)
INCLUDES := $(wildcard $(SRC_DIR)/*.h)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: pre_build libirig106.so libirig106.a

pre_build: $(OBJ_DIR)

$(OBJ_DIR):
	$(MKDIR) $(OBJ_DIR)

libirig106.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wall -o $@ $? -lc

libirig106.a: $(OBJECTS)
	ar rc $@ $?

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	$(CLEANUP) libirig106.so libirig106.a && \
	$(CLEANUP) obj && \
	$(CLEANUP) $(TEST_RUNNER) && \
	$(CLEANUP) *.dSYM && \
	$(CLEANUP) $(TEST_DIR)/*.dSYM

test:
	$(CC) $(CFLAGS) $(TEST_INCLUDES) $(TESTS) $(SOURCES) -o $(TEST_RUNNER)
	- ./$(TEST_RUNNER) -v
