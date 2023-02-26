/******************************************************************************
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library. If not, see <http://www.gnu.org/licenses/>.
 ******************************************************************************
 */
///////////////////////////////////////////////////////////////////////////////
// Name:        ads7924test.h
// Purpose:     Interrupt-test ADS7924-device driver adc.ko
// Author:      Ulrich Becker
// Modified by:
// Created:     2017.05.29
// Copyright:   www.INKATRON.de
///////////////////////////////////////////////////////////////////////////////
#ifndef _ADS7924TEST_H_
#define _ADS7924TEST_H_

#include <stdbool.h>
#include <ads7924ioctl.h>

#define ADC  "/dev/adc0A"
#define ADC0 "/dev/adc0A0"
#define ADC1 "/dev/adc0A1"

#define INVALID_HANDLE -1
#define ESC 0x1B

#ifndef ARRAY_SIZE
 #define ARRAY_SIZE( a ) (sizeof( a ) / sizeof( a[0] ))
#endif

#define DEFAULT_LLR0 0xC4
#define DEFAULT_LLR1 0xC3
#define DEFAULT_ULR0 0xFF
#define DEFAULT_ULR1 0xFF
#define DEFAULT_DIFFERENCE 0x00F

#define VERSION "1.2"

#define OUT_VERBOSE_POS 10

typedef enum
{
   ST_RELEASED,
   ST_PRESSED
} PRESS_T;

typedef unsigned int ANALOG_T;

typedef struct
{
   const char* filename;
   const int   channel;
   int         fd;
   int         fdt;
   uint8_t     ulr;
   uint8_t     llr;
   ANALOG_T    min;
   ANALOG_T    max;
   ANALOG_T    last;
   bool        printRequest;
   bool        wasPrinted;
   bool        isFirstReadung;
   int         triggerCount;
   int         pressCount;
   PRESS_T     state;
   int         line;
} ADC_CHANNEL_T;

extern ADC_CHANNEL_T g_cannelList[2];

typedef struct
{
   bool     isSuspended;
   bool     thresholdTrigger;
   bool     verbose;
   bool     streamingMode;
   bool     singleLine;
   bool     pushReleaseEvent;
   bool     printDifference;
   ANALOG_T difference;
   int      maxSample;
} GLOBAL_T;

extern GLOBAL_T global;

#endif /* ifndef _ADS7924TEST_H_ */
/*================================== EOF ====================================*/
