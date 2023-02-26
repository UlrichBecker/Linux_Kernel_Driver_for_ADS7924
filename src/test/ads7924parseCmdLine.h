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
// Name:        ads7924parseCmdLine.h
// Purpose:     Commandline-parser for ADS7924-Test
// Author:      Ulrich Becker
// Modified by:
// Created:     2017.05.29
// Copyright:   www.INKATRON.de
///////////////////////////////////////////////////////////////////////////////
#ifndef _ADS7924PARSECMDLINE_H_
#define _ADS7924PARSECMDLINE_H_
#include <parse_opts.h>
#include "ads7924test.h"

void printMenu( void );
int parseCmdLine( int argc, char** ppArgv );

#endif /* _ADS7924PARSECMDLINE_H_ */
/*================================== EOF ====================================*/
