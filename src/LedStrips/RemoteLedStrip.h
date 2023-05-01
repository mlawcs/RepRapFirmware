/*
 * RemoteLedStrip.h
 *
 *  Created on: 30 Apr 2023
 *      Author: David
 */

#ifndef SRC_LEDSTRIPS_REMOTELEDSTRIP_H_
#define SRC_LEDSTRIPS_REMOTELEDSTRIP_H_

#include "LedStripBase.h"

#if SUPPORT_LED_STRIPS

class RemoteLedStrip : public LedStripBase
{
public:
	RemoteLedStrip();
};

#endif

#endif /* SRC_LEDSTRIPS_REMOTELEDSTRIP_H_ */
