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
	TP_CONN_RX,
	TP_CONN_TX,

}
tp_conn_state_type;


/*----------Macro function-----------*/
#define GET_CAN_DATA_LEN_BY_COMP_TBL(dlc)     (dlc_comparison_tbl[(dlc)&(0x0F)])  


/*----------Macro const-----------*/


#define INVALID_CHANNEL_INDEX         0xFFu
#define INVALID_CHANNEL_ID            INVALID_CHANNEL_INDEX

#define PCI_BYTE_MASK				  0xF0u
#define PCI_SF                        0x00u
#define PCI_FF						  0x10u
#define PCI_CF                        0x20u
#define PCI_FC                        0x30u


#define CLASSIC_SF_DL_MASK			  0x0Fu
#define FLEXRATE_SF_DL_MASK

/*----------Local data struct-----------*/

typedef U8 tp_timer_type;

/*Active tp connection task type.It is used in multi-frames RX and TX.*/
typedef struct
{
	U8                    channel_idx;
	tp_conn_state_type    conn_state;
	tp_timer_type         as_cnt;   
}
tp_conn_type;



/*----------Local data-----------*/
/**/


static U8 dlc_comparison_tbl[16] = { 0,1,2,3,4,5,6,7,8,12,16,20,24,32,48,64 };

static cantp_channel_cfg_type*  tp_channel_ref;

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
/*  local prototypes                                                                                               */
/*******************************************************************************************************************/

static U8 get_channel_idx_by_can_id(U32  can_id);
static void tp_sf_rx_handle(U8 channel_idx, U8 dlc, U8* pci_ptr);
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

	}
	else 
	{
		/*Unknown PCI type frame received ,ignore it!*/

	}


}

/*SF_DL check*/
U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag)
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
static U8 tp_sf_rx_handle(U8 channel_idx, U8 dlc, U8* pci_ptr)
{
	U8 sf_dl;
	U8 result;
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
			return; // Ignore reception of frame with error SF_DL format.
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



	}
	else
	{
		//Ignore SF reception.
	}

	return result;

}


static U8 get_channel_idx_by_canid(U32  canid)
{
	//TODO
	
}