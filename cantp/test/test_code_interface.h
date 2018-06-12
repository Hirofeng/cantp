#ifndef _TEST_CODE_INTERFACE_H_
#define _TEST_CODE_INTERFACE_H_

#include "can_tp.h"


extern U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag);


extern void  can_driver_sf_rx_isr_1(void);
extern void  can_driver_sf_rx_isr_2(void);
extern void can_fmr_rx_callout(U32 can_id, U8* data_ptr, U8 dlc);

extern cantp_channel_cfg_type  cantp_channel_cfgs[CANTP_STATIC_CHANNELS_NUM];
extern void  can_driver_init_disable_tx_confirm(void);
extern void  can_driver_init_normal(void);
extern void can_fc_rx_1(void);
extern void  can_driver_ff_rx_isr_1(void);

void  can_driver_cf_rx_isr_1(U8* data_ptr, U8 data_size);
#endif