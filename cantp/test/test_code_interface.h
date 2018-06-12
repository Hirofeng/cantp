#ifndef _TEST_CODE_INTERFACE_H_
#define _TEST_CODE_INTERFACE_H_

#include "can_tp.h"


extern U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag);


extern void  can_driver_sf_rx_isr_1(void);
extern void  can_driver_sf_rx_isr_2(void);


#endif