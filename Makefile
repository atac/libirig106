
EXT=
CLEANUP = rm -rf
MKDIR = mkdir -p

# Try to detect the OS we are running on, and adjust commands as needed
ifeq ($(OS), Windows_NT)
	ifeq ($(shell uname -s),) # not in a bash-like shell
		CLEANUP = del /F /Q
		MKDIR = mkdir
	endif
	EXT =.exe
else
endif

CC=gcc
LINK=gcc
DEPEND=gcc -MM -MG -MF
CFLAGS=

# Just use GCC on mac for now
# ifeq ($(shell uname -s), Darwin)
# CC=gcc-7
# endif

ifeq ($(OS), Windows_NT)
CFLAGS += -D_MSC_VER
endif
ifneq ($(OS), DARWIN)
CFLAGS += -D_FILE_OFFSET_BITS=64
CFLAGS += -D_LARGEFILE64_SOURCE
endif
CFLAGS += -ggdb
CFLAGS += -fpack-struct=1
CFLAGS += -fPIC
CFLAGS += -std=c99
CFLAGS += -Wall

UNITY_ROOT=tests/unity
SRC_DIR=src
OBJ_DIR=obj
TEST_DIR=tests

# Includes
CFLAGS += -I$(UNITY_ROOT)/src
CFLAGS += -I$(UNITY_ROOT)/extras/fixture/src
CFLAGS += -I$(UNITY_ROOT)/extras/memory/src
CFLAGS += -Iinclude

UNITY := $(UNITY_ROOT)/src/unity.c $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c $(UNITY_ROOT)/extras/memory/src/unity_memory.c
UNITY_OBJ := $(OBJ_DIR)/unity.o $(OBJ_DIR)/unity_fixture.o $(OBJ_DIR)/unity_memory.o

TESTS := $(wildcard $(TEST_DIR)/*.c)
TEST_RUNNERS := $(TESTS:$(TEST_DIR)/test_%.c=$(TEST_DIR)/test_%$(EXT))
TEST_RUNNER = $(TEST_DIR)/run_tests$(EXT)

TEST_SOURCES = $(TEST_DEPENDENCIES) $(TESTS) $(TEST_DIR)/all_tests.c
TEST_OBJ := $(TESTS:$(TEST_DIR)/%.c=$(OBJ_DIR)/%.o)

SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: mkdirs libirig106.so libirig106.a

mkdirs:
	$(MKDIR) $(OBJ_DIR)

libirig106.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wall -o $@ $? -lc

libirig106.a: $(OBJECTS)
	ar rc $@ $?

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(TEST_OBJ): $(OBJ_DIR)/%.o : $(TEST_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(UNITY_OBJ):
	$(CC) $(CFLAGS) -c $(UNITY_ROOT)/src/unity.c -o $(OBJ_DIR)/unity.o
	$(CC) $(CFLAGS) -c $(UNITY_ROOT)/extras/fixture/src/unity_fixture.c -o $(OBJ_DIR)/unity_fixture.o
	$(CC) $(CFLAGS) -c $(UNITY_ROOT)/extras/memory/src/unity_memory.c -o $(OBJ_DIR)/unity_memory.o

clean:
	$(CLEANUP) libirig106.so libirig106.a && \
	$(CLEANUP) obj && \
	$(CLEANUP) $(TEST_RUNNER) && \
	$(CLEANUP) $(TEST_DIR)/*.dSYM

$(OBJ_DIR)/all_tests.obj: $(TEST_DIR)/all_tests.c
	$(CC) $(CFLAGS) -c $< -o $@ 

$(TEST_RUNNER): $(OBJ_DIR)/all_tests.o $(TEST_OBJ) $(UNITY_OBJ) $(OBJECTS)
	$(LINK) -o $@ $^

test: mkdirs $(TEST_RUNNER)
	./$(TEST_RUNNER)

# Runs tests within docker using the Dockerfile.
docker:
	docker build . --no-cache
