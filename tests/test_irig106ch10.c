
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "irig106ch10.h"
#include "unity.h"
#include "unity_fixture.h"
#include <assert.h>


TEST_GROUP_RUNNER(test_i106){
    RUN_TEST_CASE(test_i106, TestI106C10Open);
    RUN_TEST_CASE(test_i106, TestI106C10Close);

    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderFile);
    RUN_TEST_CASE(test_i106, TestI106C10ReadNextHeaderInOrder);
    RUN_TEST_CASE(test_i106, TestI106C10ReadPrevHeader);
    RUN_TEST_CASE(test_i106, TestI106C10ReadData);
    /* RUN_TEST_CASE(test_i106, TestI106C10ReadDataFile); */
    /* RUN_TEST_CASE(test_i106, TestI106C10WriteMsg); */

    /* RUN_TEST_CASE(test_i106, TestI106C10FirstMsg); */
    /* RUN_TEST_CASE(test_i106, TestI106C10LastMsg); */
    /* RUN_TEST_CASE(test_i106, TestI106C10SetPos); */
    /* RUN_TEST_CASE(test_i106, TestI106C10GetPos); */

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


TEST(test_i106, TestI106C10Open){
    int handle;

    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", APPEND));
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(I106_OPEN_ERROR, I106C10Open(&handle, "not-a-thing.c10", READ));
}


TEST(test_i106, TestI106C10Close){
    TEST_ASSERT_EQUAL(I106_INVALID_HANDLE, I106C10Close(-1));
    TEST_ASSERT_EQUAL(I106_OK, I106C10Close(2));
}


TEST(test_i106, TestI106C10ReadNextHeader){
    int handle = 0;
    I106C10Header header;

    handles[handle].FileMode = OVERWRITE;
    TEST_ASSERT_EQUAL(I106_WRONG_FILE_MODE, I106C10ReadNextHeader(handle, &header));

    handles[handle].FileMode = READ_NET_STREAM;
    TEST_ASSERT_EQUAL(I106C10ReadNextHeaderFile(handle, &header), I106C10ReadNextHeader(handle, &header));

    handles[handle].FileMode = READ_IN_ORDER;
    handles[handle].Index.SortStatus = SORTED;
    TEST_ASSERT_EQUAL(I106C10ReadNextHeaderInOrder(handle, &header), I106C10ReadNextHeader(handle, &header));
}


TEST(test_i106, TestI106C10ReadNextHeaderFile){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeaderFile(handle, &header));
}


TEST(test_i106, TestI106C10ReadNextHeaderInOrder){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    MakeInOrderIndex(handle);

    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadNextHeaderInOrder(handle, &header));
}


TEST(test_i106, TestI106C10ReadPrevHeader){
    int handle;
    I106C10Header header;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 10000));
    TEST_ASSERT_EQUAL(I106_OK, I106C10ReadPrevHeader(handle, &header));

    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPos(handle, 0));
    TEST_ASSERT_EQUAL(I106_BOF, I106C10ReadPrevHeader(handle, &header));
}



TEST(test_i106, TestI106C10ReadData){
    int handle;
    unsigned long buffer_size = 100;
    void *buffer = malloc(buffer_size);

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));
    TEST_ASSERT_EQUAL(
            I106C10ReadDataFile(handle, buffer_size, buffer),
            I106C10ReadData(handle, buffer_size, buffer));

    free(buffer);
}
