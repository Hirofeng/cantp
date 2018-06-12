#include "unity_fixture.h"

static void RunAllTests(void)
{
  RUN_TEST_GROUP(TpSingleFrame);
  //RUN_TEST_GROUP(ProductionCode2);
}

int main(int argc, const char * argv[])
{
	UnityMain(argc, argv, RunAllTests);
	getchar();
	return 0;

}
