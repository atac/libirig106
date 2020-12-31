
#include "libirig106.h"
#include "unity.h"
#include "unity_fixture.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif


TEST_GROUP_RUNNER(test_i106){
    // New API
    RUN_TEST_CASE(test_i106, TestI106NextHeader);
    RUN_TEST_CASE(test_i106, TestI106NextHeaderBuffer);
    RUN_TEST_CASE(test_i106, TestI106PrevHeader);

    // Old API
    RUN_TEST_CASE(test_i106, TestI106C10Open);
    RUN_TEST_CASE(test_i106, TestI106C10OpenBuffer);
    RUN_TEST_CASE(test_i106, TestI106C10Close);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderFile);
    /* RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderInOrder); */
    RUN_TEST_CASE(test_i106, TestI106C10ReadPrevHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadData);
    RUN_TEST_CASE(test_i106, TestI106C10ReadDataFile);
    RUN_TEST_CASE(test_i106, TestI106C10WriteMsg);

    /* RUN_TEST_CASE(test_i106, TestI106C10FirstMsg); */
    RUN_TEST_CASE(test_i106, TestI106C10LastMsg);
    RUN_TEST_CASE(test_i106, TestI106C10SetPos);
    RUN_TEST_CASE(test_i106, TestI106C10GetPos);

    /* RUN_TEST_CASE(test_i106, TestHeaderInit); */
    /* RUN_TEST_CASE(test_i106, TestGetHeaderLength); */
    /* RUN_TEST_CASE(test_i106, TestGetDataLength); */
    /* RUN_TEST_CASE(test_i106, TestHeaderChecksum); */
    /* RUN_TEST_CASE(test_i106, TestSecondaryHeaderChecksum); */
    /* RUN_TEST_CASE(test_i106, TestI106C10ErrorString); */
    /* RUN_TEST_CASE(test_i106, TestDataChecksum); */
    /* RUN_TEST_CASE(test_i106, TestBufferSize); */
    /* RUN_TEST_CASE(test_i106, TestAddFillerAndChecksum); */

    /* RUN_TEST_CASE(test_i106, TestMakeInOrderIndex); */
    /* RUN_TEST_CASE(test_i106, TestReadInOrderIndex); */
    /* RUN_TEST_CASE(test_i106, TestWriteInOrderIndex); */
}


TEST_GROUP(test_i106);
TEST_SETUP(test_i106){}
TEST_TEAR_DOWN(test_i106){}


// New API
TEST(test_i106, TestI106NextHeader){
    I106C10Header header;
    int fd = open("tests/indexed.c10", 0);

    TEST_ASSERT_EQUAL(I106_OK, I106NextHeader(fd, &header));
    TEST_ASSERT_EQUAL(0, header.ChannelID);
    TEST_ASSERT_EQUAL(1, header.DataType);
}


TEST(test_i106, TestI106NextHeaderBuffer){
    I106C10Header header;
    int fd = open("tests/indexed.c10", 0);
    void *buffer = malloc(100);
    int read_count = read(fd, buffer, 100);

    TEST_ASSERT_EQUAL(I106_OK, I106NextHeaderBuffer(buffer, 100, 0, &header));
    TEST_ASSERT_EQUAL(0, header.ChannelID);
    TEST_ASSERT_EQUAL(1, header.DataType);

    free(buffer);
}


TEST(test_i106, TestI106PrevHeader){
    I106C10Header header;
    int fd = open("tests/indexed.c10", 0);
    lseek(fd, 0, SEEK_END);

    TEST_ASSERT_EQUAL(I106_OK, I106PrevHeader(fd, &header));
    TEST_ASSERT_EQUAL(0, header.ChannelID);
    TEST_ASSERT_EQUAL(3, header.DataType);
}


TEST(test_i106, TestI106PrevHeaderBuffer){
    I106C10Header header;
    int fd = open("tests/indexed.c10", 0);
    void *buffer = malloc(100);
    lseek(fd, -100, SEEK_END);
    read(fd, buffer, 100);

    TEST_ASSERT_EQUAL(I106_OK, I106PrevHeaderBuffer(buffer, 100, 100, &header));
    TEST_ASSERT_EQUAL(0, header.ChannelID);
    TEST_ASSERT_EQUAL(3, header.DataType);
}


// Old API
TEST(test_i106, TestI106C10Open){
    int handle;

    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", APPEND));
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", READ));

    I106C10Close(handle);
}


void clear_handles(){
    for (int i=0;i<MAX_HANDLES;i++){
        I106C10Close(i);
    }
}


TEST(test_i106, TestI106C10OpenBuffer){
    int handle;
    void *buffer = malloc(100);

    // Test opening works
    TEST_ASSERT_EQUAL(I106_OK, I106C10OpenBuffer(&handle, buffer, 100, READ));

    // Test overflowing MAX_HANDLES produces the expected result.
    clear_handles();
    for (int i=0;i<100;i++){
        TEST_ASSERT_EQUAL(I106_OK, I106C10OpenBuffer(&handle, buffer, 100, READ));
    }
    TEST_ASSERT_EQUAL(I106_NO_FREE_HANDLES, I106C10OpenBuffer(&handle, buffer, 100, READ));

    // Test opening works again after clearing handles.
    clear_handles();
    TEST_ASSERT_EQUAL(I106_OK, I106C10OpenBuffer(&handle, buffer, 100, READ));

    free(buffer);
}


TEST(test_i106, TestI106C10Close){
    int handle;
    TEST_ASSERT_EQUAL(I106_INVALID_HANDLE, I106C10Close(-1));
    I106C10Open(&handle, "tests/copy.c10", READ);
    TEST_ASSERT_EQUAL(I106_OK, I106C10Close(handle));
}


TEST(test_i106, TestI106C10ReadNextHeader){
    int handle = 0;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    /* MakeInOrderIndex(handle); */

    handles[handle].FileMode = OVERWRITE;
    TEST_ASSERT_EQUAL(I106_WRONG_FILE_MODE, I106C10ReadNextHeader(handle, &header));

    /* handles[handle].FileMode = READ_IN_ORDER; */
    /* handles[handle].Index.SortStatus = SORTED; */
    /* TEST_ASSERT_EQUAL(I106C10ReadNextHeaderInOrder(handle, &header), I106C10ReadNextHeader(handle, &header)); */
    I106C10Close(handle);
}


TEST(test_i106, TestI106C10ReadNextHeaderFile){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeader(handle, &header));

    I106C10Close(handle);
}


/* TEST(test_i106, TestI106C10ReadNextHeaderInOrder){ */
/*     int handle; */
/*     I106C10Header header; */

/*     TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ)); */

/*     /1* MakeInOrderIndex(handle); *1/ */

/*     TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeaderInOrder(handle, &header)); */
/*     I106C10Close(handle); */
/* } */


TEST(test_i106, TestI106C10ReadPrevHeader){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 10000));
    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadPrevHeader(handle, &header));

    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 0));
    TEST_ASSERT_EQUAL(I106_BOF, I106C10ReadPrevHeader(handle, &header));

    TEST_ASSERT_EQUAL(I106_OK, I106C10Close(handle));
}


TEST(test_i106, TestI106C10ReadData){
    int handle;
    unsigned long buffer_size = 100;
    void *buffer = malloc(buffer_size);

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    /* TEST_ASSERT_EQUAL( */
    /*         I106C10ReadDataFile(handle, buffer_size, buffer), */
    /*         I106C10ReadData(handle, buffer_size, buffer)); */

    I106C10Close(handle);
    free(buffer);
}


TEST(test_i106, TestI106C10ReadDataFile){
    int handle;
    unsigned long buffer_size = 100000;
    void *buffer = malloc(buffer_size);

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 24));
    handles[handle].File_State = I106_READ_DATA;

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadData(handle, buffer_size, buffer));

    I106C10Close(handle);
    free(buffer);
}


TEST(test_i106, TestI106C10WriteMsg){
    int handle;
    I106C10Header header;
    void *buffer = malloc(100);
    header.DataLength = 100;
    header.PacketLength = 124;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/tmp.c10", OVERWRITE));

    TEST_ASSERT_EQUAL(I106_OK, I106C10WriteMsg(handle, &header, buffer));

    unlink("tests/tmp.c10");
    I106C10Close(handle);
    free(buffer);
}


/* TEST(test_i106, TestI106C10FirstMsg){ */
/*     int handle; */
/*     TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ)); */
/*     TEST_ASSERT_EQUAL(I106_OK, I106C10FirstMsg(handle)); */

/*     I106C10Close(handle); */
/* } */


TEST(test_i106, TestI106C10LastMsg){
    int handle;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106C10LastMsg(handle));
    I106C10Close(handle);
}


TEST(test_i106, TestI106C10SetPos){
    int handle;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 1000));
    off_t pos = lseek(handles[handle].File, 0, SEEK_CUR);
    TEST_ASSERT_EQUAL(pos, 1000);

    I106C10Close(handle);
}


TEST(test_i106, TestI106C10GetPos){
    int handle;
    int64_t offset;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    lseek(handles[handle].File, 1500, SEEK_SET);

    TEST_ASSERT_EQUAL(I106_OK, I106C10GetPos(handle, &offset));
    TEST_ASSERT_EQUAL(1500, offset);

    I106C10Close(handle);
}


