
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_uart.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_decode_uart){
    RUN_TEST_CASE(test_decode_uart, Test_Decode_FirstUARTF0);
    RUN_TEST_CASE(test_decode_uart, Test_Decode_NextUARTF0);
}


TEST_GROUP(test_decode_uart);
TEST_SETUP(test_decode_uart){}
TEST_TEAR_DOWN(test_decode_uart){}


TEST(test_decode_uart, Test_Decode_FirstUARTF0){
    I106C10Header header;
    void * buffer = malloc(32);
    UARTF0_Message msg;

    header.DataLength = 32;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstUARTF0(&header, buffer, &msg));

    free(buffer);
}


TEST(test_decode_uart, Test_Decode_NextUARTF0){
    UARTF0_Message msg;
    I106C10Header header;
    UARTF0_CSDW csdw;

    csdw.IPH = 0;
    header.DataLength = 32;
    msg.CSDW = &csdw;
    msg.BytesRead = 4;
    msg.Header = &header;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_NextUARTF0(&msg));
}
