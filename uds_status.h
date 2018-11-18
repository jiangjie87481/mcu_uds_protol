/***************************************************************************//**
    \file          uds-status.h
    \author        
    \mail         
    \version       0
    \date          2016-10-10
    \description   uds status code, include session and Security access
*******************************************************************************/
#ifndef	__UDS_STATUS_H_
#define	__UDS_STATUS_H_
/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "uds_type.h"

/*******************************************************************************
    Type Definition
*******************************************************************************/
/* SECURITYACCESS */
#define UNLOCKKEY					0x00000000
#define UNLOCKSEED					0x00000000
#define UNDEFINESEED				0xFFFFFFFF
#define SEEDMASK					0x80000000
#define SHIFTBIT					1
#define ALGORITHMASK				0x42303131


/*******************************************************************************
    Function  Definition
*******************************************************************************/

/**
 * uds_security_access - check the key of Security Access
 *
 * @key_buf:  recieved key buff
 * @seed   :  original seed
 *
 * returns:
 *     0 - success， -1 - fail
 */
int
uds_security_access (uint8_t key_buf[], uint8_t seed_buf[]);

#endif
/****************EOF****************/
