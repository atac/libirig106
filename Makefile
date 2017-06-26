
UNITY_ROOT=tests/unity
TEST_RUNNER=run_tests

# Try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS), Windows_NT)
	ifeq ($(shell uname -s),) # not in a bash-like shell
		CLEANUP = del /F /Q
		MKDIR = mkdir
	else # in a bash-like shell, like msys
		CLEANUP = rm -f
		MKDIR = mkdir -p
	endif
	TEST_RUNNER +=.exe
else
	CLEANUP = rm -rf
	MKDIR = mkdir -p
endif

CC=gcc
ifeq ($(shell uname -s), Darwin)
CC=clang
endif

CFLAGS =  -D_FILE_OFFSET_BITS=64
CFLAGS += -D_LARGEFILE64_SOURCE
CFLAGS += -ggdb
CFLAGS += -fpack-struct=1
CFLAGS += -fPIC
CFLAGS += -Wno-address-of-packed-member

TEST_SRC=\
	$(UNITY_ROOT)/src/unity.c \
	$(UNITY_ROOT)/extras/fixture/src/unity_fixture.c \
	tests/src/ProductionCode.c \
	tests/src/ProductionCode2.c \
	tests/TestProductionCode.c \
	tests/TestProductionCode2.c \
	tests/test_runners/TestProductionCode_Runner.c \
	tests/test_runners/TestProductionCode2_Runner.c \
	tests/test_runners/all_tests.c
INC_DIRS=-Itests/src -I$(UNITY_ROOT)/src -I$(UNITY_ROOT)/extras/fixture/src

SRC_DIR=src
OBJ_DIR=obj

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
	$(CLEANUP) *.dSYM

test:
	$(CC) $(CFLAGS) $(INC_DIRS) $(TEST_SRC) -o $(TEST_RUNNER)
	- ./$(TEST_RUNNER) -v
