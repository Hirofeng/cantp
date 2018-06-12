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

#ifdef WIN32_DEBUG
#include <stdio.h>
#include <string.h>

#endif

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



/*******************************************************************************************************************/
/*  Version check (generated data)                                                                                 */
/*******************************************************************************************************************/



/*******************************************************************************************************************/
/*  Compatibility defines                                                                                          */
/*******************************************************************************************************************/

/**********************************************************************************************************************
*  GLOBAL DATA
**********************************************************************************************************************/

/*******************************************************************************************************************/
/*  local prototypes                                                                                               */
/*******************************************************************************************************************/


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
| NAME:  XXX_example()
**********************************************************************************************************************/
/*! \brief
*  \details
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

#define TMP_DCM_RX_BUF_SIZE   5000
#define TMP_DCM_TX_BUF_SIZE   6000
U8  dcm_rx_buf[TMP_DCM_RX_BUF_SIZE] = {0};
U32 dcm_rx_buf_idx = 0;

U8 dcm_tx_buf[TMP_DCM_TX_BUF_SIZE] = { 0 };
U32 dcm_tx_buf_idx = 0;
U32 dcm_req_data_size = 6;



U8 dcm_tmp_request_rx_buffer(U32 total_size, U32* buffer_size_ptr)
{
	U8 result = CANTP_E_OK;

	if (total_size <= TMP_DCM_RX_BUF_SIZE)
	{
		dcm_rx_buf_idx = 0;
		*buffer_size_ptr = TMP_DCM_RX_BUF_SIZE - dcm_rx_buf_idx;
		
	}
	else
	{
		result = CANTP_E_NOT_OK;
	}


	return result;



}


U8 dcm_tmp_copy_rx_data(const U8 * sdu_ptr, U32 sdu_len, U32* remain_buf_size_ptr)
{
	U32 i;
	for (i = 0; i < sdu_len; i++)
	{
		dcm_rx_buf[i + dcm_rx_buf_idx] = *(sdu_ptr + i);
		
	}
	dcm_rx_buf_idx += sdu_len;
	if (dcm_rx_buf_idx < TMP_DCM_RX_BUF_SIZE)
	{
		*remain_buf_size_ptr = TMP_DCM_RX_BUF_SIZE - dcm_rx_buf_idx;
	}
	else
	{
		*remain_buf_size_ptr = 0;
	}

	return CANTP_E_OK;
}

void print_dcm_tmp_rx_data(void)
{
	U32 i;

	if (dcm_rx_buf_idx == 0)
	{
		printf("No data received in DCM rx buffer!\n");
	}
	else
	{
		printf("DCM received buffer data: ");
		for (i = 0; i < dcm_rx_buf_idx; i++)
		{
			printf(" %x ", dcm_rx_buf[i]);
		}

	}
}


void dcm_tmp_rx_indication(U8 rx_result)
{
	printf("CAN TP rx indication generated: ");
	switch (rx_result)
	{
	    case CANTP_R_OK:
		printf("CANTP_R_OK:\n");
		print_dcm_tmp_rx_data();
		break;
	}

}

U8  dcm_tmp_coyp_tx_data(U8* tx_buf_ptr, U32 tx_data_size, U32* remain_tx_buf_size_ptr)
{
	U8 result;

	if ((dcm_req_data_size - dcm_tx_buf_idx) >= tx_data_size)
	{

		memcpy((U8*)tx_buf_ptr, (U8*)(dcm_tx_buf + dcm_tx_buf_idx), tx_data_size);

		dcm_tx_buf_idx += tx_data_size;
		if (remain_tx_buf_size_ptr != NULL_PTR)
		{
			*remain_tx_buf_size_ptr = dcm_req_data_size - dcm_tx_buf_idx;
		}
		result = CANTP_E_OK;
	}
	else
	{

		result = CANTP_E_NOT_OK;
	}


	return result;

}



void dcm_tmp_tx_confirmation(U8 result)
{

#ifdef WIN32_DEBUG
	switch (result)
	{
		case CANTP_R_OK:

		printf("DCM transmittion result: CANTP_R_OK ! \n");
		break;

		case CANTP_R_ERROR:
		printf("DCM transmittion result: CANTP_R_ERROR !\n");
			break;

		case CANTP_R_TIMEOUT_A:
			printf("DCM transmittion result: CANTP_R_TIMEOUT_A !\n");

			break;

		case CANTP_R_TIMEOUT_BS:
			printf("DCM transmittion result: CANTP_R_TIMEOUT_BS !\n");
			break;
	}
#endif

}

void dcm_tmp_transmit_response(U32 data_size)
{
	U32 i;

	dcm_tx_buf_idx = 0;
	dcm_req_data_size = data_size;

	for (i = 0; i < dcm_req_data_size;i++)
	{
		dcm_tx_buf[i] = i+1;
	}

	cantp_transmit(0, dcm_req_data_size);

#ifdef WIN32_DEBUG
	printf("DCM request to transmit reponse data, data size: %d, data: ", dcm_req_data_size);

	for (i = 0; i < dcm_req_data_size; i++)
	{
		printf(" %2X ", dcm_tx_buf[i]);
	}
	printf("\n");

#endif
	
}