/***************************************************************************//**
    \file          uds-service.c
    \author        
    \mail          
    \version       0.03 - CANoe Passed
    \date          2016-09-27
    \description   uds network code, base on ISO 14229
*******************************************************************************/

/*******************************************************************************
    Include Files
*******************************************************************************/
#include "network_layer.h"
#include "uds_service.h"
#include "uds_util.h"
#include "uds_status.h"
#include "uds_support.h"
#include "obd_dtc.h"
/*******************************************************************************
    Type declaration
*******************************************************************************/
typedef struct __UDS_SERVICE_T__
{
    uint8_t uds_sid;
    uint8_t uds_min_len;
    void (* uds_service)  (uint8_t *, uint16_t);
    bool_t std_spt;   /* default session support */
    bool_t ext_spt;   /* extended session support */
    bool_t fun_spt;   /* function address support */
	bool_t ssp_spt;   /* subfunction suppress PosRsp bit support */
    uds_sa_lv uds_sa; /* security access */
}uds_service_t;

/*******************************************************************************
    Function  declaration
*******************************************************************************/
static void
uds_service_10 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_11 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_27 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_28 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_3E (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_85 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_22 (uint8_t msg_buf[], uint16_t msg_dlc); 
static void
uds_service_2E (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_14 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_19 (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_2F (uint8_t msg_buf[], uint16_t msg_dlc);
static void
uds_service_31 (uint8_t msg_buf[], uint16_t msg_dlc);

/*******************************************************************************
    Private Varaibles
*******************************************************************************/
uint8_t uds_session = UDS_SESSION_NONE;
static uint8_t org_seed_buf[UDS_SEED_LENGTH];
static uint8_t req_seed = 0;

/* current security access status */
static uds_sa_lv curr_sa = UDS_SA_NON;

/* uds user layer timer */
static uint32_t uds_timer[UDS_TIMER_CNT] = {0};

static uint8_t uds_fsa_cnt = 0;

static bool_t ssp_flg;

static const uds_service_t uds_service_list[] =
{
	{SID_10, SID_10_MIN_LEN, uds_service_10, TRUE,  TRUE, TRUE,  TRUE,  UDS_SA_NON},
    {SID_11, SID_11_MIN_LEN, uds_service_11, FALSE, TRUE, TRUE,  TRUE,  UDS_SA_NON},
    {SID_27, SID_27_MIN_LEN, uds_service_27, FALSE, TRUE, FALSE, FALSE, UDS_SA_NON},
    {SID_28, SID_28_MIN_LEN, uds_service_28, FALSE, TRUE, TRUE,  TRUE,  UDS_SA_NON},
    {SID_3E, SID_3E_MIN_LEN, uds_service_3E, TRUE,  TRUE, TRUE,  TRUE,  UDS_SA_NON},
    {SID_85, SID_85_MIN_LEN, uds_service_85, FALSE, TRUE, TRUE,  TRUE,  UDS_SA_NON},
    {SID_22, SID_22_MIN_LEN, uds_service_22, TRUE,  TRUE, TRUE,  FALSE, UDS_SA_NON},
    {SID_2E, SID_2E_MIN_LEN, uds_service_2E, FALSE, TRUE, FALSE, FALSE, UDS_SA_LV1},
    {SID_14, SID_14_MIN_LEN, uds_service_14, TRUE,  TRUE, TRUE,  FALSE, UDS_SA_NON},
    {SID_19, SID_19_MIN_LEN, uds_service_19, TRUE,  TRUE, TRUE,  FALSE, UDS_SA_NON},
    {SID_2F, SID_2F_MIN_LEN, uds_service_2F, FALSE, TRUE, FALSE, FALSE, UDS_SA_LV1},
    {SID_31, SID_31_MIN_LEN, uds_service_31, FALSE, TRUE, FALSE, FALSE, UDS_SA_LV1},
};

#define UDS_SERVICE_NUM (sizeof (uds_service_list) / sizeof (uds_service_t))

/*******************************************************************************
    Global Varaibles
*******************************************************************************/
bool_t dis_normal_xmit;
bool_t dis_normal_recv;
/*******************************************************************************
    Function  Definition
*******************************************************************************/
/**
 * uds_timer_start - start uds timer
 *
 * void :
 *
 * returns:
 *     void
 */
static void
uds_timer_start (uint8_t num)
{
	if (num >= UDS_TIMER_CNT) return;

	if (num == UDS_TIMER_FSA)
        uds_timer[UDS_TIMER_FSA] = TIMEOUT_FSA;
	if (num == UDS_TIMER_S3server)
	    uds_timer[UDS_TIMER_S3server] = TIMEOUT_S3server;
}


static void
uds_timer_stop (uint8_t num)
{
	if (num >= UDS_TIMER_CNT) return;

    uds_timer[num] = 0;
}

/**
 * uds_timer_run - run a uds timer, should be invoked per 1ms
 *
 * void :
 *
 * returns:
 *     0 - timer is not running, 1 - timer is running, -1 - a timeout occur
 */
static int
uds_timer_run (uint8_t num)
{
	if (num >= UDS_TIMER_CNT) return 0;

    if (uds_timer[num] == 0)
	{
        return 0;
	}
	else if (uds_timer[num] == 1)
	{
		uds_timer[num] = 0;
	    return -1;
	}
	else
	{
	    /* if (uds_timer[num] > 1) */
		uds_timer[num]--;
	    return 1;
	}

}

/**
 * uds_timer_chk - check a uds timer and stop it
 *
 * num :
 *
 * returns:
 *     0 - timer is not running, 1 - timer is running,
 */
static int
uds_timer_chk (uint8_t num)
{
	if (num >= UDS_TIMER_CNT) return 0;

	if (uds_timer[num] > 0)
	    return 1;
	else
		return 0;
}

/**
 * uds_no_response - uds response nothing
 *
 * @void :
 *
 * returns:
 *
 */
static void
uds_no_response (void)
{
	return;
}
/**
 * uds_negative_rsp - uds negative response
 *
 * @sid :
 * @rsp_nrc :
 *
 * returns:
 *     0 - ok, other - wrong
 */
static void
uds_negative_rsp (uint8_t sid, uint8_t rsp_nrc)
{
	uint8_t temp_buf[8] = {0};

    if (rsp_nrc != NRC_SERVICE_BUSY)
        uds_timer_start (UDS_TIMER_S3server);

    if (g_tatype == N_TATYPE_FUNCTIONAL)
	{
		if (rsp_nrc == NRC_SERVICE_NOT_SUPPORTED ||
		    rsp_nrc == NRC_SUBFUNCTION_NOT_SUPPORTED ||
			rsp_nrc == NRC_REQUEST_OUT_OF_RANGE)
		    return;
	}
	temp_buf[0] = NEGATIVE_RSP;
	temp_buf[1] = sid;
	temp_buf[2] = rsp_nrc;
	network_send_udsmsg (temp_buf, 3);
	return;
}

/**
 * uds_negative_rsp - uds positive response
 *
 * @data :
 * @len  :
 *
 * returns:
 *     0 - ok, other - wrong
 */
static void
uds_positive_rsp (uint8_t data[], uint16_t len)
{
	uds_timer_start (UDS_TIMER_S3server);
	/* SuppressPosRspMsg is true */
	if (ssp_flg == TRUE) return;
	network_send_udsmsg (data, len);
    return;
}



static int16_t
eeprom_write (void)         // we should write the rwdata_list which in eeprom to flash
{
	return 0;
}

/**
 * uds_check_len - check uds request message length
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     bool
 */
static bool_t
uds_check_len (uint8_t msg_buf[], uint16_t msg_dlc)
{
	bool_t result;
	uint8_t subfunction;
	uint8_t sid;
	sid = msg_buf[0];

	if (msg_dlc < 2)
	    return FALSE;

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);

	switch (sid)
	{
		case SID_10:
		{
		    if (msg_dlc == SID_10_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_11:
		{
			if (msg_dlc == SID_11_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_14:
		{
			if (msg_dlc == SID_14_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_19:
		{
		    if ((subfunction == REPORT_DTC_NUMBER_BY_STATUS_MASK && msg_dlc != (SID_19_MIN_LEN+1))
			 || (subfunction == REPORT_DTC_BY_STATUS_MASK && msg_dlc != (SID_19_MIN_LEN+1))
			 || (subfunction == REPORT_SUPPORTED_DTC && msg_dlc != SID_19_MIN_LEN))
			{
				result = FALSE;
			}
			else
			{
				result = TRUE;
			}
		}
		break;
		case SID_22:
		{
			if (msg_dlc == SID_22_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_27:
		{
			if ((subfunction == UDS_REQUEST_SEED && msg_dlc != SID_27_MIN_LEN)
			 || (subfunction == UDS_SEND_KEY && msg_dlc != (SID_27_MIN_LEN+4)))

			{
				result = FALSE;
			}
			else
			{
				result = TRUE;
			}
		}
		break;
		case SID_2E:
		{
		    if (msg_dlc > SID_2E_MIN_LEN)
			    result = TRUE;
			else
			    result = FALSE;
		}
		break;
		case SID_2F:
		{
			if (msg_dlc > SID_2F_MIN_LEN)
			    result = TRUE;
			else
			    result = FALSE;
		}
		break;
		case SID_28:
		{
			if (msg_dlc == SID_28_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_31:
		{
			if (msg_dlc > SID_31_MIN_LEN)
			    result = TRUE;
			else
			    result = FALSE;
		}
		break;
		case SID_3E:
		{
			if (msg_dlc == SID_3E_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		case SID_85:
		{
			if (msg_dlc == SID_85_MIN_LEN)
		        result = TRUE;
		    else
			    result = FALSE;
		}
		break;
		default:
		    result = FALSE;
		break;
	}

    return result;
}
/**
 * uds_service_10 - uds service 0x10, DiagnosticSessionControl
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_10 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);

	rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_10);
	rsp_buf[1] = subfunction;
	rsp_buf[2] = (uint8_t)(P2_SERVER >> 8);
	rsp_buf[3] = (uint8_t)(P2_SERVER & 0x00ff);
	rsp_buf[4] = (uint8_t)(P2X_SERVER >> 8);
	rsp_buf[5] = (uint8_t)(P2X_SERVER & 0x00ff);
	switch (subfunction)
	{
		case UDS_SESSION_STD:
			curr_sa = UDS_SA_NON;
			uds_session = (uds_session_t)subfunction;
			uds_positive_rsp (rsp_buf, 6);
			break;
		case UDS_SESSION_EOL:
			curr_sa = UDS_SA_NON;
			uds_session = (uds_session_t)subfunction;
			uds_positive_rsp (rsp_buf, 6);
            uds_timer_start (UDS_TIMER_S3server);
			break;
		case UDS_SESSION_PROG:       // update programe flag here all this should be with 

			break; 
		default:
			uds_negative_rsp (SID_10,NRC_SUBFUNCTION_NOT_SUPPORTED);
			break;
	}
    //uds_negative_rsp (SID_10,NRC_CONDITIONS_NOT_CORRECT);
}


/**
 * uds_service_11 - uds service 0x11, ECUReset
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_11 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);
	if (subfunction == UDS_RESET_HARD)
	{
		rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_11);
		rsp_buf[1] = UDS_RESET_HARD;
		uds_positive_rsp (rsp_buf,2);
		//wait for reset
	   /**
		for (;;)
		{}
		*/
	}
	else
	{
		uds_negative_rsp (SID_11,NRC_SUBFUNCTION_NOT_SUPPORTED);
	}
	//uds_negative_rsp (SID_11,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_27 - uds service 0x27, SecurityAccess
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_27 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];
	uint16_t i;

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);

	switch (subfunction)
	{
		case UDS_REQUEST_SEED:
		{
			if (uds_timer_chk (UDS_TIMER_FSA) > 0)
			{
				uds_negative_rsp (SID_27,NRC_REQUIRED_TIME_DELAY_NOT_EXPIRED);
			}
			req_seed = 1;
			rsp_buf[0] = USD_GET_POSITIVE_RSP (SID_27);
			rsp_buf[1] = subfunction;
			for (i = 0; i < UDS_SEED_LENGTH; i++) {
				if (curr_sa == UDS_SA_LV1)
					org_seed_buf[i] = 0;
				else
				    org_seed_buf[i] = rand_u8();
				rsp_buf[2+i] = org_seed_buf[i];
			}
			uds_positive_rsp (rsp_buf,UDS_SEED_LENGTH+2);

			//printf_os("SecurityAccess seed:%d %d %d %d\r\n",org_seed_buf[0], org_seed_buf[1], org_seed_buf[2], org_seed_buf[3],);
			break;
		}
		case UDS_SEND_KEY:
		{
			if (req_seed == 0)
			{
				uds_negative_rsp (SID_27,NRC_REQUEST_SEQUENCE_ERROR);
				break;
			}
			req_seed = 0;
			if (!uds_security_access (&msg_buf[2], org_seed_buf))
			{
				rsp_buf[0] = USD_GET_POSITIVE_RSP (SID_27);
				rsp_buf[1] = subfunction;
				uds_positive_rsp (rsp_buf,2);
				curr_sa = UDS_SA_LV1;
				//printf_os("SA_ PASS\r\n");
			}
			else
			{
				uds_fsa_cnt++;
				if (uds_fsa_cnt >= UDS_FAS_MAX_TIMES) {
					uds_timer_start (UDS_TIMER_FSA);
					uds_negative_rsp (SID_27,NRC_EXCEEDED_NUMBER_OF_ATTEMPTS);
				} else {
					uds_negative_rsp (SID_27,NRC_INVALID_KEY);
				}
			}
			break;
		}
		default:
			uds_negative_rsp (SID_27,NRC_SUBFUNCTION_NOT_SUPPORTED);
			break;
	}
	//uds_negative_rsp (SID_27,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_28 - uds service 0x28, CommunicationControl
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_28 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];
	uint8_t cc_type;

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);
	cc_type = msg_buf[2];


    switch (subfunction)          // we need add communication control flag here to support service28
	{
		case UDS_CC_MODE_RX_TX:
		    if (cc_type == UDS_CC_TYPE_NORMAL || cc_type == UDS_CC_TYPE_NM || cc_type == UDS_CC_TYPE_NM_NOR)
			{
			    dis_normal_xmit = FALSE;
				dis_normal_recv = FALSE;
				rsp_buf[0] = USD_GET_POSITIVE_RSP (SID_28);
				rsp_buf[1] = subfunction;
				uds_positive_rsp (rsp_buf,2);
			}
			else
			{
			    uds_negative_rsp (SID_28,NRC_REQUEST_OUT_OF_RANGE);
			}
		    break;
		case UDS_CC_MODE_NO_NO:
		    if (cc_type == UDS_CC_TYPE_NORMAL || cc_type == UDS_CC_TYPE_NM || cc_type == UDS_CC_TYPE_NM_NOR)
			{
			    dis_normal_xmit = TRUE;
				dis_normal_recv = TRUE;
				rsp_buf[0] = USD_GET_POSITIVE_RSP (SID_28);
				rsp_buf[1] = subfunction;
				uds_positive_rsp (rsp_buf,2);
			}
			else
			{
			    uds_negative_rsp (SID_28,NRC_REQUEST_OUT_OF_RANGE);
			}
			break;
		default:
		    uds_negative_rsp (SID_28,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
	}

   //uds_negative_rsp (SID_28,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_3E - uds service 0x3E, TesterPresent
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_3E (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t subfun_indication_bit; 
	uint8_t rsp_buf[8];

   /**
	* Any TesterPresent (3E hex) request message that is received 
	* during processing of another request message can be 
    * ignored by the server
	* Note,Cant passed canoe test
    */
	//if (uds_timer_chk (UDS_TIMER_S3server) == 0 && uds_session != UDS_SESSION_STD)
	 //  return;
	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);
	subfun_indication_bit = (msg_buf[1] & 0x80) >> 7;      // get the subfunctionIndicationBit
	if (subfunction == ZERO_SUBFUNCTION) 
	{
		if (!subfun_indication_bit){      // only when the bit is false we send positive response
			rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_3E);
			rsp_buf[1] = subfunction;
			uds_positive_rsp (rsp_buf,2);
		}else{
			// no response
		}
	}
	else
	{
		uds_negative_rsp (SID_3E,NRC_SUBFUNCTION_NOT_SUPPORTED);
	}
}

/**
 * uds_service_85 - uds service 0x85, ControlDTCSetting
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_85 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];
	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);

	switch (subfunction)
	{
		case UDS_DTC_SETTING_ON:
		    obd_dtc_ctrl(DTC_ON);
		    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_85);
		    rsp_buf[1] = subfunction;
		    uds_positive_rsp (rsp_buf,2);
		    break;
		case UDS_DTC_SETTING_OFF:
		    obd_dtc_ctrl(DTC_OFF);
		    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_85);
		    rsp_buf[1] = subfunction;
		    uds_positive_rsp (rsp_buf,2);
		    break;
		default:
		    uds_negative_rsp (SID_85,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
	}
    //uds_negative_rsp (SID_85,NRC_CONDITIONS_NOT_CORRECT);
}


/**
 * uds_service_22 - uds service 0x22, ReadDataByIdentifier
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_22 (uint8_t msg_buf[], uint16_t msg_dlc)
{
	uint8_t rsp_buf[UDS_RSP_LEN_MAX];
	uint16_t did;
	uint16_t rsp_len;
    uint16_t msg_pos, did_n;
	bool_t find_did;

    rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_22);
	rsp_len = 1;
    for (msg_pos = 1; msg_pos < msg_dlc; msg_pos+=2)
	{
	    did = ((uint16_t)msg_buf[msg_pos]) << 8;
	    did |= msg_buf[msg_pos+1];
        
		find_did = FALSE;
		for (did_n = 0; did_n < RWDATA_NUM; did_n++)
		{
			if (rwdata_list[did_n].did == did)
			{
				find_did = TRUE;
                rsp_buf[rsp_len++] = msg_buf[msg_pos];
				rsp_buf[rsp_len++] = msg_buf[msg_pos+1];
				memcpy (&rsp_buf[rsp_len], rwdata_list[did_n].p_data, rwdata_list[did_n].dlc);
				rsp_len += rwdata_list[did_n].dlc;
			    break;
			}
		}
		if (find_did == FALSE)
			break;
	}

	if (find_did == TRUE)
        uds_positive_rsp (rsp_buf,rsp_len);
	else
	    uds_negative_rsp (SID_22,NRC_REQUEST_OUT_OF_RANGE);
    
    //uds_negative_rsp (SID_22,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_2E - uds service 0x2E, WriteDataByIdentifier
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_2E (uint8_t msg_buf[], uint16_t msg_dlc)
{
	uint8_t rsp_buf[8];
	uint16_t did;
    uint16_t did_n;
	int16_t write_len;
	bool_t find_did;
	bool_t vali_dlc;

	did = ((uint16_t)msg_buf[1]) << 8;
	did |= msg_buf[2];
	
	find_did = FALSE;
	vali_dlc = FALSE;
	for (did_n = 0; did_n < RWDATA_NUM; did_n++)
	{
		if (rwdata_list[did_n].did == did)
		{
			if (rwdata_list[did_n].rw_mode == UDS_RWDATA_RDONLY)
			    break;
			find_did = TRUE;
		    if (msg_dlc != (rwdata_list[did_n].dlc + 3))
			    break;
			vali_dlc = TRUE;
			rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_2E);
			rsp_buf[1] = msg_buf[1];
			rsp_buf[2] = msg_buf[2];
			memcpy (rwdata_list[did_n].p_data, &msg_buf[3], rwdata_list[did_n].dlc);
			write_len = eeprom_write ();
			break;
		}
	}

	if (find_did == TRUE)
	{
		if (vali_dlc == TRUE)
		{
			if (write_len >= 0)
				uds_positive_rsp (rsp_buf,3);
			else
				uds_negative_rsp (SID_2E,NRC_GENERAL_PROGRAMMING_FAILURE);
		}
		else
		{
			uds_negative_rsp (SID_2E,NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT);
		}
	}
	else
	{
	    uds_negative_rsp (SID_2E,NRC_REQUEST_OUT_OF_RANGE);
	}
    
    //uds_negative_rsp (SID_2E,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_14 - uds service 0x14, ClearDiagnosticInformation
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_14 (uint8_t msg_buf[], uint16_t msg_dlc)
{
	uint8_t rsp_buf[8];
	uint32_t dtc_group;
	dtc_group = 0;
	dtc_group |= ((uint32_t)msg_buf[1]) << 16;
	dtc_group |= ((uint32_t)msg_buf[2]) << 8;
	dtc_group |= ((uint32_t)msg_buf[3]) << 0;

	if (dtc_group == UDS_DTC_GROUP_ALL)
	{
		/* clear all groups dtc */
		clear_dtc_by_group (dtc_group);
		rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_14);
		uds_positive_rsp (rsp_buf,1);
	}
	else
	{
		uds_negative_rsp (SID_14,NRC_REQUEST_OUT_OF_RANGE);
	}	
    //uds_negative_rsp (SID_14,NRC_CONDITIONS_NOT_CORRECT);
}


/**
 * uds_service_19 - uds service 0x19, ReadDTCInformation
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_19 (uint8_t msg_buf[], uint16_t msg_dlc)
{
	uint8_t subfunction;
    uint8_t rsp_buf[UDS_RSP_LEN_MAX];

	subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);

    switch (subfunction)
	{
		case REPORT_DTC_NUMBER_BY_STATUS_MASK:
		{
			uint8_t dtc_status_mask;
			uint16_t dtc_count;
			dtc_status_mask = msg_buf[2];
			dtc_count = get_dtc_number_by_status_mask (dtc_status_mask);
			rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_19);
			rsp_buf[1] = subfunction;
			rsp_buf[2] = DTC_AVAILABILITY_STATUS_MASK;
			rsp_buf[3] = DTC_FORMAT_14229;
			host_to_cans (&rsp_buf[4], dtc_count);
			uds_positive_rsp (rsp_buf,6);
		    break;
		}
		case REPORT_DTC_BY_STATUS_MASK:
		{
			uint8_t dtc_status_mask;
			uint16_t dtc_dlc;
			dtc_status_mask = msg_buf[2];
			rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_19);
		    rsp_buf[1] = subfunction;
		    rsp_buf[2] = DTC_AVAILABILITY_STATUS_MASK;
		    dtc_dlc = get_dtc_by_status_mask (&rsp_buf[3], UDS_RSP_LEN_MAX-3, dtc_status_mask);
		    uds_positive_rsp (rsp_buf, 3+dtc_dlc);
		    break;
		}
		case REPORT_DTC_SNOPSHOT_BY_DTC_NUMBER:
		    uds_negative_rsp (SID_19,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
		case REPORT_DTC_EXTENDED_DATA_RECORD_BY_DTC_NUMBER:
		    uds_negative_rsp (SID_19,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
		case REPORT_SUPPORTED_DTC:
		{
			uint16_t dtc_dlc;
			rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_19);
		    rsp_buf[1] = subfunction;
		    rsp_buf[2] = DTC_AVAILABILITY_STATUS_MASK;
		    dtc_dlc = get_supported_dtc (&rsp_buf[3], UDS_RSP_LEN_MAX-3);
		    uds_positive_rsp (rsp_buf, 3+dtc_dlc);
		    break;
		}
		default:
	        uds_negative_rsp (SID_19,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
	}

}

/**
 * uds_service_2F - uds service 0x2F, InputOutputControlByIdentifier
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_2F (uint8_t msg_buf[], uint16_t msg_dlc)
{
	uint8_t rsp_buf[8];
	uint8_t ioctrl_param;
	uint16_t did;
    uint16_t did_n;
	bool_t find_did;
	bool_t vali_par;

	did = ((uint16_t)msg_buf[1]) << 8;
	did |= msg_buf[2];
	ioctrl_param = msg_buf[3];


    if ((ioctrl_param == UDS_IOCTRL_RETURN_TO_ECU && msg_dlc != 4) ||
	    (ioctrl_param == UDS_IOCTRL_SHORT_ADJUSTMENT && msg_dlc < (2 + 4)))
	{
		uds_negative_rsp (SID_2F,NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT);
		return;
	}
	find_did = FALSE;
	for (did_n = 0; did_n < IOCTRL_NUM; did_n++)
	{
		if (ioctrl_list[did_n].did == did)
		{
			find_did = TRUE;
			break;
		}
	}

	if (find_did == TRUE)
	{
		vali_par = FALSE;
		switch (ioctrl_param)
		{
			case UDS_IOCTRL_RETURN_TO_ECU:
			{
				vali_par = TRUE;
				ioctrl_list[did_n].enable = FALSE;
				if (ioctrl_list[did_n].stop_ioctrl != NULL)
				    ioctrl_list[did_n].stop_ioctrl ();
				break;
			}
			case UDS_IOCTRL_SHORT_ADJUSTMENT:
			{
				vali_par = TRUE;
				memcpy (ioctrl_list[did_n].p_data, &msg_buf[4], rwdata_list[did_n].dlc);
				ioctrl_list[did_n].enable = TRUE;
				if (ioctrl_list[did_n].init_ioctrl != NULL)
				    ioctrl_list[did_n].init_ioctrl ();
				break;
			}
			default:
				vali_par = FALSE;
				break;
		}

		rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_2F);
		rsp_buf[1] = msg_buf[1];
		rsp_buf[2] = msg_buf[2];
		rsp_buf[3] = msg_buf[3];

		if (vali_par == TRUE)
			uds_positive_rsp (rsp_buf,4);
		else
			uds_negative_rsp (SID_2F,NRC_GENERAL_REJECT);
	}
	else
	{
	    uds_negative_rsp (SID_2F,NRC_REQUEST_OUT_OF_RANGE);
	}

    //uds_negative_rsp (SID_2F,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_service_31 - uds service 0x31, RoutineControl
 *
 * @msg_buf :
 * @msg_dlc :
 *
 * returns:
 *     void
 */
static void
uds_service_31 (uint8_t msg_buf[], uint16_t msg_dlc)
{
    uint8_t subfunction;
	uint8_t rsp_buf[8];
	uint16_t rid;
	uint16_t rid_n;
	bool_t find_rid;

    subfunction = UDS_GET_SUB_FUNCTION (msg_buf[1]);
	rid = ((uint16_t)msg_buf[2]) << 8;
	rid |= msg_buf[3];

	find_rid = FALSE;
	for (rid_n = 0; rid_n < RTCTRL_NUM; rid_n++)
	{
		if (rtctrl_list[rid_n].rid == rid)
		{
			find_rid = TRUE;
			break;
		}
	}

	rsp_buf[0] = USD_GET_POSITIVE_RSP(SID_31);
	rsp_buf[1] = msg_buf[1];
	rsp_buf[2] = msg_buf[2];
	rsp_buf[3] = msg_buf[3];
    switch (subfunction)
	{
		case UDS_ROUTINE_CTRL_START:
		    if (find_rid == TRUE)
			{
                if (rtctrl_list[rid_n].rtst == UDS_RT_ST_RUNNING)
				{
                    uds_negative_rsp (SID_31,NRC_REQUEST_SEQUENCE_ERROR);
				}
				else
				{
					rtctrl_list[rid_n].init_routine ();
					uds_positive_rsp (rsp_buf,4);
				}
			}
			else
			{
				uds_negative_rsp (SID_31,NRC_REQUEST_OUT_OF_RANGE);
			}
		    break;
		case UDS_ROUTINE_CTRL_STOP:
		    if (find_rid == TRUE)
			{
                if (rtctrl_list[rid_n].rtst == UDS_RT_ST_IDLE)
				{
                    uds_negative_rsp (SID_31,NRC_REQUEST_SEQUENCE_ERROR);
				}
				else
				{
					rtctrl_list[rid_n].stop_routine ();
					uds_positive_rsp (rsp_buf,4);
				}
			}
			else
			{
				uds_negative_rsp (SID_31,NRC_REQUEST_OUT_OF_RANGE);
			}
		    break;
		case UDS_ROUTINE_CTRL_REQUEST_RESULT:
		    if (find_rid == TRUE)
			{
                if (rtctrl_list[rid_n].rtst == UDS_RT_ST_IDLE)
				{
                    uds_negative_rsp (SID_31,NRC_REQUEST_SEQUENCE_ERROR);
				}
				else
				{
					rsp_buf[4] = (uint8_t)rtctrl_list[rid_n].rtst;
				}
			}
			else
			{
				uds_negative_rsp (SID_31,NRC_REQUEST_OUT_OF_RANGE);
			}
		    break;
		default:
		    uds_negative_rsp (SID_31,NRC_SUBFUNCTION_NOT_SUPPORTED);
		    break;
	}
    //uds_negative_rsp (SID_2F,NRC_CONDITIONS_NOT_CORRECT);
}

/**
 * uds_dataff_indication - uds first frame indication callbacl
 *
 * @msg_dlc : first frame dlc
 *
 * returns:
 *     void
 */
static void
uds_dataff_indication (uint16_t msg_dlc)
{
	uds_timer_stop (UDS_TIMER_S3server);
}
/**
 * uds_data_indication - uds data request indication callback, 
 *
 * @msg_buf  :
 * @msg_dlc  :
 * @n_result :
 *
 * returns:
 *     void
 */

static void
uds_data_indication (uint8_t msg_buf[], uint16_t msg_dlc, n_result_t n_result)
{
	uint8_t i;
	uint8_t sid;
	uint8_t ssp;
	bool_t find_sid;
	bool_t session_spt;
	uds_timer_stop (UDS_TIMER_S3server);

    if (n_result != N_OK)
	{
		uds_timer_start (UDS_TIMER_S3server);
	    return;
	}

    sid = msg_buf[0];
	ssp = UDS_GET_SUB_FUNCTION_SUPPRESS_POSRSP (msg_buf[1]);
	find_sid = FALSE;
    for (i = 0; i < UDS_SERVICE_NUM; i++)
	{
        if (sid == uds_service_list[i].uds_sid)
		{
            if (uds_session == UDS_SESSION_STD)
			    session_spt = uds_service_list[i].std_spt;
			else
			    session_spt = uds_service_list[i].ext_spt;
			if (session_spt == TRUE)
			{
                if (g_tatype == N_TATYPE_FUNCTIONAL && uds_service_list[i].fun_spt == FALSE)
				{
					uds_no_response ();
				}
				else
				{
					if (uds_check_len(msg_buf, msg_dlc))
					{
                        if (curr_sa >= uds_service_list[i].uds_sa)
						{
							if (uds_service_list[i].ssp_spt == TRUE && ssp == 0x01)
							    ssp_flg = TRUE;
							else
							    ssp_flg = FALSE;
                            uds_service_list[i].uds_service (msg_buf, msg_dlc);
						}
						else
						{
							uds_negative_rsp (sid, NRC_SECURITY_ACCESS_DENIED);
						}
					}
					else
					{
						uds_negative_rsp (sid, NRC_INVALID_MESSAGE_LENGTH_OR_FORMAT);
					}
				}
				    
			}
			else
			{
                uds_negative_rsp (sid, NRC_SERVICE_NOT_SUPPORTED_IN_ACTIVE_SESSION);
			}
			find_sid = TRUE;
			break;
		}
	}

	if (find_sid == FALSE)
	{
		uds_negative_rsp (sid, NRC_SERVICE_NOT_SUPPORTED);
	}
}

/**
 * uds_data_confirm - uds response confirm
 *
 * @n_result :
 *
 * returns:
 *     void
 */
static void
uds_data_confirm (n_result_t n_result)
{

	uds_timer_start (UDS_TIMER_S3server);
}

/*******************************************************************************
    Function  Definition - external API
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
uds_get_frame (uint8_t func_addr, uint8_t frame_buf[], uint8_t frame_dlc)
{
	network_recv_frame (func_addr, frame_buf, frame_dlc);
}


/**
 * uds_main - uds main loop, should be schedule every 1 ms
 *
 * @void  :
 *
 * returns:
 *     void
 */
extern void
uds_main (void)
{
	network_main ();

	if (uds_timer_run (UDS_TIMER_S3server) < 0)
	{
		uds_session = UDS_SESSION_STD;
		curr_sa = UDS_SA_NON;
		uds_ioctrl_allstop ();
	}


	if (uds_timer_run (UDS_TIMER_FSA) < 0)
	{
        uds_fsa_cnt = 0;
	}

}


/**
 * uds_init - uds user layer init
 *
 * @void  :
 *
 * returns:
 *     0 - ok
 */
extern int
uds_init (void)
{
    nt_usdata_t usdata;

	uds_session = UDS_SESSION_STD;

    uds_load_rwdata ();

    usdata.ffindication = uds_dataff_indication;
    usdata.indication = uds_data_indication;
    usdata.confirm = uds_data_confirm;

    return network_reg_usdata (usdata);
}

/****************EOF****************/
