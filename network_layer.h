/***************************************************************************//**
    \file          network_layer.h
    \author        
    \mail          
    \version       0
    \date          2016-09-24
    \description   uds network code, base on ISO 15765
*******************************************************************************/
#ifndef _NETWORK_LAYER_H_
#define _NETWORK_LAYER_H_

/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "uds_type.h"

typedef enum _N_TATYPE_T_
{
    N_TATYPE_NONE = 0,
    N_TATYPE_PHYSICAL,
    N_TATYPE_FUNCTIONAL
}n_tatype_t;

typedef enum _N_RESULT_
{
    N_OK = 0,
    N_TIMEOUT_A,
    N_TIMEOUT_Bs,
    N_TIMEOUT_Cr,
    N_WRONG_SN,
    N_INVALID_FS,
    N_UNEXP_PDU,
    N_WFT_OVRN,
    N_BUFFER_OVFLW,
    N_ERROR
}n_result_t;

typedef void
(*ffindication_func) (uint16_t msg_dlc);
typedef void
(*indication_func) (uint8_t msg_buf[], uint16_t msg_dlc, n_result_t n_result);
typedef void
(*confirm_func) (n_result_t n_result);

typedef struct _NETWORK_USER_DATA_T_
{
    ffindication_func ffindication;
    indication_func indication;
    confirm_func    confirm;
}nt_usdata_t;

/*******************************************************************************
    external Varaibles
*******************************************************************************/
extern uint8_t g_tatype;

/*******************************************************************************
    Function  Definition
*******************************************************************************/


/**
 * network_main - network main task, should be schedule every one ms
 *
 * @void
 *
 * returns:
 *     void
 */
extern void
network_main(void);

/**
 * network_recv_frame - recieved uds network can frame
 *
 * @func_addr : 0 - physical addr, 1 - functional addr
 * @frame_buf : uds can frame data buffer
 * @frame_dlc : uds can frame length
 *
 * returns:
 *     void
 */
extern void
network_recv_frame(uint8_t func_addr, uint8_t frame_buf[], uint8_t frame_dlc);

/**
 * network_send_udsmsg - send a uds msg by can
 *
 * @msg_buf : uds msg data buffer
 * @msg_dlc : uds msg length
 *
 * returns:
 *     void
 */
extern void
network_send_udsmsg(uint8_t msg_buf[], uint16_t msg_dlc);


/**
 * network_reg_usdata - reg usdata Function
 *
 * @usdata : uds msg data Function struct
 *
 * returns:
 *     0 - ok, other - err
 */
extern int
network_reg_usdata(nt_usdata_t usdata);
#endif
/****************EOF****************/
