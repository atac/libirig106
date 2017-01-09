
# Platform-specific commands
ifeq ($(OSTYPE),cygwin)
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=
elseifeq ($(OSTYPE),msys)
	CLEANUP = rm -f
	MKDIR = mkdir -p
	TARGET_EXTENSION=.exe
elseifeq ($(OS),Windows_NT)
	CLEANUP = del /F /Q
	MKDIR = mkdir
	TARGET_EXTENSION=.exe
else
	CLEANUP = rm -rf
	MKDIR = mkdir -p
	TARGET_EXTENSION=
endif

UNITY_ROOT=unity

CFLAGS = -std=c99
CFLAGS += -Wall
CFLAGS += -Wextra
CFLAGS += -Werror 
CFLAGS += -Wpointer-arith
CFLAGS += -Wcast-align
CFLAGS += -Wwrite-strings
CFLAGS += -Wswitch-default
CFLAGS += -Wunreachable-code
CFLAGS += -Winit-self
CFLAGS += -Wmissing-field-initializers
CFLAGS += -Wno-unknown-pragmas
CFLAGS += -Wstrict-prototypes
CFLAGS += -Wundef
CFLAGS += -Wold-style-definition

TARGET = run_tests$(TARGET_EXTENSION)
ALL_SOURCES = $(UNITY_ROOT)/src/unity.c src/ProductionCode.c TestProductionCode.c  test_runners/TestProductionCode_Runner.c
INC_DIRS=-Isrc -I$(UNITY_ROOT)/src
SYMBOLS=-DTEST

all: clean default

default:
	mkdir test_runners
	ruby $(UNITY_ROOT)/auto/generate_test_runner.rb TestProductionCode.c  test_runners/TestProductionCode_Runner.c
	gcc $(CFLAGS) $(INC_DIRS) $(SYMBOLS) $(ALL_SOURCES) -o $(TARGET)
	./$(TARGET)

clean:
	$(CLEANUP) $(TARGET) test_runners
