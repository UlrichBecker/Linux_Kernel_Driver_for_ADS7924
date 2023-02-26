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
 * @file ads7924procFs.c
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.05.10
 * @brief Representation of the ADS7924 driver status in the
 *                       process file-system.
 * @see ads7924procFs.h
 */
#if defined( CONFIG_PROC_FS ) || defined(__DOXYGEN__)

#include "ads7924driver.h"
#include "ads7924core.h"
#include "ads7924fileIo.h"
#include "ads7924procFs.h"
#include "ads7924_dev_tree_names.h"

#if defined(__DOXYGEN__) && !defined( CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS )
 #define CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS
#endif

#define FORMAT_CASE_ITEM( n ) case n: return #n
/*!----------------------------------------------------------------------------
 * @brief Helper-function for procOnOpen, makes the current read-mode
 *        human readable.
 * @see procOnOpen
 */
static const char* getOutputFormatName( OUTPUT_FORMAT_T value )
{
   switch( value )
   {
      FORMAT_CASE_ITEM( OUT_BIN );
      FORMAT_CASE_ITEM( OUT_DEC );
      FORMAT_CASE_ITEM( OUT_HEX );
      default: BUG_ON( true );
   }
   return "not defined!";
};

typedef struct
{
   char* name;
   u8    value;
} MODE_LIST_ITEM_T;

#define MODE_LIST_ITEM( m ) { .name = #m, .value = m }

static const MODE_LIST_ITEM_T mg_modeList[] =
{
   MODE_LIST_ITEM( ADS7924_MODE_IDLE ),
   MODE_LIST_ITEM( ADS7924_MODE_AWAKE ),
   MODE_LIST_ITEM( ADS7924_MODE_MANUAL_SINGLE ),
   MODE_LIST_ITEM( ADS7924_MODE_MANUAL_SCAN ),
   MODE_LIST_ITEM( ADS7924_MODE_AUTO_SINGLE ),
   MODE_LIST_ITEM( ADS7924_MODE_AUTO_SCAN ),
   MODE_LIST_ITEM( ADS7924_MODE_AUTO_SINGLE_SLEEP ),
   MODE_LIST_ITEM( ADS7924_MODE_AUTO_SCAN_SLEEP ),
   MODE_LIST_ITEM( ADS7924_MODE_AUTO_BURST_SCAN_SLEEP ),
   { .name = NULL, .value = 0 }
};

/*!----------------------------------------------------------------------------
 * @brief Helper function for procOnOpen
 * converts a 1 byte binary value in a human readable ASCII-String
 * of '0' and '1'.
 * @see procOnOpen
 */
static const char* toBin( char* buffer, const u8 value )
{
   u8 m;
   char* p = buffer;

   for( m = 0x80; m != 0; m >>= 1, p++ )
      *p = ((m & value) != 0)? '1':'0';

   *p = '\0';
   return buffer;
}

#ifdef CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS
/*!----------------------------------------------------------------------------
 * @brief Helper function for procOnOpen shows the hexadecimal numbers of
 * the macros for the user function ioctl() defined in ads7924ioctl.h.
 * 
 * @see ads7924ioctl.h
 * @see procOnOpen
 */
static inline void showIoctlCmds( struct seq_file* pSeqFile )
{
   const IOC_CHIP_INFO_T*    pCurrentChipItem;
   const IOC_CHANNEL_INFO_T* pCurrentChannelItem;
   int i;

   seq_printf( pSeqFile, "Possible modes:\n" );
   for( i = 0; mg_modeList[i].name != NULL; i++ )
   {
      seq_printf( pSeqFile, " 0x%02X: %s\n",
                  mg_modeList[i].value, mg_modeList[i].name );
   }

   seq_printf( pSeqFile, 
               "\nValid commands for ioctl() for entire chip access:\n" );
   for( pCurrentChipItem = mg_fTabIoctrlChip; 
       pCurrentChipItem->function != NULL; pCurrentChipItem++ )
   {
      seq_printf( pSeqFile, " 0x%08X:\t%s\n",
                  pCurrentChipItem->number,
                  pCurrentChipItem->name );
   }

   seq_printf( pSeqFile, 
               "\nValid commands for ioctl() for single channel access:\n" );
   for( pCurrentChannelItem = mg_fTabIoctrlChannel;
       pCurrentChannelItem->function != NULL; pCurrentChannelItem++ )
   {
      seq_printf( pSeqFile, " 0x%08X:\t%s\n",
                  pCurrentChannelItem->number,
                  pCurrentChannelItem->name );
   }
}
#endif /* ifdef CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS */

#if defined( _ADS7924_NO_DEV_TREE ) || defined(__DOXYGEN__)
/*!----------------------------------------------------------------------------
 * @brief Helper-function for displaying the status of the GPIO-interrupt input.
 */
static inline const char* getGpioStatus( void )
{
   int ret;

   if( g_data.adcInterrupt.gpioPin == 0 )
      return "undefined";
   ret = gpio_get_value( g_data.adcInterrupt.gpioPin );
   if( ret == 0 )
      return "low";
   if( ret > 0 )
      return "high";
   return "error";
}
#endif /* ifdef _ADS7924_NO_DEV_TREE */

#define __VERSION TS( VERSION )
/*!-----------------------------------------------------------------------------
 * @brief Displays the current driver status via process-file-system.
 *
 * E.g.:
 * @code
 * cat /proc/driver/adc
 * @endcode
 */
static int procOnOpen( struct seq_file* pSeqFile, void* pValue )
{
   BUS_T* pI2cBus;
   int chipIndex;
   int channelIndex;
   u8 adcRegister;
   char binAsciiBuffer[10];

   seq_printf( pSeqFile, KBUILD_MODNAME " Version: " __VERSION "\n" );

#ifdef CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS
   showIoctlCmds( pSeqFile );
#endif

#ifdef _ADS7924_NO_DEV_TREE
   seq_printf( pSeqFile, "\nInterrupt-GPIO-pin: %d -> %s\n",
               g_data.adcInterrupt.gpioPin,
               getGpioStatus() );
   seq_printf( pSeqFile, "Interrupt-number:   %d\n", g_data.adcInterrupt.irq );
#endif

   FOR_EACH_I2C_BUS( pI2cBus )
   {
      seq_printf( pSeqFile, "\nI2C-bus number: %d\n", pI2cBus->pI2cAdapter->nr );
      for( chipIndex = 0; chipIndex < ADC_CHIPS_PER_BUS; chipIndex++ )
      {
         if( pI2cBus->paChip[chipIndex] == NULL )
            continue;
         if( pI2cBus->paChip[chipIndex]->pI2cSlave == NULL )
         {
            seq_printf( pSeqFile, "No I2C-port for %s%d%c\n",
                        g_data.pName,
                        pI2cBus->pI2cAdapter->nr,
                        'A' + chipIndex );
            continue;
         }
         seq_printf( pSeqFile, "\t%s%d%c:\n\t\tI2C-Address: 0x%02X\n",
                     g_data.pName,
                     pI2cBus->pI2cAdapter->nr,
                     'A' + chipIndex,
                     pI2cBus->paChip[chipIndex]->pI2cSlave->addr
                   );
#ifndef _ADS7924_NO_DEV_TREE
         seq_printf( pSeqFile, "\t\tInterrupt-number: %d\n",
                               pI2cBus->paChip[chipIndex]->pI2cSlave->irq );
#endif
         seq_printf( pSeqFile, "\t\tOpen-count: %d\n",
                     atomic_read( &pI2cBus->paChip[chipIndex]->openCounter ));

         if( adcReadModeByte( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile, "\t\tCouldn't read mode on address 0x%02X!\n", MODECNTRL );
         }
         else
         {
            adcRegister &= ADS7924_MODE_AUTO_BURST_SCAN_SLEEP;
            seq_printf( pSeqFile, "\t\tMODECNTRL: 0x%02X, %s\n",
                        adcRegister, getModeName( adcRegister ) );
         }

         if( adcReadIntCtrl( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile,
                        "\t\tCouldn't read interrupt control register on address 0x%02X!\n",
                        INTCNTRL );
         }
         else
         {
            seq_printf( pSeqFile, "\t\tINTCNTRL:  0x%02X, %s\n",
                        adcRegister,
                        toBin( binAsciiBuffer, adcRegister )
                      );
         }

         if( adcReadIntConfig( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile,
                        "\t\tCouldn't read interrupt configuration register on address 0x%02X!\n",
                        INTCONFIG );
         }
         else
         {
            seq_printf( pSeqFile, "\t\tINTCONFIG: 0x%02X, %s\n",
                        adcRegister,
                        toBin( binAsciiBuffer, adcRegister ) );
         }

         if( adcReadSlpConfig( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile,
                        "\t\tCouldn't read sleep configuration register on address 0x%02X!\n",
                        SLPCONFIG );
         }
         else
         {
            seq_printf( pSeqFile, "\t\tSLPCONFIG: 0x%02X, %s\n",
                        adcRegister,
                        toBin( binAsciiBuffer, adcRegister ) );
         }

         if( adcReadAcqConfig( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile,
                        "\t\tCouldn't read aquire configuration register on address 0x%02X!\n",
                        ACQCONFIG );

         }
         else
         {
            seq_printf( pSeqFile, "\t\tACQCONFIG: 0x%02X, %s\n",
                        adcRegister,
                        toBin( binAsciiBuffer, adcRegister ));
         }

         if( adcReadPwrConfig( pI2cBus->paChip[chipIndex], &adcRegister ) < 0 )
         {
            seq_printf( pSeqFile,
                        "\t\tCouldn't read power configuration register on address 0x%02X!\n",
                        PWRCONFIG );
         }
         else
         {
            seq_printf( pSeqFile, "\t\tPWRCONFIG: 0x%02X, %s\n",
                        adcRegister,
                        toBin( binAsciiBuffer, adcRegister ));
         }

         for( channelIndex = 0; channelIndex < ADC_CHANNELS_PER_CHIP; channelIndex++ )
         {
            if( pI2cBus->paChip[chipIndex]->paChannel[channelIndex] == NULL )
               continue;
            seq_printf( pSeqFile, "\t\t%s%d%c%d:\n",
                        g_data.pName,
                        pI2cBus->pI2cAdapter->nr,
                        'A' + chipIndex,
                        channelIndex );
            seq_printf( pSeqFile, "\t\t\tOpen-count: %d\n",
                        atomic_read( &pI2cBus->paChip[chipIndex]->paChannel[channelIndex]->openCounter ));
            seq_printf( pSeqFile, "\t\t\tReadmode: %s\n",
                        getOutputFormatName( pI2cBus->paChip[chipIndex]->paChannel[channelIndex]->outputFormat ));
            if( adcReadUpperLimitThreshold( pI2cBus->paChip[chipIndex]->paChannel[channelIndex], &adcRegister ) < 0 )
            {
               seq_printf( pSeqFile,
                           "\t\t\tCouldn't read upper limit register %d on address 0x%02X!\n",
                           channelIndex, g_ads7924InternList[channelIndex].upperLimit );

            }
            else
            {
               seq_printf( pSeqFile, "\t\t\tULR%d: 0x%02X\n", channelIndex, adcRegister );
            }

            if( adcReadLowerLimitThreshold( pI2cBus->paChip[chipIndex]->paChannel[channelIndex], &adcRegister ) < 0 )
            {
               seq_printf( pSeqFile,
                           "\t\t\tCouldn't read lower limit register %d on address 0x%02X!\n",
                           channelIndex, g_ads7924InternList[channelIndex].lowerLimit );

            }
            else
            {
               seq_printf( pSeqFile, "\t\t\tLLR%d: 0x%02X\n", channelIndex, adcRegister );
            }

         } /* for( channelIndex = 0; channelIndex < ADC_CHANNELS_PER_CHIP; channelIndex++ ) */
      } /* for( chipIndex = 0; chipIndex < ADC_CHIPS_PER_BUS; chipIndex++ ) */
   } /* FOR_EACH_I2C_BUS( pI2cBus ) */

   return 0;
}

/*!-----------------------------------------------------------------------------
 */
static int _procOnOpen( struct inode* pInode, struct file *pFile )
{
   return single_open( pFile, procOnOpen, PDE_DATA( pInode ) );
}

/*!----------------------------------------------------------------------------
 * @brief Function is for test-purposes only.
 */
static ssize_t procOnWrite( struct file* seq, const char* pData,
                            size_t len, loff_t* pPos )
{
   DEBUG_MESSAGE( "\n" );
   return len;
}

/*-----------------------------------------------------------------------------
 */
const struct file_operations mg_procFileOps =
{
  .owner   = THIS_MODULE,
  .open    = _procOnOpen,
  .read    = seq_read,
  .write   = procOnWrite,
  .llseek  = seq_lseek,
  .release = single_release,
};

#endif /* if defined( CONFIG_PROC_FS ) || defined(__DOXYGEN__) */
/*================================== EOF ====================================*/
