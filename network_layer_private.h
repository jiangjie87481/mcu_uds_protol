/***************************************************************************//**
    \file          network_layer_private.h
    \author        
    \mail          
    \version       0
    \date          2016-09-24
    \description   uds network code, base on ISO 15765
*******************************************************************************/
#ifndef _NETWORK_LAYER_PRIVATE_H_
#define _NETWORK_LAYER_PRIVATE_H_

/*******************************************************************************
    Include Files
*******************************************************************************/

/*******************************************************************************
    Type Definition
*******************************************************************************/


#define  UDS_CAN_ID_STD

#ifdef UDS_CAN_ID_STD
#define UDS_VALID_FRAME_LEN     8
#define UDS_SF_DL_MAX           7
#define UDS_FF_DL_MIN           8
#define UDS_FF_DL_MAX           (0xFFF)
#else
#define UDS_VALID_FRAME_LEN     7
#define UDS_SF_DL_MAX           6
#define UDS_FF_DL_MIN           7
#define UDS_FF_DL_MAX           (0xFFF)
#endif

#define UDS_CF_DL_COM           7


typedef enum __NT_TIMER_T__
{
	TIMER_N_CR = 0,
	TIMER_N_BS,
    TIMER_STmin,
	TIMER_CNT
}nt_timer_t;

typedef enum __NETWORK_LAYER_STATUS_
{
    NWL_IDLE = 0,  /* Neither a segmented transmission nor segmented reception is in progress */
    NWL_XMIT,      /* transmission */
    NWL_RECV,      /* reception  */
    NWL_CNT        /* NONE */
}network_layer_st;


typedef enum __NETWORK_PCI_TYPE_
{
    PCI_SF = 0,  /* single frame */
    PCI_FF,      /* first frame */
    PCI_CF,      /* consecutive frame */
    PCI_FC       /* flow control frame */
}network_pci_type_t;


typedef enum __NETWORK_FLOW_STATUS__
{
	FS_CTS = 0, /* ContinueToSend */
	FS_WT,      /* Wait */
	FS_OVFLW,   /* OverFlow */
	FS_RESERVED
}network_flow_status_t;



#define N_PCItype_SF                (0x00)
#define N_PCItype_FF                (0x00)
#define N_PCItype_SF                (0x00)
#define N_PCItype_SF                (0x00)

#define NT_SET_PCI_TYPE_SF(low)     (0x00 | (low & 0x0f))
#define NT_SET_PCI_TYPE_FF(low)     (0x10 | (low & 0x0f))
#define NT_SET_PCI_TYPE_CF(low)     (0x20 | (low & 0x0f))
#define NT_SET_PCI_TYPE_FC(low)     (0x30 | (low & 0x0f))
#define NT_GET_PCI_TYPE(n_pci)      (n_pci >> 4)

#define NT_GET_SF_DL(n_pci)         (0x0f & n_pci)
#define NT_GET_CF_SN(n_pci)         (0x0f & n_pci)
#define NT_GET_FC_FS(n_pci)         (0x0f & n_pci)


#define NT_XMIT_FC_BS               (0)
#define NT_XMIT_FC_STMIN            (0x0A)       /* 0x00-0x7f, range:0ms-127ms */
#define NT_WFTmax                   (10)
#define TIMEOUT_N_CR                (150)        /* 150 ms */
#define TIMEOUT_N_BS                (75)        /* 75 ms */
#endif
/****************EOF****************/
