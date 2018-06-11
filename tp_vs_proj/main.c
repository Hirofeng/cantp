#include <stdio.h>
#include "can_tp.h"

extern U8 check_sf_dl(U8 can_dl, U8 sf_dl, BOOL ext_addr_flag);

int main(void)
{
	U8 dlc = 10, sf_dl = 13,ext_addr = 0;

	printf("DLC: %d, SF_DL:%d£¬valid: %d \n", dlc, sf_dl, check_sf_dl(dlc, sf_dl, ext_addr));

	dlc = 10, sf_dl = 10, ext_addr = 1;
	printf("DLC: %d, SF_DL:%d£¬valid: %d \n", dlc, sf_dl, check_sf_dl(dlc, sf_dl, ext_addr));
	dlc = 8, sf_dl = 2; ext_addr = 0;
	printf("DLC: %d, SF_DL:%d£¬valid: %d \n", dlc, sf_dl, check_sf_dl(dlc, sf_dl, ext_addr));

	dlc = 15, sf_dl = 48; ext_addr = 0;
	printf("DLC: %d, SF_DL:%d£¬valid: %d \n", dlc, sf_dl, check_sf_dl(dlc, sf_dl, ext_addr));
	getchar();
}