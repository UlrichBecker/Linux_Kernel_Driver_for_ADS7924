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
/*!
 * @file ads7924Irq.h
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @brief Handling of the GPIO-interrupt for possible alarms from ADS7924
 * @date 2017.05.12
 * @see ads7924Irq.c
 */
#ifndef _ADS7924IRQ_H
#define _ADS7924IRQ_H

#include "ads7924driver.h"

#ifdef _ADS7924_NO_DEV_TREE
extern int initGpioInterrupt( void ) _ADS7924_INIT;
#else
extern int initGpioInterrupt( ADS7924_T* pAds7924 );
#endif

#endif /* ifndef _ADS7924IRQ_H */
/*================================== EOF ====================================*/
