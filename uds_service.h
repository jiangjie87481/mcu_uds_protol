/***************************************************************************//**
    \file          uds-service.h
    \author        
    \mail          
    \version       0
    \date          2016-09-28
    \description   uds service
*******************************************************************************/
#ifndef	__UDS_SERVICE_H_
#define	__UDS_SERVICE_H_
/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "network_layer.h"

/*******************************************************************************
    Type Definition
*******************************************************************************/


#define UDS_GLOBAL
#define UDS_CAN_ID_STD


#define UDS_RSP_LEN_MAX   		    0xFF  /* base on how many did and dtc*/

typedef enum __UDS_NRC_ENUM__
{
	NRC_GENERAL_REJECT=0x10,
	NRC_SERVICE_NOT_SUPPORTED=0x11,
	NRC_SUBFUNCTION_NOT_SUPPORTED = 0x12,
	NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT=0x13,
	NRC_CONDITIONS_NOT_CORRECT=0x22,
	NRC_REQUEST_SEQUENCE_ERROR=0x24,
	NRC_REQUEST_OUT_OF_RANGE=0x31,
    NRC_SECURITY_ACCESS_DENIED=0x33,
    NRC_INVALID_KEY=0x35,
    NRC_EXCEEDED_NUMBER_OF_ATTEMPTS=0x36,
    NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED=0x37,
    NRC_GENERAL_PROGRAMMING_FAILURE=0x72,
    NRC_SERVICE_BUSY=0x78,
	NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION=0x7F,
}uds_nrc_em;


/********************/
#define UDS_PHYSREQ_CANID    0x766
#define UDS_FUNCREQ_CANID    0x7DF
#define UDS_RESPOND_CANID    0x706

#ifdef UDS_CAN_ID_STD
#define UDS_VALID_FRAME_LEN     8
#else
#define UDS_VALID_FRAME_LEN     7
#endif

#define SID_10        (0x10) /* SessionControl */
#define SID_11        (0x11) /* ECUReset */
#define SID_14        (0x14) /* ClearDTC */
#define SID_18        (0x18) /* KWPReadDTC */
#define SID_19        (0x19) /* ReadDTC */
#define SID_22        (0x22) /* ReadID */
#define SID_27        (0x27) /* SecurityAccess */
#define SID_2E        (0x2E) /* WriteID */
#define SID_2F        (0x2F) /* InputOutputControlID */
#define SID_28        (0x28) /* CommunicationControl */
#define SID_31        (0x31) /* RoutineControl */
#define SID_3E        (0x3E) /* TesterPresent */
#define SID_85        (0x85) /* ControlDTCSetting */


#define SID_10_MIN_LEN      (0x02u)
#define SID_11_MIN_LEN      (0x02u)
#define SID_27_MIN_LEN      (0x02u)
#define SID_28_MIN_LEN      (0x03u)
#define SID_3E_MIN_LEN      (0x02u)
#define SID_85_MIN_LEN      (0x02u)
#define SID_22_MIN_LEN      (0x03u)
#define SID_2E_MIN_LEN      (0x04u)
#define SID_14_MIN_LEN      (0x04u) /* 3 Bytes DTC */
#define SID_19_MIN_LEN      (0x02u)
#define SID_2F_MIN_LEN      (0x04u)
#define SID_31_MIN_LEN      (0x04u)


#define UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP(byte)    ((byte >> 7u)&0x01u)
#define UDS_GET_SUB_FUNCTION(byte)     (byte & 0x7fu)


#define POSITIVE_RSP 			0x40
#define NEGATIVE_RSP 			0x7F
#define USD_GET_POSITIVE_RSP(server_id)         (POSITIVE_RSP + server_id)



#define TIMEOUT_FSA          (10000) /* 10s */
#define TIMEOUT_S3server     (5000)  /* 5000ms */
/* uds app layer timer */
typedef enum __UDS_TIMER_T__
{
	UDS_TIMER_FSA = 0,
	UDS_TIMER_S3server,
	UDS_TIMER_CNT
}uds_timer_t;
#define P2_SERVER               (0x32)    /* 50ms */
#define P2X_SERVER              (0x190)  /* 4000ms / 10 */


/* uds diagnostic session */
typedef enum __UDS_SESSION_T_
{
	UDS_SESSION_NONE = 0,
	UDS_SESSION_STD,
	UDS_SESSION_PROG,
	UDS_SESSION_EOL
}uds_session_t;

/* uds reset type */
typedef enum __UDS_RESET_T_
{
	UDS_RESET_NONE = 0,
	UDS_RESET_HARD,
	UDS_RESET_KEYOFFON,
	UDS_RESET_SOFT
}uds_reset_t;

/* uds security access level */
typedef enum __UDS_SA_LV__
{
	UDS_SA_NON = 0,
	UDS_SA_LV1,
	UDS_SA_LV2,
}uds_sa_lv;


/* uds Communication control type */
typedef enum __UDS_CC_TYPE__
{
	UDS_CC_TYPE_NONE = 0,
	UDS_CC_TYPE_NORMAL,
	UDS_CC_TYPE_NM,
	UDS_CC_TYPE_NM_NOR
}uds_cc_type;

/* uds Communication control mode */
typedef enum __UDS_CC_MODE__
{
	UDS_CC_MODE_RX_TX = 0,
	UDS_CC_MODE_RX_NO,
	UDS_CC_MODE_NO_TX,
	UDS_CC_MODE_NO_NO
}uds_cc_mode;

/* uds DTC setting */
typedef enum __UDS_DTC_SETTING__
{
	UDS_DTC_SETTING_NONE = 0,
	UDS_DTC_SETTING_ON,
	UDS_DTC_SETTING_OFF
}uds_dtc_setting;



/* uds io control type */
typedef enum __UDS_IOCTRL_PARAM__
{
	UDS_IOCTRL_RETURN_TO_ECU = 0,
	UDS_IOCTRL_RETSET_TO_DEFAULT = 0x01,
	UDS_IOCTRL_FREEZE_CURRENT_STATE = 0x02,
	UDS_IOCTRL_SHORT_ADJUSTMENT = 0x03
}uds_ioctrl_param;

/* uds routine control type */
typedef enum __UDS_ROUTINE_CTRL_TYPE__
{
	UDS_ROUTINE_CTRL_NONE = 0,
	UDS_ROUTINE_CTRL_START = 0x01,
	UDS_ROUTINE_CTRL_STOP = 0x02,
	UDS_ROUTINE_CTRL_REQUEST_RESULT = 0x03
}uds_routine_ctrl_type;


#define DTC_SUPPORT_STATUS                (0x09)
#define DTC_GROUP_ALL                     (0xFFFFFF)

#define REPORT_DTC_NUMBER_BY_STATUS_MASK  (0x01)
#define REPORT_DTC_BY_STATUS_MASK         (0x02)
#define REPORT_DTC_SNOPSHOT_BY_DTC_NUMBER (0x04)
#define REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER (0x06)
#define REPORT_SUPPORTED_DTC              (0x0a)


#define UDS_SEED_LENGTH                   (0x04)
#define UDS_REQUEST_SEED                  (0x01)
#define UDS_SEND_KEY                      (0x02)
#define UDS_FAS_MAX_TIMES                 (0x02)  /* failed security access */


#define ZERO_SUBFUNCTION                  (0x00)



/*******************************************************************************
    Global Varaibles Extern
*******************************************************************************/
extern bool_t dis_normal_xmit;
extern bool_t dis_normal_recv;

/*******************************************************************************
    Function  Extern
*******************************************************************************/

/**
 * uds_get_frame - uds get a can frame, then transmit to network
 *
 * @func_addr : 0 - physical addr, 1 - functional addr
 * @frame_buf : uds can frame data buffer
 * @frame_dlc : uds can frame length
 *
 * returns:
 *     void
 */
extern void
uds_get_frame (uint8_t func_addr, uint8_t frame_buf[], uint8_t frame_dlc);

/**
 * uds_main - uds main loop, should be schedule every 1 ms
 *
 * @void  :
 *
 * returns:
 *     void
 */
extern void
uds_main (void);

/**
 * uds_init - uds user layer init
 *
 * @void  :
 *
 * returns:
 *     void
 */
extern int
uds_init (void);

#endif
