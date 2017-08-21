
#include <errno.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_index.h"
#include "unity.h"
#include "unity_fixture.h"
#include <assert.h>


TEST_GROUP_RUNNER(test_time){
    RUN_TEST_CASE(test_time, TestSetRelTime);
    RUN_TEST_CASE(test_time, TestRel2IrigTime);
    RUN_TEST_CASE(test_time, TestRelInt2IrigTime);
    RUN_TEST_CASE(test_time, TestIrig2RelTime);
    RUN_TEST_CASE(test_time, TestI106_Ch4Binary2IrigTime);
    RUN_TEST_CASE(test_time, TestIEEE15882IrigTime);
    RUN_TEST_CASE(test_time, TestFillInTimeStruct);
    RUN_TEST_CASE(test_time, TestLLInt2TimeArray);
    RUN_TEST_CASE(test_time, TestTimeArray2LLInt);
    /* RUN_TEST_CASE(test_time, TestI106_SyncTime); */
    /* RUN_TEST_CASE(test_time, TestCh10SetPosToIrigTime); */
    /* RUN_TEST_CASE(test_time, TestIrigTime2String); */
    /* RUN_TEST_CASE(test_time, Testmkgmtime); */
}


TEST_GROUP(test_time);
TEST_SETUP(test_time){}
TEST_TEAR_DOWN(test_time){}


TEST(test_time, TestSetRelTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_SetRelTime(1, &t, rtc), I106_OK);
}


TEST(test_time, TestRel2IrigTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_Rel2IrigTime(1, rtc, &t), I106_OK);
}


TEST(test_time, TestRelInt2IrigTime){
    I106Time t;
    TEST_ASSERT_EQUAL(I106_RelInt2IrigTime(1, 1, &t), I106_OK);
}


TEST(test_time, TestIrig2RelTime){
    I106Time t;
    uint8_t rtc[] = {1, 2, 3};
    TEST_ASSERT_EQUAL(I106_Irig2RelTime(1, &t, rtc), I106_OK);
}


TEST(test_time, TestI106_Ch4Binary2IrigTime){
    I106Ch4_Binary_Time ch4_time;
    I106Time t;
    TEST_ASSERT_EQUAL(I106_Ch4Binary2IrigTime(&ch4_time, &t), I106_OK);
}


TEST(test_time, TestIEEE15882IrigTime){
    IEEE1588_Time ieee_time;
    I106Time t;
    TEST_ASSERT_EQUAL(I106_IEEE15882IrigTime(&ieee_time, &t), I106_OK);
}


TEST(test_time, TestFillInTimeStruct){
    I106C10Header header;
    IntraPacketTS ipts;
    TimeRef t;
    TEST_ASSERT_EQUAL(FillInTimeStruct(&header, &ipts, &t), I106_OK);
}


TEST(test_time, TestLLInt2TimeArray){
    uint8_t arr[] = {1, 2, 3};
    int64_t rtc;
    LLInt2TimeArray(&rtc, &arr[0]);
}


TEST(test_time, TestTimeArray2LLInt){
    uint8_t arr[] = {1, 2, 3};
    int64_t rtc;
    TimeArray2LLInt(arr, &rtc);
}


TEST(test_time, TestI106_SyncTime){
    int handle;
    assert(I106_OK == I106C10Open(&handle, "tests/indexed.c10", READ));
    TEST_ASSERT_EQUAL(I106_OK, I106_SyncTime(handle, 1, 10));
}
