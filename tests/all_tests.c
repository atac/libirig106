#include "unity_fixture.h"


static void RunAllTests(void){
    RUN_TEST_GROUP(test_1553);
    RUN_TEST_GROUP(test_analog);
    RUN_TEST_GROUP(test_arinc429);
    RUN_TEST_GROUP(test_can);
    RUN_TEST_GROUP(test_discrete);
    RUN_TEST_GROUP(test_ethernet);
    RUN_TEST_GROUP(test_decode_index);
    RUN_TEST_GROUP(test_decode_pcm);
    RUN_TEST_GROUP(test_decode_time);
    RUN_TEST_GROUP(test_decode_tmats);
    RUN_TEST_GROUP(test_decode_uart);
    RUN_TEST_GROUP(test_decode_video);
    RUN_TEST_GROUP(test_index);
    RUN_TEST_GROUP(test_time);
    RUN_TEST_GROUP(test_i106);
}

int main(int argc, const char * argv[]){
    return UnityMain(argc, argv, RunAllTests);
}
