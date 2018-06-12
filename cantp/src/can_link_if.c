
#include "can_tp.h"

static U8 dlc_comparison_tbl[16] = { 0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64 };

void  can_driver_sf_rx_isr_1(void)
{
	U32  can_id = 0x720;
	U8 DLC,i;
	U8 can_rx_data[] = {0x07,1,2,3,4,5,6,7};

	DLC = sizeof(can_rx_data) / sizeof(U8);

	printf("CAN frame received: CAN ID£º0x%4X, DLC:%d ,Data: ",can_id, DLC);

	for (i = 0; i < dlc_comparison_tbl[DLC]; i++)
	{
		printf(" %2X ", can_rx_data[i]);
	}

	cantp_rx_indication(can_id, DLC, can_rx_data);

}


void  can_driver_ff_rx_isr_1(void)
{
	U32  can_id = 0x720;
	U8 DLC, i;
	U8 can_rx_data[] = { 0x10,50,1,2,3,4,5,6 };

	DLC = sizeof(can_rx_data) / sizeof(U8);

	printf("First frame received: CAN ID£º0x%4X, DLC:%d ,Data: ", can_id, DLC);

	for (i = 0; i < dlc_comparison_tbl[DLC]; i++)
	{
		printf(" %2X ", can_rx_data[i]);
	}
	printf("\n");

	cantp_rx_indication(can_id, DLC, can_rx_data);
}

void  can_driver_cf_rx_isr_1(U8* data_ptr, U8 data_size)
{
	U32  can_id = 0x720;
	U8 DLC, i;
	

	printf("Consecutive frame received: CAN ID£º0x%4X, DLC:%d ,Data: ", can_id, data_size);

	for (i = 0; i < data_size; i++)
	{
		printf(" %2X ", data_ptr[i]);
	}
	printf("\n");

	cantp_rx_indication(can_id, data_size, data_ptr);
}













void  can_driver_sf_rx_isr_2(void)
{
	U32  can_id = 0x720;
	U8 DLC, i;
	U8 can_rx_data[] = { 0x00,0x0C,1,2,3,4,5,6,7,8,9,10,11,12,13,14 };

	DLC = 10;

	printf("CAN frame received: CAN ID£º0x%4X, DLC:%d ,Data: ", can_id, DLC);

	for (i = 0; i < dlc_comparison_tbl[DLC]; i++)
	{
		printf(" %2X ", can_rx_data[i]);
	}
	printf("\n");


	cantp_rx_indication(can_id, DLC, can_rx_data);

}


void can_fc_rx_1(void)
{
	U32  can_id = 0x720;
	U8 DLC, i;
	/*FS,BS,STmin*/
	U8 can_rx_data[] = { 0x30,0x05,2,0,0,0,0,0 };

	DLC = 8;

	printf("Flow control frame received: CAN ID£º0x%4X, DLC:%d ,Data: ", can_id, DLC);

	for (i = 0; i < dlc_comparison_tbl[DLC]; i++)
	{
		printf(" %2X ", can_rx_data[i]);
	}
	printf("\n");

	cantp_rx_indication(can_id, DLC, can_rx_data);

}