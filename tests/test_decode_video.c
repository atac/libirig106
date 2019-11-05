
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "i106_decode_video.h"
#include "unity.h"
#include "unity_fixture.h"


TEST_GROUP_RUNNER(test_decode_video){
    RUN_TEST_CASE(test_decode_video, Test_Decode_FirstVideoF0);
    RUN_TEST_CASE(test_decode_video, Test_Decode_NextVideoF0);
}


TEST_GROUP(test_decode_video);
TEST_SETUP(test_decode_video){}
TEST_TEAR_DOWN(test_decode_video){}


TEST(test_decode_video, Test_Decode_FirstVideoF0){
    I106C10Header header;
    void * buffer = malloc(32);
    VideoF0_Message msg;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_FirstVideoF0(&header, buffer, &msg));

    free(buffer);
}


TEST(test_decode_video, Test_Decode_NextVideoF0){
    I106C10Header header;
    VideoF0_Message msg;
    char * buffer = malloc(254);

    memset(buffer, 0, sizeof(VideoF0_CSDW));
    header.DataLength = 512;
    msg.Data = (uint8_t *)buffer + 4;
    msg.CSDW = (VideoF0_CSDW *)buffer;

    msg.CSDW->ET = 0;

    TEST_ASSERT_EQUAL(I106_OK, I106_Decode_NextVideoF0(&header, &msg));

    free(buffer);
}
