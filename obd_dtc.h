/***************************************************************************//**
    \file          obd-dtc.h
    \author        
    \mail          
    \version       0
    \date          2016-10-10
    \description   obd dtc code, according SIO 14229(2006) D.2.2
*******************************************************************************/
#ifndef	__OBD_DTC_H_
#define	__OBD_DTC_H_
/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "uds_type.h"

/*******************************************************************************
    Type Definition
*******************************************************************************/

#define DTC_FORMAT_15031                  (0x00)
#define DTC_FORMAT_14229                  (0x01)
#define DTC_AVAILABILITY_STATUS_MASK      (0x7F)

#define DTC_TEST_PASSED     (1u)
#define DTC_TEST_NORESULT   (0u)
#define DTC_TEST_FAILED     (-1u)

#define DTC_OFF             (TRUE)
#define DTC_ON              (FALSE)

/* uds DTC group */
typedef enum __OBD_DTC_TEST__
{
	OBD_DTC_TEST_NORESULT = 0,
	OBD_DTC_TEST_PASSED,
	OBD_DTC_TEST_FAILED
}obd_dtc_test_t;

/* uds DTC group */
typedef enum __UDS_DTC_GROUP__
{
	UDS_DTC_GROUP_EMISSION = 0,
	UDS_DTC_GROUP_NETWORK = 0xC00000,
	UDS_DTC_GROUP_ALL = 0xFFFFFF
}uds_dtc_group;

typedef enum __OBD_DTC_NAME_T__
{
    DTC_FUEL_SENSOR_BELOW = 0,
    DTC_FUEL_SENSOR_ABOVE,
    DTC_BATT_VOLTAG_BELOW,
    DTC_BATT_VOLTAG_ABOVE,
    DTC_CAN_BUS_OFF,
    DTC_COM_LOST_EMS,
    DTC_COM_LOST_TCU,
    DTC_COM_LOST_ABS,
    DTC_COM_LOST_TPMS,
    DTC_COM_LOST_EPB,
    DTC_COM_LOST_ESP,
    DTC_COM_LOST_EPS,
    DTC_COM_LOST_BCM,
    DTC_COM_LOST_SRS,
    DTC_COM_LOST_DVD,
    DTC_COM_LOST_PDC,
    DTC_COM_LOST_ALS,
    DTC_COM_LOST_PEPS,
    DTC_COM_LOST_AC,
    DTC_COM_LOST_AVM,
    DTC_LIGHT_ERROR_BREAK,
    DTC_LIGHT_ERROR_EPC,
    DTC_LIGHT_ERROR_TCU,
    DTC_LIGHT_ERROR_AIR_BAG,
    DTC_LIGHT_ERROR_ESC,
    DTC_LIGHT_ERROR_ABS,
    DTC_LIGHT_ERROR_MIL,
    DTC_LIGHT_ERROR_EPB_FAULT,
    DTC_LIGHT_ERROR_ESC_OFF,
    OBD_DTC_CNT
}obd_dtc_name_t;


/*******************************************************************************
    Function  Definition
*******************************************************************************/
/**
 * obd_dtc_ctrl - set the dtc off flag
 *
 * @val : dtc off value, TRUE or FALSE
 *
 * returns:
 *     void
 */
void
obd_dtc_ctrl (bool_t val);
/**
 * obd_update_dtc - manage the dtc status bits in a fault test operation cycle
 *
 * @dtc_n : dtc index
 * @test_result : obd fault detect result
 *
 * returns:
 *     void
 *
 */
void
uds_update_obddtc (uint16_t dtc_n, obd_dtc_test_t test_result);
/**
 * uds_load_obddtc - load obd dtc data from eeprom to ram
 *
 * @void : 
 *
 * returns:
 *     void
 */
void
uds_load_obddtc (void);

/**
 * uds_save_obddtc - save obd dtc data to eeprom
 *
 * @void : 
 *
 * returns:
 *     void
 */
void
uds_save_obddtc (void);
#endif
/****************EOF****************/
