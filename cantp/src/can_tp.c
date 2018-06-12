/********************************************************************************************************************
|    File Name: XXX.c
|
|  Description: Implementation of the XXX
|--------------------------------------------------------------------------------------------------------------------
|               C O P Y R I G H T
|--------------------------------------------------------------------------------------------------------------------
| Copyright (c) 1996-2017 by XXX.       All rights reserved.
|
| This software is copyright protected and proprietary
| to XXX.XXX
| grants to you only those rights as set out in the
| license conditions. All other rights remain with
| XXX.
|
|--------------------------------------------------------------------------------------------------------------------
|               A U T H O R   I D E N T I T Y
|--------------------------------------------------------------------------------------------------------------------
| Initials     Name                      Company
| --------     ---------------------     -------------------------------------
| Rna          Ruediger Naas             Vector Informatik GmbH
|--------------------------------------------------------------------------------------------------------------------
|               R E V I S I O N   H I S T O R Y
|--------------------------------------------------------------------------------------------------------------------
| Date        Version   Author  Description
| ----------  --------  ------  -------------------------------------------------------------------------------------
| 2012-06-26  05.00.00  Rna     ESCAN00060132:Support ASR 4.0 Rev 3
|*******************************************************************************************************************/



/*******************************************************************************************************************/
/*  Include files                                                                                                  */
/*******************************************************************************************************************/

#include "can_tp.h"
#include "string.h"

/*******************************************************************************************************************/
/*  Version check                                                                                                  */
/*******************************************************************************************************************/


/*******************************************************************************************************************/
/*  Compatibility / Defaults                                                                                       */
/*******************************************************************************************************************/


/*******************************************************************************************************************/
/*  Version check (external modules)                                                                               */
/*******************************************************************************************************************/


/*******************************************************************************************************************/
/*  Defines / data types / structs / unions                                                                        */
/*******************************************************************************************************************/

/*----------Enum type-----------*/
typedef enum
{
	TP_CONN_IDLE = 0,
	TP_CONN_WAIT_SF_TX,
	TP_CONN_WAIT_SF_TX_CONFIRM,
	TP_CONN_WAIT_FF_TX,
	TP_CONN_WAIT_FF_TX_CONFIRM,
	TP_CONN_WAIT_FC_RX,
	TP_CONN_WAIT_CF_TX,
	TP_CONN_WAIT_CF_TX_CONFIRM,


}
tp_conn_state_type;



/*----------Macro const-----------*/

#define MODULE_DEBUG
#ifdef  MODULE_DEBUG
#define  STATIC 
#else 
#define STATIC  static

#endif

#define INVALID_CHANNEL_INDEX         0xFFu
#define INVALID_CHANNEL_ID            INVALID_CHANNEL_INDEX
#define INVALID_CONNECTION_INDEX      0xFFu

#define TIMER_IDLE_VALUE              (-1)

#define PCI_BYTE_MASK				  0xF0u
#define PCI_SF                        0x00u
#define PCI_FF						  0x10u
#define PCI_CF                        0x20u
#define PCI_FC                        0x30u

#define PADDING_BYTE				  0xCC


#define CLASSIC_SF_DL_MASK			  0x0Fu
#define FLEXRATE_SF_DL_MASK


#define MAX_CONNECTION_NUM             3

#define MAIN_FUNC_PERIOD               5u   //unit:ms

#define FS_CTS						   0u
#define FS_WAIT						   1u
#define FS_OVFLW                       2u



/*----------Macro function-----------*/
#define GET_CAN_DATA_LEN_BY_COMP_TBL(dlc)     (dlc_comparison_tbl[(dlc)&(0x0F)])  

#define RELEAS_CONNECTION(idx)                do{cantp_conn_cb[idx].conn_state = TP_CONN_IDLE;   \
												 cantp_conn_cb[idx].channel_idx = 0; \
												 cantp_conn_cb[idx].timer.A = INVALID_CHANNEL_INDEX;  \
												 cantp_conn_cb[idx].timer.B = INVALID_CHANNEL_INDEX;  \
                                                 cantp_conn_cb[idx].timer.C = INVALID_CHANNEL_INDEX;  \
												 cantp_conn_cb[idx].fc_buf_ref = NULL_PTR;}while(0)

/*----------Local data struct-----------*/


/*Type of module internal buffer for CAN frame.*/
typedef struct
{
	U8 tx_buffer[64];
	U8 tx_data_size;
}
inter_buf_type;


typedef  S16   ms_timer_type;


/*Network layer timing parameter type.it can be used for both sender and receiver.*/
typedef struct
{
	ms_timer_type    A;  //Time for transmission of the CAN frame.Start:LData.request, End: LData.confirm
	ms_timer_type    B; 
	ms_timer_type    C;
}
timing_parameter_type;



typedef struct
{
	U8   block_size;
	U8   st_min;
	U32  remaining_data_size;   //Remaining data size that has not been received or transmitted.
	U8   cf_count;               //cf_count records consecutive frame number whitch can be used to calc.SN value.
	U8   st_coordinator;
}flow_control_param_type;

/*Active tp connection task type.It is used in multi-frames RX and TX.*/
typedef struct
{
	U8                        channel_idx;
	tp_conn_state_type        conn_state; 
	U32                       sdu_data_size;  //Total data size to be transmitted or received of multi-frame.
	timing_parameter_type     timer;

	inter_buf_type*           fc_buf_ref;
	flow_control_param_type   fc_param;

	U8                        tx_copy_retry;
}
tp_conn_type;



/*----------Local data-----------*/
/**/


static  U8 dlc_comparison_tbl[16] = { 0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64 };
STATIC cantp_channel_cfg_type*  tp_channel_ref;

STATIC tp_conn_type cantp_conn_cb[MAX_CONNECTION_NUM];
STATIC inter_buf_type   segment_tx_buffer;


/*******************************************************************************************************************/
/*  Version check (generated data)                                                                                 */
/*******************************************************************************************************************/



/*******************************************************************************************************************/
/*  Compatibility defines                                                                                          */
/*******************************************************************************************************************/

/**********************************************************************************************************************
*  GLOBAL DATA
**********************************************************************************************************************/
extern cantp_channel_cfg_type  cantp_channel_cfgs[CANTP_STATIC_CHANNELS_NUM];
/*******************************************************************************************************************/
/*  external  prototypes                                                                                            */
/*******************************************************************************************************************/
void can_fmr_tx_callout(U32 can_id, U8* data_ptr, U8 data_size);
void TEST_PRINT_GLOBAL_TIME(U32 setp);
/*******************************************************************************************************************/
/*  local prototypes                                                                                               */
/*******************************************************************************************************************/
STATIC  U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag);
STATIC  U8 get_channel_idx_by_canid(U32  can_id);
STATIC  U8 tp_sf_rx_handle(U8 channel_idx, U8 dlc, U8* pci_ptr);
STATIC  U8 get_channel_idx_by_channel_id(U8 channel_id);
STATIC  U8 get_idle_conn(void);
STATIC  U8 get_active_conn_idx_by_channel_idx(U8 channel_idx);
/***********************************************************************************************************************
| NAME:  XXX_example()
**********************************************************************************************************************/
/*! \brief       Checks if queued PDUs have to be transmitted.
*  \details     Called out of CanIf_TxConfirmation(), CanIf_CancelTxConfirmation(), CanIf_CancelTxNotification().
*  \param[in]   hth: Index of the hardware transmit handle. Parameter must be smaller than CANIF_CFG_MAX_MAILBOXES.
*  \return      E_OK: Any Tx-PDU transmitted.
*               E_NOT_OK: No Tx-PDU transmitted.
*  \pre         Must be called with entered critical section in order to secure the Tx-queue.
*               Must only be called for "hth" to which any Tx-buffer is mapped.
*  \context     ANY
*  \reentrant   FALSE
*  \synchronous TRUE
*  \config      CANIF_TRANSMIT_BUFFER_PRIO_BY_CANID == STD_ON || CANIF_TRANSMIT_BUFFER_FIFO == STD_ON
**********************************************************************************************************************/
/***********************************************************************************************************************
| NAME:  cantp_init()
**********************************************************************************************************************/
/*! \brief     Indicates a CAN(FD) frame reception.
*  \details    This func. can be called either by ISR or CANTP mainfunction.
*  \param[in]
*  \return
*
*  \pre
*
*  \context
*  \reentrant
*  \synchronous
*  \config
**********************************************************************************************************************/

U8 cantp_init(void)
{
	U8 i, result = CANTP_E_OK;

	tp_channel_ref = cantp_channel_cfgs;

	for (i = 0; i < MAX_CONNECTION_NUM; i++)
	{
		RELEAS_CONNECTION(i);

	}
	return result;

}
/***********************************************************************************************************************
| NAME:  cantp_rx_indication()
**********************************************************************************************************************/
/*! \brief     Indicates a CAN(FD) frame reception.
*  \details    This func. can be called either by ISR or CANTP mainfunction.
*  \param[in]
*  \return
*
*  \pre
*
*  \context
*  \reentrant
*  \synchronous
*  \config
**********************************************************************************************************************/
void cantp_main_function(void)
{
	U8 i;
	U8 channel_idx;
	U32 remain_tx_buffer_size;
	U8 result, tmp_ext_addr_flag;
	U8 ff_sdu_data_size;
	U8 sdu_data_offset;

	/*-------------Polling Rx------------------------*/
	//TODO: if cycle polling rx mode is enable, add polling handle funciton here.
	/*Print global time info.Used only for debug.*/
	TEST_PRINT_GLOBAL_TIME(MAIN_FUNC_PERIOD);

	for (i = 0; i < MAX_CONNECTION_NUM; i++)
	{
		channel_idx = cantp_conn_cb[i].channel_idx;
		tmp_ext_addr_flag = tp_channel_ref[channel_idx].ext_addr_flag;

		switch (cantp_conn_cb[i].conn_state)
		{
		case TP_CONN_WAIT_SF_TX:
	
			result = tp_channel_ref[channel_idx].copy_tx_data_func(segment_tx_buffer.tx_buffer, cantp_conn_cb[i].sdu_data_size,NULL_PTR);
			
			if (result == CANTP_E_OK)
			{
				/*call lower layer driver to transmit can frame.*/
				cantp_conn_cb[i].conn_state = TP_CONN_WAIT_SF_TX_CONFIRM;
				/*Start timer As*/
				cantp_conn_cb[i].timer.A = 0;

				can_fmr_tx_callout(tp_channel_ref[channel_idx].can_id, segment_tx_buffer.tx_buffer, cantp_conn_cb[i].sdu_data_size);
			}
			else
			{
				/*Copy data filed ,confirm to upper layer*/
				tp_channel_ref[channel_idx].tx_confirmation_func(CANTP_R_ERROR);
			}
			break;
		case TP_CONN_WAIT_CF_TX_CONFIRM:
		case TP_CONN_WAIT_FF_TX_CONFIRM:
		case TP_CONN_WAIT_SF_TX_CONFIRM:

			cantp_conn_cb[i].timer.A += MAIN_FUNC_PERIOD;
			
			if (cantp_conn_cb[i].timer.A >= tp_channel_ref[channel_idx].A_TO)
			{
				RELEAS_CONNECTION(i);
				tp_channel_ref[channel_idx].tx_confirmation_func(CANTP_R_TIMEOUT_A);

			}
			else
			{
				//continue wait SF confirmation..
			}

			break;
		case TP_CONN_WAIT_FF_TX:

			if (tp_channel_ref[channel_idx].TX_DL == 8)
			{
				ff_sdu_data_size = 6 - tp_channel_ref[channel_idx].ext_addr_flag;

				/*TX_DL=8, FF pci format*/
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag] = ((cantp_conn_cb[i].sdu_data_size & 0x0F00) >> 8)| 0x10;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag + 1] = cantp_conn_cb[i].sdu_data_size & 0xFF;

				sdu_data_offset = 2 + tp_channel_ref[channel_idx].ext_addr_flag;
			}
			else
			{
				ff_sdu_data_size = tp_channel_ref[channel_idx].TX_DL - tp_channel_ref[channel_idx].ext_addr_flag-6;

				/*TX_DL>8, FF pci format*/
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag] = 0x10;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag+1] = 0x00;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag + 2] = (cantp_conn_cb[i].sdu_data_size & 0xFF000000)>>24;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag + 3] = (cantp_conn_cb[i].sdu_data_size & 0x00FF0000) >> 16;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag + 4] = (cantp_conn_cb[i].sdu_data_size & 0x0000FF00) >> 8;
				segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag + 5] = (cantp_conn_cb[i].sdu_data_size & 0x000000FF) ;
				sdu_data_offset = 6 + tp_channel_ref[channel_idx].ext_addr_flag;
			}

			cantp_conn_cb[i].fc_param.remaining_data_size = cantp_conn_cb[i].sdu_data_size - ff_sdu_data_size;
			cantp_conn_cb[i].fc_param.cf_count = 1;

			result = tp_channel_ref[channel_idx].copy_tx_data_func(segment_tx_buffer.tx_buffer+ sdu_data_offset, ff_sdu_data_size, NULL_PTR);

			if (result == CANTP_E_OK)
			{
				/**/
				cantp_conn_cb[i].conn_state = TP_CONN_WAIT_FF_TX_CONFIRM;
				/*Start timer As*/
				cantp_conn_cb[i].timer.B = 0;
				can_fmr_tx_callout(tp_channel_ref[channel_idx].can_id, segment_tx_buffer.tx_buffer, tp_channel_ref[channel_idx].TX_DL);
			}
			else
			{
				/*Copy data filed ,confirm to upper layer*/
				tp_channel_ref[channel_idx].tx_confirmation_func(CANTP_R_ERROR);
			}

			break; 
			
		case TP_CONN_WAIT_FC_RX:

				/*If conn has no flow control frame buffer reference, it means no FC frame has been recieved yet.*/
				/*So check Bs timeout*/
				cantp_conn_cb[i].timer.B += MAIN_FUNC_PERIOD;

				if (cantp_conn_cb[i].timer.B >= tp_channel_ref[channel_idx].BS_TO)
				{
					RELEAS_CONNECTION(i);
					tp_channel_ref[channel_idx].tx_confirmation_func(CANTP_R_TIMEOUT_BS);

				}
				else
				{
					//continue wait SF confirmation..
				}



			break;

		case TP_CONN_WAIT_CF_TX:
		{
			U8 tmp_data_size;

			if (!cantp_conn_cb[i].fc_param.st_coordinator)
			{
				/*restart stmin coordinator*/

				if (cantp_conn_cb[i].fc_param.st_min <= 127)
				{
					cantp_conn_cb[i].fc_param.st_coordinator = cantp_conn_cb[i].fc_param.st_min / MAIN_FUNC_PERIOD;
				}

				if (cantp_conn_cb[i].fc_param.remaining_data_size > (tp_channel_ref[channel_idx].TX_DL - 1))
				{
					/*Not last CF*/
					tmp_data_size = tp_channel_ref[channel_idx].TX_DL - 1;

				}
				else
				{
					tmp_data_size = cantp_conn_cb[i].fc_param.remaining_data_size;
					memset(segment_tx_buffer.tx_buffer, PADDING_BYTE, tp_channel_ref[channel_idx].TX_DL);

					/*Last CF*/
				}
				result = tp_channel_ref[channel_idx].copy_tx_data_func(segment_tx_buffer.tx_buffer + 1 + tmp_ext_addr_flag, tmp_data_size, NULL_PTR);

				if (result == CANTP_E_OK)
				{
					segment_tx_buffer.tx_buffer[tp_channel_ref[channel_idx].ext_addr_flag] = cantp_conn_cb[i].fc_param.cf_count % 16 | 0x20;

					cantp_conn_cb[i].fc_param.cf_count++;
					cantp_conn_cb[i].fc_param.remaining_data_size -= tmp_data_size;

					cantp_conn_cb[i].conn_state = TP_CONN_WAIT_CF_TX_CONFIRM;
					cantp_conn_cb[i].timer.A = 0;
					can_fmr_tx_callout(tp_channel_ref[channel_idx].can_id, segment_tx_buffer.tx_buffer, tp_channel_ref[channel_idx].TX_DL);


				}
				else
				{
					//TODO in futrue: copy tx data retry is not supported now.

				}
			}
			else
			{
				cantp_conn_cb[i].fc_param.st_coordinator--;
			}
		}
			break;

		default:
			break;
		}





	}





}





/***********************************************************************************************************************
| NAME:  cantp_rx_indication()
**********************************************************************************************************************/
/*! \brief     Indicates a CAN(FD) frame reception.
*  \details    This func. can be called either by ISR or CANTP mainfunction.
*  \param[in]
*  \return
*
*  \pre
*
*  \context
*  \reentrant
*  \synchronous
*  \config
**********************************************************************************************************************/

U8 cantp_transmit(U8 channel_id, U32  req_dl)
{
	U8 channel_idx;
	U8 result = CANTP_E_NOT_OK;
	U8 tx_dl;
	U8 ext_addr_flg;
	U8 conn_idx;
	if ((channel_idx = get_channel_idx_by_channel_id(channel_id)) != INVALID_CHANNEL_INDEX)
	{
		ext_addr_flg = (tp_channel_ref[channel_idx].ext_addr_flag) & 0x01;
		tx_dl = tp_channel_ref[channel_idx].TX_DL;

		conn_idx = get_idle_conn();
		if (conn_idx == INVALID_CONNECTION_INDEX)
		{
			result = CANTP_E_NOT_OK;
		}
		else
		{
			cantp_conn_cb[conn_idx].channel_idx = channel_idx;
			cantp_conn_cb[conn_idx].sdu_data_size = req_dl;

			if (((tx_dl == 8) && ((req_dl + ext_addr_flg + 1) <= tx_dl)) || \
				((req_dl + ext_addr_flg + 2) <= tx_dl))
			{
				/*SF data length judgement:
				1.if TX_DL =8, max. request data length is 7 or 6(with extended or mix address);
				2.if TX_DL>8, max.request data length is (TX_DL-2) or (TX_DL-3 with extended or mix address
				*/
				cantp_conn_cb[conn_idx].conn_state = TP_CONN_WAIT_SF_TX;
				
			}
			else
			{
				cantp_conn_cb[conn_idx].conn_state = TP_CONN_WAIT_FF_TX;
				/*multi-frames transmission:*/

			}
			result = CANTP_E_OK;
		}
		
		
	}
	else
	{
		/*Not supported channel id*/
		result = CANTP_E_NOT_OK;

	}

	return result;
}

/***********************************************************************************************************************
| NAME:  cantp_rx_indication()
**********************************************************************************************************************/
/*! \brief     Indicates a CAN(FD) frame reception.
*  \details    This func. can be called either by ISR or CANTP mainfunction.
*  \param[in]
*  \return
*
*  \pre
*
*  \context
*  \reentrant
*  \synchronous
*  \config
**********************************************************************************************************************/

void cantp_tx_confirmation(U32 canid, U8 result)
{	
	U8 channel_idx;
	U8 conn_idx;
	tp_conn_state_type* tmp_conn_state_ptr;
	channel_idx = get_channel_idx_by_channel_id(canid);

	conn_idx = get_active_conn_idx_by_channel_idx(channel_idx);

	tmp_conn_state_ptr = &cantp_conn_cb[conn_idx].conn_state;
	/**/
	if (*tmp_conn_state_ptr == TP_CONN_WAIT_SF_TX_CONFIRM)
	{
		/*SF transmit success,confirm to upper layer!*/
		cantp_channel_cfgs[channel_idx].tx_confirmation_func(CANTP_R_OK);
	}
	else if(*tmp_conn_state_ptr == TP_CONN_WAIT_FF_TX_CONFIRM)
	{
		*tmp_conn_state_ptr = TP_CONN_WAIT_FC_RX;

		// Start Bs timer waiting for flow control frame.
		cantp_conn_cb[conn_idx].timer.B = 0;

	}
	else if(*tmp_conn_state_ptr ==  TP_CONN_WAIT_CF_TX_CONFIRM)
	{
		if (cantp_conn_cb[conn_idx].fc_param.remaining_data_size == 0)
		{
			/*Last CF transmitted.*/
			RELEAS_CONNECTION(conn_idx);
			cantp_channel_cfgs[channel_idx].tx_confirmation_func(CANTP_R_OK);
		}
		else if (cantp_conn_cb[conn_idx].fc_param.block_size)
		{
			cantp_conn_cb[conn_idx].fc_param.block_size--;

			if (cantp_conn_cb[conn_idx].fc_param.block_size == 0)
			{
				/*Change to wait FC frame state.*/
				cantp_conn_cb[conn_idx].timer.B = 0;
				cantp_conn_cb[conn_idx].conn_state = TP_CONN_WAIT_FC_RX;
			}
			else
			{
				cantp_conn_cb[conn_idx].conn_state = TP_CONN_WAIT_CF_TX;
			}
		}
		else
		{
			cantp_conn_cb[conn_idx].conn_state = TP_CONN_WAIT_CF_TX;
		}


	}


}


/***********************************************************************************************************************
| NAME:  cantp_rx_indication()
**********************************************************************************************************************/
/*! \brief     Indicates a CAN(FD) frame reception. 
*  \details    This func. can be called either by ISR or CANTP mainfunction.   
*  \param[in]  
*  \return     
*             
*  \pre      
*              
*  \context    
*  \reentrant   
*  \synchronous 
*  \config      
**********************************************************************************************************************/
void  cantp_rx_indication(U32 canid, U8 dlc, U8* data_ptr)
{
	U8 rx_data_len;
	U8 pci_offset = 0; //In extended or mixed address, pci info will be located in CAN data field with offset = 1;
	U8 pci_type;
	U8 channel_idx = 0;
	U8 active_conn_idx;
	U8 flow_status;
#ifndef  CANTP_FLEX_DATA_RATE_SUPPORTED 

	if (dlc > 8)
	{
		dlc = 8;
	}
#else
	
#endif

	rx_data_len = GET_CAN_DATA_LEN_BY_COMP_TBL(dlc);

	/*---------------Check valid CANTP channel---------------------*/
	//TODO: pci_offset = 1 in extended or mixed 
	//TODO: get channel index channel_idx = ?
	channel_idx = get_channel_idx_by_canid(canid);
	/*---------------Verify received CAN frames--------------------*/
	pci_type = (*(data_ptr + pci_offset)) & PCI_BYTE_MASK;

	if (pci_type == PCI_SF)
	{
		tp_sf_rx_handle(channel_idx, dlc , data_ptr + pci_offset);
	}
	else if (pci_type ==  PCI_FF)
	{



	}
	else if (pci_type == PCI_CF)
	{

	}
	else if (pci_type == PCI_FC)
	{
		/*1.Check if active conn waiting for FC exsists.*/
		active_conn_idx = get_active_conn_idx_by_channel_idx(channel_idx);

		if (cantp_conn_cb[active_conn_idx].conn_state == TP_CONN_WAIT_FC_RX)
		{
			flow_status = (*(data_ptr + pci_offset)) & 0x0F;

			if (flow_status == FS_CTS)
			{
				cantp_conn_cb[active_conn_idx].fc_param.block_size = *(data_ptr + pci_offset + 1);
				cantp_conn_cb[active_conn_idx].fc_param.st_min = *(data_ptr + pci_offset + 2);               
				cantp_conn_cb[active_conn_idx].fc_param.st_coordinator = 0;
				cantp_conn_cb[active_conn_idx].conn_state = TP_CONN_WAIT_CF_TX;
			}
			else if (flow_status == FS_WAIT)
			{

			}
			else if (flow_status == FS_OVFLW)
			{


			}
		}
		else
		{
			//Unexpected frame received , ignore it!
		}

	}
	else 
	{
		/*Unknown PCI type frame received ,ignore it!*/

	}


}

/*SF_DL check*/
STATIC U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag)
{
	U8 result = CANTP_E_OK;
	U8 address_scheme_tbl[] = { 8,10,11,14,15,18,19,22,23,30,31,46,47,62 };
	
	if (sf_dl)
	{

		if (can_dl <= 8)
		{
#if defined(CANTP_CAN_FRAME_PADDING)
			if ((can_dl < 8) || ((sf_dl + 1 + ext_addr_flag) > can_dl))
			{
				/* 2 cases cause SF_DL error:
				1.in PADDING mode, CAN_DL do not equal 8;
				2. SF_DL is greater than (CAN_DL ¨C 1) of the received frame  when
				using normal addressing or greater than (CAN_DL ¨C 2) of the received
				frame for extended or mixed addressing.
				*/
				result = CANTP_E_NOT_OK;
			}
#elif defined(CANTP_CAN_FRAME_OPTIM)

			if (((can_dl + ext_addr_flag) <= 2) || ((sf_dl + 1 + ext_addr_flag) != can_dl))
			{
				/* 1 case causes SF_DL error: (ISO15765-2:2016 page 28,Table 12)
				*/
				result = CANTP_E_NOT_OK;
			}
#endif
			else
			{
				result = CANTP_E_OK;

			}
		}
		else
		{
			/*note: can_dl mean DLC code, not the actual data length of CANFD frame.*/
			if( ((sf_dl+ ext_addr_flag) >= address_scheme_tbl[(can_dl-9)*2]) && ((sf_dl + ext_addr_flag) <= address_scheme_tbl[(can_dl - 9) * 2+1]))
			{
				/*SF_DL must fall into the valid range shown in ISO15765-2:2016 page 28,Table 13*/
				result = CANTP_E_OK;
			}
			else
			{
				result = CANTP_E_NOT_OK;
			}
		}
	}
	else
	{
		result = CANTP_E_NOT_OK;
	}

	return result;
}



/*Execute single frame reception process.*/
STATIC U8 tp_sf_rx_handle(U8 channel_idx, U8 dlc, U8* pci_ptr)
{
	U8 sf_dl;
	U8 result = CANTP_E_OK, mem_req_result;
	U32 mem_req_dl,mem_remain_size;
	U8 ext_addr_flag;
	U8* n_sdu_ptr;
	/*1. Get SF_DL value*/
	if (dlc > 8)
	{
		if (((*pci_ptr) & 0x0F) == 0)
		{
			sf_dl = *(pci_ptr + 1); // In CAN FD format SF_DL is determined by byte#2 after PCI type byte.
			n_sdu_ptr = pci_ptr + 2;
		}
		else
		{
			return result; // Ignore reception of frame with error SF_DL format.
		}
	}
	else
	{
		sf_dl = (*pci_ptr) & 0x0F;
		n_sdu_ptr = pci_ptr + 1;
	}
	
	/*default regard all address as  normal address format.*/
	ext_addr_flag = 0;

	/*2. check if SF_DL is valid*/ 
	result = check_sf_dl(dlc, sf_dl, ext_addr_flag);

	if (result == CANTP_E_OK)
	{
		/*Request rx data memory from upper layer.*/
		mem_req_result = tp_channel_ref[channel_idx].req_rx_buf_func(sf_dl,&mem_req_dl);
		if ((mem_req_result == CANTP_E_OK) && (mem_req_dl >= sf_dl))
		{
			/*Enough memory size can be provided by upper layer.*/
			tp_channel_ref[channel_idx].copy_rx_data_func(n_sdu_ptr, sf_dl, &mem_remain_size);

			tp_channel_ref[channel_idx].rx_indication_func(CANTP_R_OK);

		}
		else
		{
			result = CANTP_E_NOT_OK;
		}

	}
	else
	{
		//Ignore SF reception.
	}

	return result;

}


STATIC U8 get_channel_idx_by_canid(U32  canid)
{
	//TODO
	return 0;
}


STATIC U8 get_channel_idx_by_channel_id(U8 channel_id)
{
	return 0;
}

STATIC U8 get_idle_conn(void)
{
	U8 i,conn_idx = INVALID_CONNECTION_INDEX;
	for (i = 0; i<MAX_CONNECTION_NUM;i++)
	{

		if (cantp_conn_cb[i].conn_state == TP_CONN_IDLE)
		{
			conn_idx = i;
			break;
		}
	}

	return conn_idx;

}

STATIC U8 get_active_conn_idx_by_channel_idx(U8 channel_idx)
{

	//TODO

	return 0;

}



void TEST_PRINT_GLOBAL_TIME(U32 step)
{
	static	U32    TEST_GLOBAL_MS_TMIER = 0;

	U8 hour, min, sec;
	U32 msec;
	
	msec = TEST_GLOBAL_MS_TMIER % 1000;
	sec = (TEST_GLOBAL_MS_TMIER / 1000) % 60;
	min = (TEST_GLOBAL_MS_TMIER / 1000 / 60) % 60;
	hour = (TEST_GLOBAL_MS_TMIER / 1000 / 60 / 60) % 24;

	TEST_GLOBAL_MS_TMIER += step;

	if ((TEST_GLOBAL_MS_TMIER + step) > 1000 * 60 * 60 * 24)
	{
		TEST_GLOBAL_MS_TMIER = TEST_GLOBAL_MS_TMIER + step - 1000 * 60 * 60 * 24;
	}

	printf("Global time:%d h: %d m: %d s: %d ms   \n", hour, min, sec, msec);

}