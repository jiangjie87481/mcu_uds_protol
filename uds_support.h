/***************************************************************************//**
    \file          uds-support.h
    \author        
    \mail          
    \version       0
    \date          2016-10-08
    \description
*******************************************************************************/
#ifndef	__UDS_SUPPORT_H_
#define	__UDS_SUPPORT_H_
/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "uds_type.h"
/*******************************************************************************
    Type Definition
*******************************************************************************/
/* uds read/write data read-write mode */
typedef enum __UDS_RWDATA_RW__
{
	UDS_RWDATA_RDONLY = 0,
	UDS_RWDATA_RDWR,
    UDS_RWDATA_RDWR_WRONCE,
    UDS_RWDATA_RDWR_INBOOT
}uds_rwdata_rw;

/* uds read/write data read-write mode */
typedef enum __UDS_RWDATA_TYPE__
{
	UDS_RWDATA_HEX = 0,
	UDS_RWDATA_ASCII,
	UDS_RWDATA_BCD
}uds_rwdata_type;

/* uds read/write data read-write mode */
typedef enum __UDS_RWDATA_STORE__
{
	UDS_RWDATA_RAM = 0,
	UDS_RWDATA_DFLASH,
	UDS_RWDATA_EEPROM
}uds_rwdata_store;

/* uds read/write data typedef */
typedef struct __UDS_RWDATA_T__
{
    uint16_t did;    /* 0100 - EFFF and F010 - F0FF for vehicleManufacturerSpecific */
    uint8_t *p_data;
    uint8_t dlc;
    uds_rwdata_rw rw_mode;
    uds_rwdata_store rw_store;
}uds_rwdata_t;

/* uds io control type */
typedef struct __UDS_IOCTRL_T__
{
    uint16_t did;
    uint8_t *p_data;
    uint8_t dlc;
    uint8_t default_value;
    uint8_t step;
    bool_t  enable;
    void (* init_ioctrl) (void);
    void (* stop_ioctrl) (void);
}uds_ioctrl_t;


/* uds routine control status */
typedef enum __UDS_ROUTINE_STATUS__
{
	UDS_RT_ST_IDLE = 0,
	UDS_RT_ST_RUNNING = 0x01,
	UDS_RT_ST_SUCCESS = 0x02,
	UDS_RT_ST_FAILED = 0x03
}uds_routine_status;

/* uds routine control */
typedef struct __UDS_RTCTROL_T__
{
    uint16_t rid;
    uds_routine_status rtst;
    uint8_t (* init_routine) (void);
    uint8_t (* run_routine) (void);
    uint8_t (* stop_routine) (void);
}uds_rtctrl_t;

#define RWDATA_CNT  11
#define IOCTRL_CNT  5
#define RTCTRL_CNT  1
extern const uds_rwdata_t rwdata_list[RWDATA_CNT];
extern uds_ioctrl_t ioctrl_list[IOCTRL_CNT];
extern const uds_rtctrl_t rtctrl_list[RTCTRL_CNT];


#define RWDATA_NUM  (sizeof(rwdata_list) / sizeof(uds_rwdata_t))
#define IOCTRL_NUM  (sizeof(ioctrl_list) / sizeof(uds_ioctrl_t))
#define RTCTRL_NUM  (sizeof(rtctrl_list) / sizeof(uds_rtctrl_t))



/*******************************************************************************
    Function  Definition
*******************************************************************************/
/**
 * uds_ioctrl_allstop - main handle of io control
 *
 * @void : 
 *
 * returns:
 *     void
 */
void
uds_ioctrl_allstop (void);

/**
 * uds_load_rwdata - load read / write data from eeprom to ram
 *
 * @void : 
 *
 * returns:
 *     void
 */
void
uds_load_rwdata (void);
#endif
/****************EOF****************/
