/***************************************************************************//**
    \file          obd-dtc-private.h
    \author        
    \mail          
    \version       0
    \date          2016-10-10
    \description
*******************************************************************************/
#ifndef	__OBD_DTC_PRIVATE_H_
#define	__OBD_DTC_PRIVATE_H_
/*******************************************************************************
    Include Files
*******************************************************************************/
#include <stdint.h>
#include "uds_type.h"

/*******************************************************************************
    Type Definition
*******************************************************************************/

#define DTCG_EMISSION_START   0x00000000ul /* 24Bit DTC define ISO-15031-6 DTC + ISO-15031-6 Failure Type, ISO14229-1 format */
#define DTCG_EMISSION_END     0x000FFFFFul
#define DTCG_POWERTRAIN_START 0x00100000ul
#define DTCG_POWERTRAIN_END   0x003FFFFFul
#define DTCG_CHASSIS_START    0x00400000ul
#define DTCG_CHASSIS_END      0x007FFFFFul
#define DTCG_BODY_START       0x00800000ul
#define DTCG_BODY_END         0x00BFFFFFul
#define DTCG_NETWORK_START    0x00C00000ul
#define DTCG_NETWORK_END      0x00FFFFFFul


typedef struct __OBD_DTC_PARA_T__
{
    uint32_t     dtc_code;

}obd_dtc_para_t;


typedef union __DTC_STATUS_T__
{
    uint8_t all;
    struct
    {
        uint8_t test_fail       : 1;
        uint8_t test_fail_toc   : 1;
        uint8_t pending         : 1;
        uint8_t confirmed       : 1;
        uint8_t test_ncmp_slc   : 1;
        uint8_t test_fail_slc   : 1;
        uint8_t test_ncmp_toc   : 1;
        uint8_t wn_indr         : 1;
    }bit;
}dtc_status_t;

/* Bit 0, testFailed, clear by test pass or ClearDiagnosticInformation */
/* Bit 1, testFailedThisOperationCycle, clear by new OC or ClearDiagnosticInformation */
/* Bit 2, latch, pendingDTC, clear by fault never happened in cur OC or ClearDiagnosticInformation, if test not completed, must hold. */
/* Bit 3, latch, confirmedDTC, clear by ClearDiagnosticInformation, one(only by normal fault) or 40 OC not detect any fault. */
/* Bit 4, latch, testNotCompletedSinceLastClear, set by a new OC start or ClearDiagnosticInformation */
/* Bit 5, latch, testFailedSinceLastClear, clear by ClearDiagnosticInformation */
/* Bit 6, testNotCompletedThisOperationCycle */
/* Bit 7, warningIndicatorRequested(unsupported) */

typedef struct __OBD_DTC_DATA_T__
{
#ifdef SNAPSHOT
    bool_t       has_ff;
#endif

    dtc_status_t dtc_st;
    uint8_t      fec_cnt;
    int16_t      fdt_cnt;        /* FaultDetectionCount */
    uint8_t      agn_cnt;        /* DTCAgingCounter */
}obd_dtc_data_t;

#define FDT_MAX               (127)
#define FDT_MIN               (-128)
#define AGN_MAX               40
#endif
/****************EOF****************/
