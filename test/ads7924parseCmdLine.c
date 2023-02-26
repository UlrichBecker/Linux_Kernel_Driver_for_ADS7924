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
// Name:        ads7924parseCmdLine.c
// Purpose:     Commandline-parser for ADS7924-Test
// Author:      Ulrich Becker
// Modified by:
// Created:     2017.05.29
// Copyright:   www.INKATRON.de
///////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "ads7924parseCmdLine.h"

/*
 * In this program we prefer for line feeds "\r\n" because of possible stupid 
 * MS-Windows terminals rater than "\n". ;-)
 */

/*!----------------------------------------------------------------------------
 */
void printMenu( void )
{
   printf( "Key 'Esc': End program\r\n" );
   printf( "Key 's':   Suspend on/off\r\n" );
   printf( "Key 't':   Switch between threshold and continued triggering\r\n" );
   printf( "Key 'r':   Reset trigger-counter and min, max\r\n" );
   printf( "Key '1':   Read \""ADC0"\" direct\r\n" );
   printf( "Key '2':   Read \""ADC1"\" direct\r\n" );
   printf( "Key 'a':   Read \""ADC0"\" and \""ADC1"\" direct\r\n" );
}

/*!----------------------------------------------------------------------------
 */
static int onOptHelp( struct BLOCK_FUNCTION_ARG_T* pArg )
{
   printf( "Kernel driver testprogram for Analog to Digital Converter ADS792\r\n" );
   printf( "Author: Ulrich Becker\r\n\n" );
   printf( "Usage: %s [options]\r\n\n", pArg->ppAgv[0] );
   printMenu();
   printf( "\nOptions:\r\n" );
   printOptionList( stdout, pArg->pOptBlockList );
   printf( "\n\rOutput-example:\n\r" );
   printf( "\"C0; 3187; 2519; 3190; 2737;  C1; 3183; 3168; 3189; 2737\"\r\n" );
   printf( "  |    |     |     |     |     |    |     |     |     |\r\n" );
   printf( "  |    |     |     |     |     |    |     |     |     +- Interrupt count of channel 1\r\n" );
   printf( "  |    |     |     |     |     |    |     |     +------- Maximum value of channel 1\r\n" );
   printf( "  |    |     |     |     |     |    |     +------------- Minimum value of channel 1\r\n" );
   printf( "  |    |     |     |     |     |    +------------------- Analog value of channel 1\r\n" );
   printf( "  |    |     |     |     |     +------------------------ Analog input channel 1\r\n" );
   printf( "  |    |     |     |     |\r\n" );
   printf( "  |    |     |     |     +------------------------------ Interrupt count of channel 0\r\n" );
   printf( "  |    |     |     +------------------------------------ Maximum value of channel 0\r\n" );
   printf( "  |    |     +------------------------------------------ Minimum value of channel 0\r\n" );
   printf( "  |    +------------------------------------------------ Analog value of channel 0\r\n" );
   printf( "  +----------------------------------------------------- Analog input channel 0\r\n\n" );
   printf( "Interrupt counter, minimum and maximum values can be reseted\r\n" );
   printf( "during the runtime by pushing the key \"r\"\r\n" );
   exit( EXIT_SUCCESS );
   return 0;
}

/*!----------------------------------------------------------------------------
*/
static int readNumber( long* pNum, const char* str )
{
   char* pEnd;
   size_t l = strlen( str );

   if( l >= 2 && str[0] == '0' && str[1] == 'x' )
      *pNum = strtol( &str[2], &pEnd, 16 );
   else
      *pNum = strtol( str, &pEnd, 10 );

   return (*pEnd == '\0')? l : -1;
}

/*!----------------------------------------------------------------------------
*/
static int onOptDifference( struct BLOCK_FUNCTION_ARG_T* pArg )
{
   long diff;

   global.printDifference = true;
   if( pArg->optArg == NULL )
      return 0;

   if( readNumber( &diff, pArg->optArg ) < 0 )
   {
      fprintf( stderr, "ERROR: Unable to interpret value of option: \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" Value: %s\r\n", pArg->optArg );
      return -1;
   }
   if( (diff < 0) || (diff > 0xFF) )
   {
      fprintf( stderr, "ERROR: Value of option \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" out of range: %d\r\n", diff );
      fprintf( stderr, "Allowed values: max: %d, min: %d\r\n",
               0xFF, 0 );
      return -1;
   }
   global.difference = diff;
   return 0;
}

/*!----------------------------------------------------------------------------
*/
#define MIN_EXCEEDING 1
#define MAX_EXCEEDING 7
static int onOptSetThresholdCount( struct BLOCK_FUNCTION_ARG_T* pArg )
{
   long n;

   if( readNumber( &n, pArg->optArg ) < 0 )
   {
      fprintf( stderr, "ERROR: Unable to interpret value of option: \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" Value: %s\r\n", pArg->optArg );
      return -1;
   }
   if( (n < MIN_EXCEEDING) || (n > MAX_EXCEEDING) )
   {
      fprintf( stderr, "ERROR: Value of option \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" out of range: %d\r\n", n );
      fprintf( stderr, "Allowed values: max: %d, min: %d\r\n",
               MAX_EXCEEDING, MIN_EXCEEDING );
      return -1;
   }
   global.maxSample = n;
   return 0;
}

/*!----------------------------------------------------------------------------
*/
#define THRESHOLD_MAX 0xFF
#define THRESHOLD_MIN 0
static int onSetAlarmThreshold( long* pValue, struct BLOCK_FUNCTION_ARG_T* pArg, int channel )
{
   assert( channel >= 0 );
   assert( channel <= 1 );

   if( readNumber( pValue, pArg->optArg ) < 0 )
   {
      fprintf( stderr, "ERROR: Unable to interpret value of option: \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" Value: %s\r\n", pArg->optArg );
      return -1;
   }

   if( (*pValue < THRESHOLD_MIN) || (*pValue > THRESHOLD_MAX) )
   {
      fprintf( stderr, "ERROR: Value of option \"" );
      printOption( stderr, pArg->pCurrentBlock );
      fprintf( stderr, "\" out of range: %d\r\n", *pValue );
      fprintf( stderr, "Allowed values: max: %d, min: %d\r\n",
               THRESHOLD_MAX, THRESHOLD_MIN );
      return -1;
   }
}

/*!----------------------------------------------------------------------------
*/
static int onOptSetLLR( struct BLOCK_FUNCTION_ARG_T* pArg, int channel )
{

   long n;

   if( onSetAlarmThreshold( &n, pArg, channel ) < 0 )
      return -1;
   g_cannelList[channel].llr = n;
   return 0;
}

/*!----------------------------------------------------------------------------
*/
static int onOptSetULR( struct BLOCK_FUNCTION_ARG_T* pArg, int channel )
{

   long n;

   if( onSetAlarmThreshold( &n, pArg, channel ) < 0 )
      return -1;
   g_cannelList[channel].ulr = n;
   return 0;
}

/*!----------------------------------------------------------------------------
 */
int parseCmdLine( int argc, char** ppArgv )
{
   int i;
   struct OPTION_BLOCK_T optionObjectList[] =
   {
      {
         .optFunction = onOptHelp,
         .hasArg      = NO_ARG,
         .shortOpt    = 'h',
         .longOpt     = "help",
         .helpText    = "Print this help and exit"
      },
      {
         OPT_LAMBDA( pArg,
                     {
                        printf( VERSION"\r\n" );
                        exit( EXIT_SUCCESS );
                        return 0;
                     }),
         .hasArg     = NO_ARG,
         .shortOpt   = 'V',
         .longOpt    = "version",
         .helpText   = "Print program version and exit."
      },
      {
         OPT_LAMBDA( pArg, { global.verbose = true; return 0; }),
         .hasArg      = NO_ARG,
         .shortOpt    = 'v',
         .longOpt     = "verbose",
         .helpText    = "Be verbose"
      },
      {
         OPT_LAMBDA( pArg, { global.thresholdTrigger = false; return 0; }),
         .hasArg      = NO_ARG,
         .shortOpt    = 'c',
         .longOpt     = "continued",
         .helpText    = "Starts in the continued trigger mode.\r\n"
                        "The default mode is the threshold trigger mode.\r\n"
                        "This mode can also be changed during the runtime "
                        "by pushing the key \"t\"."
      },
      {
         OPT_LAMBDA( pArg, { global.isSuspended = true; return 0; }),
         .hasArg      = NO_ARG,
         .shortOpt    = 'u',
         .longOpt     = "suspended",
         .helpText    = "Starts in the suspended mode.\r\n"
                        "You can also switch between suspended and resumed during the runtime\r\n"
                        "by pushing the key \"s\"."
      }, 
      {
         .optFunction = onOptDifference,
         .hasArg      = OPTIONAL_ARG,
         .shortOpt    = 'd',
         .longOpt     = "difference",
         .helpText    = "Displays only when a difference exist between the\r\n"
                        "last value and the actual value.\r\n"
                        "Because of the noise it will only displayed if the\r\n"
                        "absolute difference greater or equal PARAM.\r\n"
                        "If PARAM not given a default value of 15 will be used.\r\n"
                        "Allowed range: 0 <= PARAM <= 255"
      },
      {
         OPT_LAMBDA( pArg, { global.streamingMode = true; return 0; }),
         .hasArg      = NO_ARG,
         .shortOpt    = 's',
         .longOpt     = "stream",
         .helpText    = "Enables the streaming mode "
                        "without terminal- escape- sequences."

      },
      {
         OPT_LAMBDA( pArg, { global.singleLine = true; return 0; }),
         .hasArg      = NO_ARG,
         .shortOpt    = 'S',
         .longOpt     = "singleline",
         .helpText    = "Displays the analog values in a single line,\r\n"
                        "if streaming mode and continued triggering enabled."
      },
      {
         .optFunction = onOptSetThresholdCount,
         .hasArg      = REQUIRED_ARG,
         .shortOpt    = 'e',
         .longOpt     = "threshold-exceeding",
         .helpText    = "Sets the maximum threshold exceeding of the\r\n"
                        "ADS7924 window comparator.\r\n"
                        "See also in specification ADS7924.pdf page 24.\r\n"
                        "If this option not used, this program will set this value to 1.\r\n"
                        "Allowed range: 1 <= PARAM <= 7"
      },
      {
         OPT_LAMBDA( pArg, { global.pushReleaseEvent = true; return 0; }),
         .hasArg     = NO_ARG,
         .shortOpt   = 'p',
         .longOpt    = "push-release",
         .helpText   = "Enables the displaying of push/release events,\r\n"
                       "depending on the values of the ADS7924 thresholt\r\n"
                       "registers LLR0 for channel 0 and LLR1 for channel 1."
      },
      {
         OPT_LAMBDA( pArg, { return onOptSetLLR( pArg, 0 ); }),
         .hasArg     = REQUIRED_ARG,
         .longOpt    = "LLR0",
         .helpText   = "Sets the LLR0 compare register of channel 0\r\n"
                       "See also in specification ADS7924.pdf page 23.\r\n"
                       "Allowed range: 0 <= PARAM <= 255"
      },
      {
         OPT_LAMBDA( pArg, { return onOptSetLLR( pArg, 1 ); }),
         .hasArg     = REQUIRED_ARG,
         .longOpt    = "LLR1",
         .helpText   = "Sets the LLR1 compare register of channel 1\r\n"
                       "See also in specification ADS7924.pdf page 23.\r\n"
                       "Allowed range: 0 <= PARAM <= 255"
      },
      {
         OPT_LAMBDA( pArg, { return onOptSetULR( pArg, 0 ); }),
         .hasArg     = REQUIRED_ARG,
         .longOpt    = "ULR0",
         .helpText   = "Sets the ULR0 compare register of channel 0\r\n"
                       "See also in specification ADS7924.pdf page 23.\r\n"
                       "Allowed range: 0 <= PARAM <= 255"
      },
      {
         OPT_LAMBDA( pArg, { return onOptSetULR( pArg, 1 ); }),
         .hasArg     = REQUIRED_ARG,
         .longOpt    = "ULR1",
         .helpText   = "Sets the ULR1 compare register of channel 0\r\n"
                       "See also in specification ADS7924.pdf page 23.\r\n"
                       "Allowed range: 0 <= PARAM <= 255"
      },
      OPTION_BLOCKLIST_END_MARKER
   };

   for( i = 1; i < argc; i++ )
   {
      i = parseCommandLineOptionsAt( i, argc, ppArgv, optionObjectList, NULL );
      if( i < 0 )
         return i;
      if( i < argc )
      {
      //TODO Evaluate possible non-option arguments here.
         fprintf( stderr, "ERROR: Unrecognized parameter: \"%s\"\n", ppArgv[i] );
         return -1;
      }
   }

   if( global.singleLine )
      global.singleLine = global.streamingMode;

   if( global.pushReleaseEvent )
      global.pushReleaseEvent = !global.singleLine;

   if( !global.verbose )
      return 0;

   for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
      g_cannelList[i].line += OUT_VERBOSE_POS;

   return 0;
}

/*================================== EOF ====================================*/
