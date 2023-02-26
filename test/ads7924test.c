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
// Name:        ads7924test.c
// Purpose:     Interrupt-test ADS7924-device driver adc.ko
// Author:      Ulrich Becker
// Modified by:
// Created:     2017.05.16
// Copyright:   www.INKATRON.de
///////////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/timerfd.h>
#include <assert.h>

#include "ads7924parseCmdLine.h"
#include "ads7924test.h"


#define PUSH_RELEASE_POS 3

#define RELEASE_TIME_OUT 100


#define CHANNEL_INITIALIZER( n ) \
   [n] =                         \
   {                             \
      .filename = ADC##n,        \
      .channel = n,              \
      .fd  = INVALID_HANDLE,     \
      .fdt = INVALID_HANDLE,     \
      .ulr = DEFAULT_ULR##n,     \
      .llr = DEFAULT_LLR##n,     \
      .min = ((ANALOG_T)~0),     \
      .max = 0,                  \
      .last = 0,                 \
      .printRequest = false,     \
      .wasPrinted   = false,     \
      .isFirstReadung = true,    \
      .triggerCount = 0,         \
      .pressCount = 0,           \
      .state = ST_RELEASED,      \
      .line = n + 1              \
   }

ADC_CHANNEL_T g_cannelList[2] =
{
   CHANNEL_INITIALIZER( 0 ),
   CHANNEL_INITIALIZER( 1 )
};

GLOBAL_T global =
{
   .isSuspended      = false,
   .thresholdTrigger = true,
   .verbose          = false,
   .streamingMode    = false,
   .singleLine       = false,
   .pushReleaseEvent = false,
   .printDifference  = false,
   .difference       = DEFAULT_DIFFERENCE,
   .maxSample        = 1
};

int g_adcFd = INVALID_HANDLE;

static struct termios g_originTerminal;

/*-----------------------------------------------------------------------------
*/
static inline int resetTerminalInput( void )
{
   return tcsetattr( STDIN_FILENO, TCSANOW, &g_originTerminal );
}

/*-----------------------------------------------------------------------------
*/
static int prepareTerminalInput( void )
{
   struct termios newTerminal;

   if( tcgetattr( STDIN_FILENO, &g_originTerminal ) < 0 )
      return -1;

   newTerminal = g_originTerminal;
   newTerminal.c_lflag     &= ~(ICANON | ECHO);  /* Disable canonic mode and echo.*/
   newTerminal.c_cc[VMIN]  = 1;  /* Reading is complete after one byte only. */
   newTerminal.c_cc[VTIME] = 0;  /* No timer. */

   return tcsetattr( STDIN_FILENO, TCSANOW, &newTerminal );
}

/*-----------------------------------------------------------------------------
*/
void gotoxy( int x, int y )
{
   printf( "\033[%d;%dH", y, x );
   fflush( stdout );
}

/*-----------------------------------------------------------------------------
*/
void clrscr( void )
{
   printf( "\033[H\033[J" );
   fflush( stdout );
}

/*-----------------------------------------------------------------------------
*/
void hideCursor( void )
{
   printf( "\033[?25l" );
   fflush( stdout );
}

/*-----------------------------------------------------------------------------
*/
void showCursor( void )
{
   printf( "\033[?25h" );
   fflush( stdout );
}

/*-----------------------------------------------------------------------------
*/
int setInterruptConfig( int val )
{
   assert( val >= 0 );
   assert( val <= 7 );

   if( ioctl( g_adcFd, ADS7924_IOCTL_SET_INTCONFIG, val << 5 ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could send SET_INTCONFIG to \""ADC"\" : %s\r\n",
               strerror( errno ) );
      return -1;
   }
   return 0;
}

/*-----------------------------------------------------------------------------
*/
int setUlr( ADC_CHANNEL_T* pChannel )
{
   if( ioctl( pChannel->fd, ADS7924_IOCTL_SET_ULR, pChannel->ulr ) < 0 )
   {
      fprintf( stderr,
              "ERROR: Could not send SET_ULR 0x%02X to %s: %s\r\n",
               pChannel->ulr,
               pChannel->filename,
               strerror( errno ) );
      return -1;
   }
   return 0;
}

/*-----------------------------------------------------------------------------
*/
int setLlr( ADC_CHANNEL_T* pChannel )
{
   if( ioctl( pChannel->fd, ADS7924_IOCTL_SET_LLR, pChannel->llr ) < 0 )
   {
      fprintf( stderr,
              "ERROR: Could not send SET_LLR 0x%02X to %s: %s\r\n",
               pChannel->llr,
               pChannel->filename,
               strerror( errno ) );
      return -1;
   }
   return 0;
}

/*-----------------------------------------------------------------------------
*/
int setReadMode( ADC_CHANNEL_T* pChannel, unsigned int cmd )
{
   if( ioctl( pChannel->fd, cmd, 0 ) < 0 )
   {
      fprintf( stderr,
              "ERROR: CoulhideCursord not send read-mode 0x%08X to %s: %s\r\n",
               cmd,
               pChannel->filename,
               strerror( errno ) );
      return -1;
   }
}

/*-----------------------------------------------------------------------------
*/
int enableAlarm( ADC_CHANNEL_T* pChannel )
{
   if( ioctl( pChannel->fd, ADS7924_IOCTL_ALARM_ENABLE, 0 ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could not send ALARM_ENABLE: to %s: %s\r\n",
               pChannel->filename,
               strerror( errno ) );
      return -1;
   }
   return 0;
}

/*-----------------------------------------------------------------------------
*/
int _prepareAdc( void )
{
   int i;
   g_adcFd = open( ADC, O_RDONLY );
   if( g_adcFd < 0 )
   {
      fprintf( stderr,
               "ERROR: Could not open \""ADC"\" : %s\r\n",
                strerror( errno ) );
      return -1;
   }

   for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
   {
      g_cannelList[i].fd = open( g_cannelList[i].filename, O_RDONLY );
      if( g_cannelList[i].fd < 0 )
      {
         fprintf( stderr, "ERROR: Could not open \"%s\" : %s\r\n",
                  g_cannelList[i].filename, strerror( errno ) );
         return -1;
      }
      g_cannelList[i].fdt = timerfd_create( CLOCK_REALTIME, 0 );
      if( g_cannelList[i].fdt < 0 )
      {
         fprintf( stderr, "ERROR: Could not open timer-fd for \"%s\" : %s\r\n",
                  g_cannelList[i].filename, strerror( errno ) );
         return -1;
      }
   }

   if( ioctl( g_adcFd, ADS7924_IOCTL_RESET, 0 ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could send RESET to \""ADC"\" : %s\r\n",
               strerror( errno ) );
      return -1;
   }
   usleep( 250000 );

   if( ioctl( g_adcFd, ADS7924_IOCTL_SET_MODE, ADS7924_MODE_AWAKE ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could send SET_MODE MODE_AWAKE to \""ADC"\" : %s\r\n",
               strerror( errno ) );
      return -1;
   }

   if( ioctl( g_adcFd, ADS7924_IOCTL_SET_MODE, ADS7924_MODE_AUTO_SCAN_SLEEP ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could send SEglobalT_MODE MODE_AUTO_SCAN_SLEEP to \""ADC"\" : %s\r\n",
               strerror( errno ) );
      return -1;
   }

   if( ioctl( g_adcFd, ADS7924_IOCTL_SET_SLPCONFIG, 0x03 ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Could send SET_SLPCONFIG to \""ADC"\" : %s\r\n",
               strerror( errno ) );
      return -1;
   }

   if( setInterruptConfig( global.thresholdTrigger? global.maxSample : 0 ) < 0 )
      return -1;

   for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
   {
      if( setReadMode( &g_cannelList[i], ADS7924_IOCTL_READMODE_BIN ) < 0 )
         return -1;
      if( setUlr( &g_cannelList[i] ) < 0 )
         return -1;
      if( setLlr( &g_cannelList[i] ) < 0 )
         return -1;
      if( enableAlarm( &g_cannelList[i] ) < 0 )
         return -1;
   }
}

/*-----------------------------------------------------------------------------
*/
int prepareAdc( void )
{
   int status;
   int i;

   status = _prepareAdc();
   if( status < 0 )
   {
      for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
      {
         if( g_cannelList[i].fd > 0 )
         {
            close( g_cannelList[i].fd );
            g_cannelList[i].fd = INVALID_HANDLE;
         }
         if( g_cannelList[i].fdt > 0 )
         {
            close( g_cannelList[i].fdt );
            g_cannelList[i].fdt = INVALID_HANDLE;
         }
      }
      if( g_adcFd < 0 )
      {
         close( g_adcFd );
         g_adcFd = INVALID_HANDLE;
      }
   }
   return status;
}

/*-----------------------------------------------------------------------------
*/
int startTimer( int fd, unsigned long milisec )
{
   struct itimerspec time;

   time.it_value.tv_sec  = milisec / 1000;
   time.it_value.tv_nsec = (milisec % 1000) * 1000000;
   //time.it_interval = time.it_value;
   time.it_interval.tv_sec  = 0;
   time.it_interval.tv_nsec = 0;

   if( timerfd_settime( fd, 0, &time, NULL ) < 0 )
   {
      fprintf( stderr,
               "ERROR: Unable to start timer-filediscriptor: %s\n",
               strerror( errno ) );
      return -1;
   }
   return 0;
}


/*-----------------------------------------------------------------------------
*/
void readChannel( ADC_CHANNEL_T* pChannel )
{
   int ret;
   ANALOG_T analog = 0;
   bool canPrint;

   ret = read( pChannel->fd, &analog, sizeof( analog ) );
   if( ret > 0 )
   {
      if( analog < pChannel->min )
         pChannel->min = analog;
      if( analog > pChannel->max )
         pChannel->max = analog;

      canPrint = pChannel->printRequest;
      if( pChannel->isFirstReadung )
      {
         pChannel->isFirstReadung = false;
         canPrint = true;
      }
      else if( global.printDifference )
      {
         if( abs( ((int)pChannel->last) - ((int)analog) ) >= global.difference )
            canPrint = true;
      }
      else
         canPrint = true;

      if( canPrint )
      {

         if( !global.streamingMode )
            gotoxy( 1, pChannel->line );

         if( global.verbose )
         {
            printf( "Value of \"%s\": 0x%03X; %04d; min: %04d; max: %04d; count: %d",
                    pChannel->filename,
                    analog,
                    analog,
                    pChannel->min,
                    pChannel->max,
                    pChannel->triggerCount );
         }
         else
         {
            printf( "C%d; %04d; %04d; %04d; %d",
                    pChannel->channel,
                    analog,
                    pChannel->min,
                    pChannel->max,
                    pChannel->triggerCount );
         }

         if( global.streamingMode )
         {
            if( global.singleLine &&
               !global.thresholdTrigger &&
               !global.printDifference &&
               (pChannel->channel == 0)
              )
               printf( ";  " );
            else
               printf( "\r\n" );
         }
         else
            printf( "    " );

         fflush( stdout );
         pChannel->wasPrinted = true;
         pChannel->printRequest = false;
      }
      pChannel->last = analog;
   }
   else if( ret < 0 )
   {
      fprintf( stderr,
      "ERROR: Could read from \"%s\" : %s\n",
      pChannel->filename,
      strerror( errno ) );
   }
}


/*-----------------------------------------------------------------------------
 */
int main( int argc, char** ppArgv )
{
   fd_set rfds;
   int i;
   int inKey = 0;
   int state;
   int maxFd;
   uint64_t timerValue;
   bool wasPrinted;

   if( parseCmdLine( argc, ppArgv ) < 0 )
      return EXIT_FAILURE;

   if( prepareAdc() < 0 )
      return EXIT_FAILURE;

   if( !global.streamingMode )
   {
      clrscr();
      hideCursor();
   }
   if( global.verbose )
      printMenu();

   maxFd = STDIN_FILENO;
   for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
   {
      if( maxFd < g_cannelList[i].fd )
         maxFd = g_cannelList[i].fd;
      if( maxFd < g_cannelList[i].fdt )
         maxFd = g_cannelList[i].fdt;
   }
   maxFd++;

   prepareTerminalInput();
   do
   {
      FD_ZERO( &rfds );
      FD_SET( STDIN_FILENO, &rfds );
      if( !global.isSuspended )
      {
         for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
         {
            FD_SET( g_cannelList[i].fd, &rfds );
            g_cannelList[i].wasPrinted = false;
            if( global.thresholdTrigger && (g_cannelList[i].state == ST_PRESSED) )
               FD_SET( g_cannelList[i].fdt, &rfds );
         }
      }
      wasPrinted = false;
      state = select( maxFd, &rfds, NULL, NULL, NULL );
      if( state > 0 )
      {
         if( FD_ISSET( STDIN_FILENO, &rfds ) && (read( STDIN_FILENO, &inKey, sizeof( inKey ) ) > 0) )
         {
            inKey &= 0xFF;
            switch( inKey )
            {
               case 'r':
               {
                  for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
                  {
                     g_cannelList[i].isFirstReadung = true;
                     g_cannelList[i].triggerCount = 0;
                     g_cannelList[i].pressCount = 0;
                     g_cannelList[i].min = ((ANALOG_T)~0);
                     g_cannelList[i].max = 0;
                  }
                  break;
               }
               case '1':
               {
                  g_cannelList[0].printRequest = true;
                  readChannel( &g_cannelList[0] );
                  break;
               }
               case '2':
               {
                  g_cannelList[1].printRequest = true;
                  readChannel( &g_cannelList[1] );
                  break;
               }
               case 'a':
               {
                  for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
                  {
                     g_cannelList[i].printRequest = true;
                     readChannel( &g_cannelList[i] );
                  }
                  break;
               }
               case 's':
               {
                  if( !global.streamingMode )
                     gotoxy( 1, 8 );
                  if( global.isSuspended )
                  {
                     global.isSuspended = false;
                     if( global.verbose && !global.streamingMode )
                        printf( "          " );
                  }
                  else
                  {
                     global.isSuspended = true;
                     for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
                        FD_CLR( g_cannelList[i].fd, &rfds );
                     if( global.verbose )
                     {
                        printf( "Suspended!" );
                        if( global.streamingMode )
                           printf( "\r\n" );
                     }
                  }
                  fflush( stdout );
                  break;
               }
               case 't':
               {
                  if( !global.streamingMode )
                     gotoxy( 1, 9 );
                  if( global.thresholdTrigger )
                  {
                     if( setInterruptConfig( 0 ) >= 0 )
                     {
                        if( global.verbose )
                        {
                           printf( "Continued trigger" );
                           if( global.streamingMode )
                              printf( "\r\n" );
                        }
                        global.thresholdTrigger = false;
                        for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
                           FD_CLR( g_cannelList[i].fdt, &rfds );
                     }
                  }
                  else
                  {
                     if( setInterruptConfig( global.maxSample ) >= 0 )
                     {
                        if( global.verbose )
                        {
                           printf( "Threshold trigger" );
                           if( global.streamingMode )
                              printf( "\r\n" );
                        }
                        global.thresholdTrigger = true;
                     }
                  }
                  fflush( stdout );
                  break;
               }
            }
         }
         for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
         {
            if( !FD_ISSET( g_cannelList[i].fd, &rfds ) )
               continue;
            g_cannelList[i].triggerCount++;
            readChannel( &g_cannelList[i] );
            startTimer( g_cannelList[i].fdt, RELEASE_TIME_OUT * global.maxSample );
            switch( g_cannelList[i].state )
            {
               case ST_RELEASED:
               {
                  g_cannelList[i].pressCount++;
                  if( global.pushReleaseEvent )
                  {
                     if( !global.streamingMode )
                        gotoxy( 1, g_cannelList[i].line + PUSH_RELEASE_POS );
                     printf( "\"%s\": Pressed  %d     ",
                             g_cannelList[i].filename,
                             g_cannelList[i].pressCount );
                     if( global.streamingMode )
                        printf( "\r\n" );
                     fflush( stdout );
                  }
                  g_cannelList[i].state = ST_PRESSED;
                  break;
               }
               default: break;
            }
         }

         for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
         {
            if( !FD_ISSET( g_cannelList[i].fdt, &rfds ) )
               continue;
            if( read( g_cannelList[i].fdt, &timerValue, sizeof( timerValue ) ) < 0 )
            {
               fprintf( stderr,
                        "ERROR: Unable to timer of \"%s\": %s\r\n",
                        g_cannelList[i].filename,
                        strerror( errno ) );
               continue;
            }
            switch( g_cannelList[i].state )
            {
               case ST_PRESSED:
               {
                  if( global.pushReleaseEvent )
                  {
                     if( !global.streamingMode )
                        gotoxy( 1, g_cannelList[i].line + PUSH_RELEASE_POS );
                     printf( "\"%s\": Released %d     ",
                             g_cannelList[i].filename,
                             g_cannelList[i].pressCount
                           );
                     if( global.streamingMode )
                        printf( "\r\n" );
                     fflush( stdout );
                  }
                  g_cannelList[i].state = ST_RELEASED;
                  break;
               }
               default: break;
            }
         }
      }
   }
   while( inKey != ESC );

   resetTerminalInput();
   for( i = 0; i < ARRAY_SIZE( g_cannelList ); i++ )
   {
      close( g_cannelList[i].fd );
      close( g_cannelList[i].fdt );
   }
   close( g_adcFd );

   if( !global.streamingMode )
      clrscr();
   if( global.verbose )
      printf( "End..." );
   if( !global.streamingMode )
      showCursor();
   printf( "\r\n" );

   return EXIT_SUCCESS;
}

/*================================== EOF ====================================*/
