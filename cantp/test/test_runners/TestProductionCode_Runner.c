#include "unity.h"
#include "unity_fixture.h"

TEST_GROUP_RUNNER(TpSingleFrame)
{
  //RUN_TEST_CASE(TpSingleFrame, TestSFDataLength);
	//RUN_TEST_CASE(TpSingleFrame, TestSFReceptionFlow);

	RUN_TEST_CASE(TpSingleFrame, TestSFTransmission);

	//RUN_TEST_CASE(TpSingleFrame, TestFFTransmission);

}

TEST_GROUP_RUNNER(TpMultiFrame)
{
	//RUN_TEST_CASE(TpMultiFrame, TestMultiFrameReception); 
	RUN_TEST_CASE(TpMultiFrame, TestMultiFrameTransmission);
}