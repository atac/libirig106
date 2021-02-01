
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

    /* RUN_TEST_CASE(test_index, TestIndexPresent); */
    /* RUN_TEST_CASE(test_index, TestReadIndexes); */
    /* RUN_TEST_CASE(test_index, TestMakeIndex); */

    RUN_TEST_CASE(test_util, TestSetRelTime);
    RUN_TEST_CASE(test_util, TestRel2IrigTime);
    RUN_TEST_CASE(test_util, TestRelInt2IrigTime);
    RUN_TEST_CASE(test_util, TestIrig2RelTime);
    RUN_TEST_CASE(test_util, TestI106_Ch4Binary2IrigTime);
    RUN_TEST_CASE(test_util, TestIEEE15882IrigTime);
    RUN_TEST_CASE(test_util, TestFillInTimeStruct);
    RUN_TEST_CASE(test_util, TestLLInt2TimeArray);
    RUN_TEST_CASE(test_util, TestTimeArray2LLInt);
    RUN_TEST_CASE(test_util, TestI106_SyncTime);
    /* RUN_TEST_CASE(test_util, TestC10SetPosToIrigTime); */
    RUN_TEST_CASE(test_util, TestIrigTime2String);
    RUN_TEST_CASE(test_util, Testmkgmtime);
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


char tmats[1000];
int tmats_size;


TEST(test_util, TestIndexPresent){
    int handle = 1;
    int found_index;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, IndexPresent(handle, &found_index));
    TEST_ASSERT_EQUAL(0, found_index);
    I106C10Close(handle);
}


TEST(test_util, TestReadIndexes){
    int handle = 1;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, ReadIndexes(handle));
    I106C10Close(handle);
}


TEST(test_util, TestMakeIndex){
    int handle = 1;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/copy.c10", READ));

    TEST_ASSERT_EQUAL(I106_OK, MakeIndex(handle, 2));
    I106C10Close(handle);
}


TEST(test_util, TestSetRelTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_SetRelTime(1, &t, rtc), I106_OK);
}


TEST(test_util, TestRel2IrigTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_Rel2IrigTime(1, rtc, &t), I106_OK);
}


TEST(test_util, TestRelInt2IrigTime){
    I106Time t;
    TEST_ASSERT_EQUAL(I106_RelInt2IrigTime(1, 1, &t), I106_OK);
}


TEST(test_util, TestIrig2RelTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_Irig2RelTime(1, &t, rtc), I106_OK);
}


TEST(test_util, TestI106_Ch4Binary2IrigTime){
    I106Ch4_Binary_Time ch4_time;
    I106Time t;
    TEST_ASSERT_EQUAL(I106_Ch4Binary2IrigTime(&ch4_time, &t), I106_OK);
}


TEST(test_util, TestIEEE15882IrigTime){
    IEEE1588_Time ieee_time;
    I106Time t;
    TEST_ASSERT_EQUAL(I106_IEEE15882IrigTime(&ieee_time, &t), I106_OK);
}


TEST(test_util, TestFillInTimeStruct){
    I106C10Header header;
    IntraPacketTS ipts;
    TimeRef t;
    TEST_ASSERT_EQUAL(FillInTimeStruct(&header, &ipts, &t), I106_OK);
}


TEST(test_util, TestLLInt2TimeArray){
    uint8_t arr[] = {1, 2, 3};
    int64_t rtc;
    LLInt2TimeArray(&rtc, arr);
}


TEST(test_util, TestTimeArray2LLInt){
    uint8_t arr[] = {1, 2, 3};
    int64_t rtc;
    TimeArray2LLInt(arr, &rtc);
}


TEST(test_util, TestI106_SyncTime){
    int handle;
    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106_SyncTime(handle, 0, 10));
    I106C10Close(handle);
}


TEST(test_util, TestC10SetPosToIrigTime){
    int handle;
    I106Time t;

    TEST_ASSERT_EQUAL(I106_OK, I106C10Open(&handle, "tests/indexed.c10", READ));
    /* MakeInOrderIndex(handle); */
    TEST_ASSERT_EQUAL(I106_OK, I106_RelInt2IrigTime(handle, 38129384813, &t));
    TEST_ASSERT_EQUAL(I106_OK, I106C10SetPosToIrigTime(handle, &t));
    I106C10Close(handle);
}


TEST(test_util, TestIrigTime2String){
    I106Time t;
    t.Seconds = 1504191545;
    t.Fraction = 0;
    t.Format = I106_DATEFMT_DAY;
    TEST_ASSERT_EQUAL_STRING("243:14:59:05.000", IrigTime2String(&t));

    t.Format = I106_DATEFMT_DMY;
    TEST_ASSERT_EQUAL_STRING("2017/08/31 14:59:05.000", IrigTime2String(&t));
}


TEST(test_util, Testmkgmtime){
    struct tm t = {
        .tm_year = 117,
        .tm_mon = 7,
        .tm_mday = 31,
        .tm_hour = 14,
        .tm_min = 59,
        .tm_sec = 5,
        .tm_isdst = 1
    };

    TEST_ASSERT_EQUAL(1504191545, mkgmtime(&t));
}
