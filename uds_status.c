/***************************************************************************//**
    \file          uds-status.c
    \author        
    \mail         
    \version       0.00
    \date          2016-10-10
    \description   uds status code, include session and Security access
*******************************************************************************/

/*******************************************************************************
    Include Files
*******************************************************************************/
#include "uds_status.h"
#include "uds_util.h"

/*******************************************************************************
    Function  Definition
*******************************************************************************/


/**
 * seedTOKey - the algorithms to compute access key
 *
 * @seed: the seed
 *
 * returns:
 *     the key
 */
static uint32_t
seedTOKey( uint32_t seed )
{
	uint8_t i;
	uint32_t key;
	key = UNLOCKKEY;
	if(!(( seed == UNLOCKSEED )
	||( seed == UNDEFINESEED )))
	{
		for( i = 0; i < 35; i++ )
		{
			if( seed & SEEDMASK )
			{
				seed = seed << SHIFTBIT;
				seed = seed ^ ALGORITHMASK;
			}
			else
			{
				seed = seed << SHIFTBIT;
			}
		}
		key = seed;
	}
	return key;
}

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
uds_security_access (uint8_t key_buf[], uint8_t seed_buf[])
{
	uint32_t key;
	uint32_t seed;
    can_to_hostl (key_buf, &key);
	can_to_hostl (seed_buf, &seed);

	if (key == seedTOKey(seed))
	    return 0;
	else
	    return -1;
}

/****************EOF****************/
