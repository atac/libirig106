#include "unity_fixture.h"


static void RunAllTests(void){
    RUN_TEST_GROUP(test_1553);
    RUN_TEST_GROUP(test_analog);
}

int main(int argc, const char * argv[]){
    return UnityMain(argc, argv, RunAllTests);
}
