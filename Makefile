
CC=gcc
CFLAGS=\
	   -D_FILE_OFFSET_BITS=64\
	   -D_LARGEFILE64_SOURCE\
	   -ggdb\
	   -fpack-struct=1\
	   -fPIC\
	   -Wno-address-of-packed-member\
	   -Wno-format\
	   -Wno-switch

SRC_DIR=src
OBJ_DIR=obj

SOURCES := $(wildcard $(SRC_DIR)/*.c)
INCLUDES := $(wildcard $(SRC_DIR)/*.h)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

all: pre_build libirig106.so libirig106.a

pre_build: $(OBJ_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

libirig106.so: $(OBJECTS)
	$(CC) -shared -fPIC -Wall -o $@ $? -lc

libirig106.a: $(OBJECTS)
	ar rc $@ $?

$(OBJECTS): $(OBJ_DIR)/%.o : $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@ 

clean:
	$(MAKE) -C test clean && \
	rm libirig106.so libirig106.a && \
	rm -rf obj

tests:
	$(MAKE) -C test
