
#include "libirig106.h"
#include "i106_util.h"
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


TEST_GROUP_RUNNER(test_util){

    RUN_TEST_CASE(test_util, TestHeaderInit);
    RUN_TEST_CASE(test_util, TestGetHeaderLength);
    RUN_TEST_CASE(test_util, TestGetDataLength);
    RUN_TEST_CASE(test_util, TestHeaderChecksum);
    /* RUN_TEST_CASE(test_util, TestSecondaryHeaderChecksum); */
    RUN_TEST_CASE(test_util, TestI106C10ErrorString);
    /* RUN_TEST_CASE(test_util, TestDataChecksum); */
    /* RUN_TEST_CASE(test_util, TestBufferSize); */
    /* RUN_TEST_CASE(test_util, TestAddFillerAndChecksum); */

    /* RUN_TEST_CASE(test_util, TestMakeInOrderIndex); */
    /* RUN_TEST_CASE(test_util, TestReadInOrderIndex); */
    /* RUN_TEST_CASE(test_util, TestWriteInOrderIndex); */
}


TEST_GROUP(test_util);
TEST_SETUP(test_util){}
TEST_TEAR_DOWN(test_util){}


TEST(test_util, TestHeaderInit){
    I106C10Header header;
    TEST_ASSERT_EQUAL(HeaderInit(&header, 4, 5, 6, 7), 0);
    TEST_ASSERT_EQUAL(4, header.ChannelID);
    TEST_ASSERT_EQUAL(5, header.DataType);
    TEST_ASSERT_EQUAL(6, header.PacketFlags);
    TEST_ASSERT_EQUAL(7, header.SequenceNumber);
}


TEST(test_util, TestGetHeaderLength){
    I106C10Header header;
    header.PacketFlags = 0;

    TEST_ASSERT_EQUAL(24, GetHeaderLength(&header));

    header.PacketFlags |= I106CH10_PFLAGS_SEC_HEADER;
    TEST_ASSERT_EQUAL(36, GetHeaderLength(&header));
}


TEST(test_util, TestGetDataLength){
    I106C10Header header;
    header.PacketFlags = 0;
    header.PacketLength = 100;
    TEST_ASSERT_EQUAL(76, GetDataLength(&header));
}


TEST(test_util, TestHeaderChecksum){
    I106C10Header header;
    HeaderInit(&header, 4, 5, 6, 7);

    TEST_ASSERT_EQUAL(63305, HeaderChecksum(&header));
}


TEST(test_util, TestSecondaryHeaderChecksum){
    void *raw_header = malloc(36);
    I106C10Header header;
    header.PacketFlags |= I106CH10_PFLAGS_SEC_HEADER;
    memset(raw_header, 0, 36);
    memcpy(raw_header, &header, HEADER_SIZE);

    TEST_ASSERT_EQUAL(28073, SecondaryHeaderChecksum(raw_header));

    free(raw_header);
}


TEST(test_util, TestI106C10ErrorString){
    TEST_ASSERT_EQUAL_STRING("File not open", I106ErrorString(I106_NOT_OPEN));
    TEST_ASSERT_EQUAL_STRING("No index", I106ErrorString(I106_NO_INDEX));
    TEST_ASSERT_EQUAL_STRING("Unknown error", I106ErrorString(1000));
}


/* TEST(test_util, TestBufferSize){ */
/*     TEST_ASSERT_EQUAL(52, BufferSize(50, 0)); */
/*     TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_NONE)); */
/*     TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_8)); */
/*     TEST_ASSERT_EQUAL(52, BufferSize(50, I106CH10_PFLAGS_CHKSUM_16)); */
/*     TEST_ASSERT_EQUAL(56, BufferSize(50, I106CH10_PFLAGS_CHKSUM_32)); */
/* } */


TEST(test_util, TestAddFillerAndChecksum){
    void *data = malloc(100);
    I106C10Header header;
    TEST_ASSERT_EQUAL(I106_OK, AddFillerAndChecksum(&header, data));

    free(data);
}
