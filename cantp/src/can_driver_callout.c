#include "can_tp.h"

#include "std_types.h"


U8 CONTROL_CODE;

#define NORMAL_MODE                     0u
#define DISABLE_TX_CONFIRMATION         1u


void  can_driver_init_normal(void)
{
	CONTROL_CODE = NORMAL_MODE;
}


void  can_driver_init_disable_tx_confirm(void)
{
	CONTROL_CODE = DISABLE_TX_CONFIRMATION;
}




/*从CAN Driver寄存器内读取缓存的数据帧*/

void can_fmr_rx_callout(U32 can_id, U8* data_ptr,U8 dlc)
{
  
}



void can_fmr_tx_callout(U32 can_id, U8* data_ptr,U8 data_size)
{
	U8 i;


	printf("can driver transmit frame: ");
	switch (*data_ptr & 0xF0)
	{
	case 0x00:
		printf(" Single frame ");
		break;
	case 0x10:
		printf(" First frame ");
		break;
	case 0x20:
		printf(" Consecutive frame ");

		break;
	case 0x30:
		printf(" Flow control ");
		break;
	}

#ifdef WIN32_DEBUG

	printf(" CAN ID: 0x%4X,DLC: %d, Data: ", can_id, data_size);
	
	for (i = 0; i < data_size; i++)
	{
		printf(" %2X ", data_ptr[i]);
	}
	printf("\n");
#endif
	if (CONTROL_CODE == NORMAL_MODE)
	{
		cantp_tx_confirmation(0x720, CANTP_R_OK);
	}
}