
#include "irig106ch10.h"
#include "unity.h"
#include "unity_fixture.h"
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#ifndef _WIN32
#include <sys/mman.h>
#include <unistd.h>
#endif


TEST_GROUP_RUNNER(test_i106){
    RUN_TEST_CASE(test_i106, TestI106C10Open);
    RUN_TEST_CASE(test_i106, TestI106C10Close);

    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderFile);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderInOrder);
    RUN_TEST_CASE(test_i106, TestI106C10ReadPrevHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadData);
    RUN_TEST_CASE(test_i106, TestI106C10ReadDataFile);
    RUN_TEST_CASE(test_i106, TestI106C10WriteMsg);

    RUN_TEST_CASE(test_i106, TestI106C10FirstMsg);
    RUN_TEST_CASE(test_i106, TestI106C10LastMsg);
    RUN_TEST_CASE(test_i106, TestI106C10SetPos);
    RUN_TEST_CASE(test_i106, TestI106C10GetPos);

    RUN_TEST_CASE(test_i106, TestHeaderInit);
    RUN_TEST_CASE(test_i106, TestGetHeaderLength);
    RUN_TEST_CASE(test_i106, TestGetDataLength);
    RUN_TEST_CASE(test_i106, TestHeaderChecksum);
    // TODO: fix this!
    /* RUN_TEST_CASE(test_i106, TestSecondaryHeaderChecksum); */
    RUN_TEST_CASE(test_i106, TestI106C10ErrorString);
    /* RUN_TEST_CASE(test_i106, TestDataChecksum); */
    RUN_TEST_CASE(test_i106, TestBufferSize);
    RUN_TEST_CASE(test_i106, TestAddFillerAndChecksum);

    /* RUN_TEST_CASE(test_i106, TestMakeInOrderIndex); */
    /* RUN_TEST_CASE(test_i106, TestReadInOrderIndex); */
    /* RUN_TEST_CASE(test_i106, TestWriteInOrderIndex); */
}


TEST_GROUP(test_i106);
TEST_SETUP(test_i106){}
TEST_TEAR_DOWN(test_i106){}


TEST(test_i106, TestI106C10Open){
    int handle;

    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", APPEND));
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", READ));

    I106C10Close(handle);
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

    MakeInOrderIndex(handle);

    handles[handle].FileMode = OVERWRITE;
    TEST_ASSERT_EQUAL(I106_WRONG_FILE_MODE, I106C10ReadNextHeader(handle, &header));

    handles[handle].FileMode = READ_NET_STREAM;
    TEST_ASSERT_EQUAL(I106C10ReadNextHeaderFile(handle, &header), I106C10ReadNextHeader(handle, &header));

    I106C10Open(&handle, "tests/indexed.c10", READ);
    MakeInOrderIndex(handle);
    handles[handle].FileMode = READ_IN_ORDER;
    handles[handle].Index.SortStatus = SORTED;
    TEST_ASSERT_EQUAL(I106C10ReadNextHeaderInOrder(handle, &header), I106C10ReadNextHeader(handle, &header));
    I106C10Close(handle);
}


TEST(test_i106, TestI106C10ReadNextHeaderFile){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeaderFile(handle, &header));

    I106C10Close(handle);
}


TEST(test_i106, TestI106C10ReadNextHeaderInOrder){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    MakeInOrderIndex(handle);

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeaderInOrder(handle, &header));
    I106C10Close(handle);
}


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
    TEST_ASSERT_EQUAL(
            I106C10ReadDataFile(handle, buffer_size, buffer),
            I106C10ReadData(handle, buffer_size, buffer));

    I106C10Close(handle);
    free(buffer);
}


TEST(test_i106, TestI106C10ReadDataFile){
    int handle;
    unsigned long buffer_size = 10000;
    void *buffer = malloc(buffer_size);

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 24));
    handles[handle].File_State = I106_READ_DATA;

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadDataFile(handle, buffer_size, buffer));

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


TEST(test_i106, TestI106C10FirstMsg){
    int handle;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106C10FirstMsg(handle));

    I106C10Close(handle);
}


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


TEST(test_i106, TestHeaderInit){
    I106C10Header header;
    TEST_ASSERT_EQUAL(HeaderInit(&header, 4, 5, 6, 7), 0);
    TEST_ASSERT_EQUAL(4, header.ChannelID);
    TEST_ASSERT_EQUAL(5, header.DataType);
    TEST_ASSERT_EQUAL(6, header.PacketFlags);
    TEST_ASSERT_EQUAL(7, header.SequenceNumber);
}


TEST(test_i106, TestGetHeaderLength){
    I106C10Header header;

    TEST_ASSERT_EQUAL(24, GetHeaderLength(&header));

    header.PacketFlags |= I106CH10_PFLAGS_SEC_HEADER;
    TEST_ASSERT_EQUAL(36, GetHeaderLength(&header));
}


TEST(test_i106, TestGetDataLength){
    I106C10Header header;
    header.PacketLength = 100;
    TEST_ASSERT_EQUAL(76, GetDataLength(&header));
}


TEST(test_i106, TestHeaderChecksum){
    I106C10Header header;
    HeaderInit(&header, 4, 5, 6, 7);

    TEST_ASSERT_EQUAL(63305, HeaderChecksum(&header));
}


TEST(test_i106, TestSecondaryHeaderChecksum){
    void *raw_header = malloc(36);
    I106C10Header header;
    header.PacketFlags |= I106CH10_PFLAGS_SEC_HEADER;
    memset(raw_header, 0, 36);
    memcpy(raw_header, &header, HEADER_SIZE);

    TEST_ASSERT_EQUAL(28073, SecondaryHeaderChecksum(raw_header));

    free(raw_header);
}


TEST(test_i106, TestI106C10ErrorString){
    TEST_ASSERT_EQUAL_STRING("File not open", I106ErrorString(I106_NOT_OPEN));
    TEST_ASSERT_EQUAL_STRING("No index", I106ErrorString(I106_NO_INDEX));
    TEST_ASSERT_EQUAL_STRING("Unknown error", I106ErrorString(1000));
}


TEST(test_i106, TestBufferSize){
    TEST_ASSERT_EQUAL(52, BufferSize(50, 0));
    TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_NONE));
    TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_8));
    TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_16));
    TEST_ASSERT_EQUAL(56, BufferSize(50, I106CH10_PFLAGS_CHKSUM_32));
}


TEST(test_i106, TestAddFillerAndChecksum){
    void *data = malloc(100);
    I106C10Header header;
    TEST_ASSERT_EQUAL(I106_OK, AddFillerAndChecksum(&header, data));

    free(data);
}
