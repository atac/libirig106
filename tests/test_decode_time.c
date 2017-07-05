
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_time.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_decode_time){
    RUN_TEST_CASE(test_decode_time, TestDecode_TimeF1);
    RUN_TEST_CASE(test_decode_time, TestDecode_TimeF1_Buffer);
    RUN_TEST_CASE(test_decode_time, TestEncode_TimeF1);
}


TEST_GROUP(test_decode_time);
TEST_SETUP(test_decode_time){}
TEST_TEAR_DOWN(test_decode_time){}


TEST(test_decode_time, TestDecode_TimeF1){
    I106C10Header header;
    void * buffer = malloc(32);
    I106Time time;
    TimeF1_CSDW csdw;

    memcpy(buffer, &csdw, sizeof(TimeF1_CSDW));

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_TimeF1(&header, buffer, &time));

    free(buffer);
}


TEST(test_decode_time, TestDecode_TimeF1_Buffer){
    void * buffer = malloc(32);
    I106Time out_time;
    Time_MessageDayFormat time;

    // Target date/time: 5/4/1991 4:45:32.23 PM
    time.Hmn = 3;
    time.Tmn = 2;
    time.TSn = 3;
    time.Sn = 2;
    time.TMn = 4;
    time.Mn = 5;
    time.THn = 1;
    time.Hn = 6;
    time.HDn = 1;
    time.TDn = 2;
    time.Dn = 4;
    memcpy(buffer, &time, sizeof(Time_MessageDayFormat));

    I106_Decode_TimeF1_Buffer(0, 0, buffer, &out_time);

    TEST_ASSERT_EQUAL(42223532, out_time.Seconds);
    TEST_ASSERT_EQUAL(3200000, out_time.Fraction);

    free(buffer);
}


TEST(test_decode_time, TestEncode_TimeF1){
    I106C10Header header;
    I106Time input_time;
    void * buffer = malloc(32);
    MessageTimeF1 * msg;
    Time_MessageDayFormat * time;

    input_time.Seconds = 42223532;
    input_time.Fraction = 3200000;

    TEST_ASSERT_EQUAL(I106_OK, I106_Encode_TimeF1(&header, 2, 1, 0, &input_time,
                buffer));
    msg = (MessageTimeF1 *)buffer;

    TEST_ASSERT_EQUAL(2, msg->CSDW.TimeSource);
    TEST_ASSERT_EQUAL(1, msg->CSDW.TimeFormat);
    TEST_ASSERT_EQUAL(0, msg->CSDW.DateFormat);

    time = &(msg->Message.DayFormat);
    int results[] = {time->Hmn, time->Tmn, time->TSn, time->Sn, time->TMn,
        time->Mn, time->THn, time->Hn, time->HDn, time->TDn, time->Dn};
    int expected[] = {3, 2, 3, 2, 4, 5, 1, 6, 1, 2, 4};
    TEST_ASSERT_EQUAL_INT_ARRAY(results, expected, 11);

    free(buffer);
}
