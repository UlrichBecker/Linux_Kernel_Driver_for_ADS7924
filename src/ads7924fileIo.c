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
 * @file ads7924fileIo.c
 * @brief Implementation of the device-file IO user-interface of ADS7924
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.05.05
 * @see ads7924fileIo.h
 */
#include "ads7924driver.h"
#include "ads7924core.h"
#include "ads7924fileIo.h"
#include "ads7924_dev_tree_names.h"
#include "ads7924ioctl.h"
#include <linux/slab.h>
#include <linux/poll.h>

//#define _DEBUG_POLL

#define IOCTL_ITEM( n, f ) { #n,   n, f    } //!<@brief Function-table item
#define IOCTL_LIST_END     { NULL, 0, NULL } //!<@brief Table terminator

/* Device file operations begin **********************************************/
/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 * @ingroup IOCTL_CHANNEL
 * @brief Callback functions specialized either for entire chip access or 
 *        single channel access.
 * @see getObjectByMinor
 * @see onOpen
 */
typedef struct
{
   /*!
    * @brief Placeholder for pointer to object ADS7924_T* or ADC_CHANNEL_T*
    * @see getChipFromInstance
    * @see getChannelFromInstance
    */
   void*        pPrivate;

   int          (*pOnOpen)( struct inode* pInode, struct file* pInstance );
   int          (*pOnClose)( struct inode *pInode, struct file* pInstance );
   ssize_t      (*pOnRead)( struct file* pInstance, char __user* pBuffer,
                            size_t len, loff_t* pOffset );
   ssize_t      (*pOnWrite)( struct file *pInstance,  const char __user* pBuffer,
                            size_t len, loff_t* pOffset );
   unsigned int (*pOnPoll)( struct file* pInstance, poll_table* pPollTable );
   long         (*pOnIoctrl)( struct file* pInstance, unsigned int cmd,
                              unsigned long arg );
} USER_INRTEFACE_T;

/* Call-back functions for the entire chip ADS2974 begin *********************/
/*!----------------------------------------------------------------------------
 * @see USER_INRTEFACE_T
 */
static inline ADS7924_T* getChipFromInstance( struct file* pInstance )
{
   BUG_ON( pInstance->private_data == NULL );
   return (ADS7924_T*)(((USER_INRTEFACE_T*)pInstance->private_data)->pPrivate);
}

/*!----------------------------------------------------------------------------
 *  @brief Not really used for entire chip access. Dummy
 */
static int onChipOpen( struct inode* pInode, struct file* pInstance )
{
   ADS7924_T* poChip;

   DEBUG_MESSAGE( ": Minor-number: %d\n", MINOR(pInode->i_rdev) );

   poChip = getChipFromInstance( pInstance );
   atomic_inc( &poChip->openCounter );
   DEBUG_MESSAGE( ": Open-counter: %d\n", atomic_read(&poChip->openCounter) );
   return 0;
};

/*!----------------------------------------------------------------------------
 *  @brief Not really used for entire chip access. Dummy
 */
static int onChipClose( struct inode *pInode, struct file* pInstance )
{
   ADS7924_T* poChip;

   DEBUG_MESSAGE( ": Minor-number: %d\n", MINOR(pInode->i_rdev) );

   poChip = getChipFromInstance( pInstance );
   atomic_dec( &poChip->openCounter );
   DEBUG_MESSAGE( ": Open-counter: %d\n", atomic_read(&poChip->openCounter) );
   return 0;
};

/*!----------------------------------------------------------------------------
 * @brief Not really used for entire chip access. Dummy
 */
static ssize_t onChipRead( struct file* pInstance, /*!< @see include/linux/fs.h   */
                           char __user* pBuffer,   /*!< buffer to fill with data */
                           size_t len,             /*!< length of the buffer     */
                           loff_t* pOffset )
{
   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   return 0;
}

/*!----------------------------------------------------------------------------
 *  @brief Not really used for entire chip access. Dummy
 */
static ssize_t onChipWrite( struct file *pInstance,
                        const char __user* pBuffer,
                        size_t len,
                        loff_t* pOffset )
{
   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   return len;
}

/*!----------------------------------------------------------------------------
 *  @brief At the moment not really used for entire chip access. Dummy
 */
static unsigned int onChipPoll( struct file* pInstance, poll_table* pPollTable )
{
#ifdef _DEBUG_POLL
   DEBUG_MESSAGE( "\n" );
#endif
   return 0;
}

/* ioctrl call back functions for entire chip access BEGIN *******************/
/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 * @brief Callback function performs a reset of the entire chip ADS7924
 */
static long onIoctlChipReset( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( "\n" );
   if( atomic_read( &pChip->openCounter ) > 1 )
   {
      ERROR_MESSAGE( ": Operation only for one opened instance permitted!\n" );
      return -EMFILE;
   }
   if( adcChipReset( pChip ) < 0 )
   {
      ERROR_MESSAGE( ": adcChipReset() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipSetMode( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( ": Setting mode 0x%02X: %s\n", (int)arg, getModeName( arg ));
   if( adcWriteModeByte( pChip, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteModeByte() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipGetMode( ADS7924_T* pChip, unsigned long arg )
{
   u8 mode;

   if( adcReadModeByte( pChip, &mode ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadModeByte() failed!\n" );
      return -EIO;
   }
   mode &= ADS7924_MODE_AUTO_BURST_SCAN_SLEEP;
   DEBUG_MESSAGE( ": Reading current mode: 0x%02X: %s\n", mode, getModeName( mode ));
   if( put_user( mode, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipSetIntconfig( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( ": Setting INTCONFIG: 0x%02X\n", (uint8_t)arg );
   if( adcWriteIntConfig( pChip, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteIntConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipGetIntconfig( ADS7924_T* pChip, unsigned long arg )
{
   u8 intConfig;

   if( adcReadIntConfig( pChip, &intConfig ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadIntConfig() failed\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading INTCONFIG: 0x%02X\n", intConfig );
   if( put_user( intConfig, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipEditIntconfig( ADS7924_T* pChip, unsigned long arg )
{
   ADS7924_BIT_EDIT_T eIntConfig;
   unsigned long ret;

   ret = copy_from_user( &eIntConfig, (void*)arg, sizeof( ADS7924_BIT_EDIT_T ) );
   if( ret != 0 )
   {
      ERROR_MESSAGE( ": copy_from_user() failed!\n" );
      return -EFAULT;
   }
   if( adcEditIntConfig( pChip, eIntConfig.m_set.byte, eIntConfig.m_clear.byte ) < 0 )
   {
      ERROR_MESSAGE( ": adcEditIntConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipSetSlpconfig( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( ": Setting SLPCONFIG: 0x%02X\n", (u8)arg );
   if( adcWriteSlpConfig( pChip, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteSlpConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipGetSlpconfig( ADS7924_T* pChip, unsigned long arg )
{
   u8 slpConfig;

   if( adcReadSlpConfig( pChip, &slpConfig ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadSlpConfig() failed\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading SLPCONFIG: 0x%02X\n", slpConfig );
   if( put_user( slpConfig, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipEditSlpconfig( ADS7924_T* pChip, unsigned long arg )
{
   ADS7924_BIT_EDIT_T eSlpConfig;
   unsigned long ret;

   ret = copy_from_user( &eSlpConfig, (void*)arg, sizeof( ADS7924_BIT_EDIT_T ) );
   if( ret != 0 )
   {
      ERROR_MESSAGE( ": copy_from_user() failed!\n" );
      return -EFAULT;
   }
   if( adcEditSlpConfig( pChip, eSlpConfig.m_set.byte, eSlpConfig.m_clear.byte ) < 0 )
   {
      ERROR_MESSAGE( ": adcEditSlpConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!---------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipSetAcqconfig( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( ": Setting ACQCONFIG: 0x%02X\n", (u8)arg );
   if( adcWriteAcqConfig( pChip, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteAcqConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!---------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipGetAcqconfig( ADS7924_T* pChip, unsigned long arg )
{
   u8 acqConfig;

   if( adcReadAcqConfig( pChip, &acqConfig ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadAcqConfig() failed!\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading ACQCONFIG: 0x%02X\n", acqConfig );
   if( put_user( acqConfig, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipEditAcqconfig( ADS7924_T* pChip, unsigned long arg )
{
   ADS7924_BIT_EDIT_T eAcqConfig;
   unsigned long ret;

   ret = copy_from_user( &eAcqConfig, (void*)arg, sizeof( ADS7924_BIT_EDIT_T ) );
   if( ret != 0 )
   {
      ERROR_MESSAGE( ": copy_from_user() failed!\n" );
      return -EFAULT;
   }
   if( adcEditAcqConfig( pChip, eAcqConfig.m_set.byte, eAcqConfig.m_clear.byte ) < 0 )
   {
      ERROR_MESSAGE( ": adcEditAcqConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipSetPwrconfig( ADS7924_T* pChip, unsigned long arg )
{
   DEBUG_MESSAGE( ": Setting PWRCONFIG: 0x%02X\n", (u8)arg );
   if( adcWritePwrConfig( pChip, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWritePwrConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipGetPwrconfig( ADS7924_T* pChip, unsigned long arg )
{
   u8 pwrConfig;

   if( adcReadPwrConfig( pChip, &pwrConfig ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadPwrConfig() failed!\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading PWRCONFIG: 0x%02X\n", pwrConfig );
   if( put_user( pwrConfig, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onIoCtlChipEditPwrconfig( ADS7924_T* pChip, unsigned long arg )
{
   ADS7924_BIT_EDIT_T ePwrConfig;
   unsigned long ret;

   ret = copy_from_user( &ePwrConfig, (void*)arg, sizeof( ADS7924_BIT_EDIT_T ) );
   if( ret != 0 )
   {
      ERROR_MESSAGE( ": copy_from_user() failed!\n" );
      return -EFAULT;
   }
   if( adcEditPwrConfig( pChip, ePwrConfig.m_set.byte, ePwrConfig.m_clear.byte ) < 0 )
   {
      ERROR_MESSAGE( ": adcEditPwrConfig() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 * @brief Initializer list of function table for entire chip specific ioctl().
 *
 * Alternative to switch-case statements, which would be very large
 * and very confusing.
 *
 * @see onChipIoctrl
 * @see IOC_CHIP_INFO_T
 */
const IOC_CHIP_INFO_T mg_fTabIoctrlChip[] =
{
   IOCTL_ITEM( ADS7924_IOCTL_RESET,          onIoctlChipReset ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_MODE,       onIoCtlChipSetMode ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_MODE,       onIoCtlChipGetMode ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_INTCONFIG,  onIoCtlChipSetIntconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_INTCONFIG,  onIoCtlChipGetIntconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_EDIT_INTCONFIG, onIoCtlChipEditIntconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_SLPCONFIG,  onIoCtlChipSetSlpconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_SLPCONFIG,  onIoCtlChipGetSlpconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_EDIT_SLPCONFIG, onIoCtlChipEditSlpconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_ACQCONFIG,  onIoCtlChipSetAcqconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_ACQCONFIG,  onIoCtlChipGetAcqconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_EDIT_ACQCONFIG, onIoCtlChipEditAcqconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_PWRCONFIG,  onIoCtlChipSetPwrconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_PWRCONFIG,  onIoCtlChipGetPwrconfig ),
   IOCTL_ITEM( ADS7924_IOCTL_EDIT_PWRCONFIG, onIoCtlChipEditPwrconfig ),
   IOCTL_LIST_END
};

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHIP
 */
static long onChipIoctrl( struct file* pInstance,
                          unsigned int cmd,
                          unsigned long arg )
{
   int ret = 0;
   const IOC_CHIP_INFO_T* pCurrentItem;

   DEBUG_MESSAGE( ": cmd = %d arg = %08lX\n", cmd, arg );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );

   for( pCurrentItem = mg_fTabIoctrlChip; pCurrentItem->function != NULL; pCurrentItem++ )
   {
      if( pCurrentItem->number != cmd )
         continue;
      DEBUG_MESSAGE( ": execute ioctl-command: %s\n", pCurrentItem->name );
      ret = pCurrentItem->function( getChipFromInstance(pInstance), arg );
      if( ret < 0 )
         ERROR_MESSAGE( ": executing of ioctl-command %s failed!\n",
                        pCurrentItem->name );
      return ret;
   }

   ERROR_MESSAGE( ": Unknown ioctl-command: 0x%08X\n", cmd );
   return -EINVAL;
}
/* ioctrl call back functions for entire chip access END *********************/
/* Call-back functions for the entire chip ADS2974 end ***********************/

/* Call-back functions for single analog channel begin ***********************/
/*!----------------------------------------------------------------------------
 *  @see USER_INRTEFACE_T
 */
static inline ADC_CHANNEL_T* getChannelFromInstance( struct file* pInstance )
{
   BUG_ON( pInstance->private_data == NULL );
   return (ADC_CHANNEL_T*)(((USER_INRTEFACE_T*)pInstance->private_data)->pPrivate);
}

/*!----------------------------------------------------------------------------
 * @brief Callback function for opening one of the analog channels for reading
 *        the received analog value or using ioctl().
 */
static int onChannelOpen( struct inode* pInode, struct file* pInstance )
{
   ADC_CHANNEL_T* poChannel;

   DEBUG_MESSAGE( ": Minor-number: %d\n", MINOR(pInode->i_rdev) );

   poChannel = getChannelFromInstance( pInstance );
   BUG_ON( poChannel == NULL );
   BUG_ON( poChannel->minor != MINOR(pInode->i_rdev) );

   if( atomic_inc_return( &poChannel->openCounter ) == 1 )
      poChannel->waitQueue.awoken = false;

   DEBUG_MESSAGE( ": Open-counter: %d\n", atomic_read(&poChannel->openCounter) );
   DEBUG_MESSAGE( ": Channel number = %d\n", poChannel->cannelNumber );

   return 0;
}

/*!----------------------------------------------------------------------------
 * @brief Callback function for closing one of the analog channels.
 */
static int onChannelClose( struct inode *pInode, struct file* pInstance )
{
   ADC_CHANNEL_T* poChannel;
   DEBUG_MESSAGE( ": Minor-number: %d\n", MINOR(pInode->i_rdev) );

   poChannel = getChannelFromInstance( pInstance );
   BUG_ON( poChannel == NULL );
   BUG_ON( poChannel->minor != MINOR(pInode->i_rdev) );

   atomic_dec( &poChannel->openCounter );
   DEBUG_MESSAGE( ": Open-counter: %d\n", atomic_read(&poChannel->openCounter) );
   DEBUG_MESSAGE( ": Channel number = %d\n", poChannel->cannelNumber );

   return 0;
}

/*!----------------------------------------------------------------------------
 * @brief Base function becomes invoked by the callback-function
 *        onChannelRead()
 * @see onChannelRead
 */
static inline ssize_t _onChannelRead( ADC_CHANNEL_T* pChannel,
                                      char __user* pBuffer,
                                      size_t len,
                                      loff_t* pOffset )
{
   char tmp[16];
   void* pOut;
   ssize_t n;
   size_t limit;
   VALUE_T result;

   if( ((*pOffset) == 0) && !pChannel->result.isValid )
   {
      if( readAnalogValue( pChannel ) < 0 )
      {
         ERROR_MESSAGE( ": Unable to read analog channel %d\n", pChannel->cannelNumber );
         return -EIO;
      }
   //   pChannel->result.isValid = false;
   }
   pChannel->result.isValid = false;

   result = getResult( pChannel );
   limit = min( len, sizeof(tmp)-1 );

   //TODO: snprintf() is a little bit expensive.
   //      Worry that they are called only once per user-read.
   switch( pChannel->outputFormat )
   {
      case OUT_BIN:
      {
         n = min( len, sizeof( result ));
         break;
      }
      case OUT_DEC:
      {
         n = snprintf( tmp, limit, "%d", result );
         tmp[n++] = '\0';
         break;
      }
      case OUT_HEX:
      {
         n = snprintf( tmp, limit, "%03X", result );
         tmp[n++] = '\0';
         break;
      }
      default:
      {
         BUG_ON( true ); // respectively: assert( false )
         break;
      }
   }

   DEBUG_MESSAGE( ": n = %d\n", n );
   n -= (*pOffset);
   if( n <= 0 )
   {
      *pOffset = 0;
      //pChannel->waitQueue.waiting = true; //TEST
      return 0;
   }

   if( pChannel->outputFormat == OUT_BIN )
      pOut = &result;
   else
      pOut = tmp + *pOffset;

   if( copy_to_user( pBuffer, pOut, n ) != 0 )
   {
      ERROR_MESSAGE( "copy_to_user: %d bytes\n", n );
      return -EFAULT;
   }

   if( pChannel->outputFormat != OUT_BIN )
      (*pOffset) += n;

   return n;
   /* Number of bytes successfully read. */
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function read() from the
 *        user-space.
 * @note The kernel invokes onRead as many times till it returns 0 !!!
 */
static ssize_t onChannelRead( struct file* pInstance,   /*!< @see include/linux/fs.h   */
                              char __user* pBuffer,     /*!< buffer to fill with data */
                              size_t len,               /*!< length of the buffer     */
                              loff_t* pOffset )
{
   ssize_t n;
   ADC_CHANNEL_T* pChannel;

   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );

   pChannel = getChannelFromInstance( pInstance );
   BUG_ON( pChannel == NULL );

   DEBUG_MESSAGE( ": Minor: %d\n", pChannel->minor );
   DEBUG_MESSAGE( ": Open-counter: %d\n", atomic_read( &pChannel->openCounter ));
   DEBUG_MESSAGE( ": *** Channel number = %d ***\n", pChannel->cannelNumber );

   if( pChannel->waitQueue.waiting && ((pInstance->f_flags & O_NONBLOCK) != 0) )
   {
      DEBUG_MESSAGE( ": No new analog data present.\n" );
      return -EAGAIN;
   }
   if( sleepChannelIfNecessary( pChannel ) )
   {
      DEBUG_MESSAGE( ": Signal occurred.\n" );
      return -ERESTARTSYS;
   }

   mutex_lock( &pChannel->oMutex );
   n = _onChannelRead( pChannel, pBuffer, len, pOffset );
   mutex_unlock( &pChannel->oMutex );

   DEBUG_MESSAGE( ": Return %d\n", n );
   return n;
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function write() from the
 *        user-space.
 *
 * At the moment this function will not really used it is a dummy.
 */
static ssize_t onChannelWrite( struct file *pInstance,
                               const char __user* pBuffer,
                               size_t len,
                               loff_t* pOffset )
{
#ifdef CONFIG_DEBUG_ADS7924
   ADC_CHANNEL_T* pChannel = getChannelFromInstance( pInstance );
#endif

   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );

   DEBUG_MESSAGE( "   Minor: %d\n", pChannel->minor );
   DEBUG_MESSAGE( "   Open-counter: %d\n",
                   atomic_read( &pChannel->openCounter ));
   DEBUG_MESSAGE( "Channel number = %d\n", pChannel->cannelNumber );
   DEBUG_MESSAGE( "   Received: \"%s\"\n", pBuffer );
   return len;
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function select() from the
 *        user-space.
 */
static unsigned int onChannelPoll( struct file* pInstance, poll_table* pPollTable )
{
   ADC_CHANNEL_T* pChannel = getChannelFromInstance( pInstance );
#ifdef _DEBUG_POLL
   DEBUG_MESSAGE( ": Channel number: %d\n", pChannel->cannelNumber );
#endif
   poll_wait( pInstance, &pChannel->waitQueue.queue, pPollTable );
   if( isAwoken( &pChannel->waitQueue ) )
   {
   #ifdef _DEBUG_POLL
      DEBUG_MESSAGE( ": Return != 0\n" );
   #endif
      return (POLLIN | POLLRDNORM);
   }
   return 0;
}

/* ioctrl call back functions for single channel access BEGIN ****************/

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 * @see onIoctlSetReadmodeBin
 * @see onIoctlSetReadmodeDec
 * @see onIoctlSetReadmodeHex
 */
static long setOutputFormat( ADC_CHANNEL_T* pChannel, OUTPUT_FORMAT_T outFormat )
{
   if( atomic_read( &pChannel->openCounter ) > 1 )
   {
      ERROR_MESSAGE( ": Operation only for one opened instance permitted!\n" );
      return -EMFILE;
   }
   pChannel->outputFormat = outFormat;
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoctlSetReadmodeBin( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   DEBUG_MESSAGE( "\n" );
   return setOutputFormat( pChannel, OUT_BIN );
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoctlSetReadmodeDec( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   DEBUG_MESSAGE( "\n" );
   return setOutputFormat( pChannel, OUT_DEC );
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoctlSetReadmodeHex( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   DEBUG_MESSAGE( "\n" );
   return setOutputFormat( pChannel, OUT_HEX );
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlSetUlr( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   DEBUG_MESSAGE( ": 0x%02X\n", (int)arg );
   if( adcWriteUpperLimitThreshold( pChannel, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteUpperLimitThreshold() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlSetLlr( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   DEBUG_MESSAGE( ": 0x%02X\n", (int)arg );
   if( adcWriteLowerLimitThreshold( pChannel, arg ) < 0 )
   {
      ERROR_MESSAGE( ": adcWriteLowerLimitThreshold() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlGetUlr( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   u8 threshold;

   if( adcReadUpperLimitThreshold( pChannel, &threshold ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadUpperLimitThreshold() failed!\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading ULR%d: 0x%02X\n", pChannel->cannelNumber, threshold );
   if( put_user( threshold, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlGetLlr( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   u8 threshold;

   if( adcReadLowerLimitThreshold( pChannel, &threshold ) < 0 )
   {
      ERROR_MESSAGE( ": adcReadLowerLimitThreshold() failed!\n" );
      return -EIO;
   }
   DEBUG_MESSAGE( ": Reading LLR%d: 0x%02X\n", pChannel->cannelNumber, threshold );
   if( put_user( threshold, (u8*)arg ) < 0 )
   {
      ERROR_MESSAGE( ": put_user() failed!\n" );
      return -EFAULT;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlAlarmEnable( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   if( adcAlarmEnable( pChannel ) < 0 )
   {
      ERROR_MESSAGE( ": adcAlarmEnable() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 */
static long onIoCtlAlarmDisable( ADC_CHANNEL_T* pChannel, unsigned long arg )
{
   if( adcAlarmDisable( pChannel ) < 0 )
   {
      ERROR_MESSAGE( ": adcAlarmDisable() failed!\n" );
      return -EIO;
   }
   return 0;
}

/*!----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 * @brief Initializer list of function table for channel specific ioctl().
 *
 * Alternative to switch-case statements, which would be very large
 * and very confusing.
 *
 * @see IOC_CHANNEL_INFO_T
 * @see onChannelIoctrl
 */
const IOC_CHANNEL_INFO_T mg_fTabIoctrlChannel[] =
{
   IOCTL_ITEM( ADS7924_IOCTL_READMODE_BIN,  onIoctlSetReadmodeBin ),
   IOCTL_ITEM( ADS7924_IOCTL_READMODE_DEC,  onIoctlSetReadmodeDec ),
   IOCTL_ITEM( ADS7924_IOCTL_READMODE_HEX,  onIoctlSetReadmodeHex ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_ULR,       onIoCtlSetUlr ),
   IOCTL_ITEM( ADS7924_IOCTL_SET_LLR,       onIoCtlSetLlr ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_ULR,       onIoCtlGetUlr ),
   IOCTL_ITEM( ADS7924_IOCTL_GET_LLR,       onIoCtlGetLlr ),
   IOCTL_ITEM( ADS7924_IOCTL_ALARM_ENABLE,  onIoCtlAlarmEnable ),
   IOCTL_ITEM( ADS7924_IOCTL_ALARM_DISABLE, onIoCtlAlarmDisable ),
   IOCTL_LIST_END
};

/*!-----------------------------------------------------------------------------
 * @ingroup IOCTL_CHANNEL
 * @brief Callback function becomes invoked by the function ioctrl() from the
 *        user-space.
 */
static long onChannelIoctrl( struct file* pInstance,
                            unsigned int cmd,
                            unsigned long arg )
{
   int ret = 0;
   const IOC_CHANNEL_INFO_T* pCurrentItem;

   DEBUG_MESSAGE( ": cmd = %d arg = %08lX\n", cmd, arg );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   DEBUG_MESSAGE( "   Minor: %d\n", getChannelFromInstance(pInstance)->minor );
   DEBUG_MESSAGE( "   Channel number: %d\n", getChannelFromInstance(pInstance)->cannelNumber );
   DEBUG_MESSAGE( "   Open-counter: %d\n",
                   atomic_read( &(getChannelFromInstance(pInstance)->openCounter) ));

   for( pCurrentItem = mg_fTabIoctrlChannel; pCurrentItem->function != NULL; pCurrentItem++ )
   {
      if( pCurrentItem->number != cmd )
         continue;
      DEBUG_MESSAGE( ": execute ioctl-command: %s\n", pCurrentItem->name );
      ret = pCurrentItem->function( getChannelFromInstance(pInstance), arg );
      if( ret < 0 )
         ERROR_MESSAGE( ": executing of ioctl-command %s failed!\n",
                        pCurrentItem->name );
      return ret;
   }

   ERROR_MESSAGE( ": Unknown ioctl-command: 0x%08X\n", cmd );
   return -EINVAL;
}

/* ioctrl call back functions for single channel access END ******************/

/* Call-back functions for single analog channel end *************************/

/*!---------------------------------------------------------------------------
 * @brief Return-type for function getObjectByMinor
 * @see getObjectByMinor
 * @see onOpen
 */
typedef enum
{
   IS_ADC_CHANNEL,
   IS_ADC_CHIP,
   NOT_FOUND
} GET_OBJECT_RETURN_T;

/*!----------------------------------------------------------------------------
 * @brief Helper function for the callback function onOpen().
 * 
 * Function searches the appropriate object by the given minor-number and
 * copy it in the first argument ppObject if found.
 *
 * @param  ppObject Copy target for the found object-pointer.
 * @param  minor Minor-number
 * @retval IS_ADC_CHIP *ppObject points to a object of type ADS7924_T.
 * @retval IS_ADC_CHANNEL *ppObject points to a object of type ADC_CHANNEL_T.
 * @retval NOT_FOUND No object found.
 * @see USER_INRTEFACE_T
 * @see onOpen
 */
static inline GET_OBJECT_RETURN_T getObjectByMinor( void** ppObject, const int minor )
{
   int            chipIndex;
   int            adcChannelIndex;
   BUS_T*         pI2cBus;
   ADS7924_T*     pAds7924;
   ADC_CHANNEL_T* pChannel;

   FOR_EACH_I2C_BUS( pI2cBus )
   {
      for( chipIndex = 0; chipIndex < ADC_CHIPS_PER_BUS; chipIndex++ )
      {
         pAds7924 = pI2cBus->paChip[chipIndex];
         if( pAds7924 == NULL )
            continue;
         if( pAds7924->minor == minor )
         {
            *ppObject = pAds7924;
            return IS_ADC_CHIP;
         }
         for( adcChannelIndex = 0; adcChannelIndex < ADC_CHANNELS_PER_CHIP; adcChannelIndex++ )
         {
            pChannel = pAds7924->paChannel[adcChannelIndex];
            if( pChannel == NULL )
               continue;
            if( pChannel->minor == minor )
            {
               *ppObject = pChannel;
               return IS_ADC_CHANNEL;
            }
         }
      }
   }
   *ppObject = NULL;
   return NOT_FOUND;
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function open() from the
 *        user-space.
 * @see USER_INRTEFACE_T
 */
static int onOpen( struct inode* pInode, struct file* pInstance )
{
   USER_INRTEFACE_T* pUserInterface;
   int               minor = MINOR(pInode->i_rdev);

   DEBUG_MESSAGE( ": Minor-number: %d\n", minor );
   BUG_ON( pInstance->private_data != NULL );

   pUserInterface = kmalloc( sizeof( USER_INRTEFACE_T ), GFP_KERNEL );
   if( pUserInterface == NULL )
   {
      ERROR_MESSAGE( ": Unable to allocate kernel-memory for USER_INRTEFACE_T !\n" );
      return -EIO;
   }

   switch( getObjectByMinor( &pUserInterface->pPrivate, minor ) )
   {
      case IS_ADC_CHIP:
      {
         DEBUG_MESSAGE( ": Initializing user-interface for entire chip-access.\n" );
         pUserInterface->pOnOpen   = onChipOpen;
         pUserInterface->pOnClose  = onChipClose;
         pUserInterface->pOnRead   = onChipRead;
         pUserInterface->pOnWrite  = onChipWrite;
         pUserInterface->pOnPoll   = onChipPoll;
         pUserInterface->pOnIoctrl = onChipIoctrl;
         break;
      }
      case IS_ADC_CHANNEL:
      {
         DEBUG_MESSAGE( ": Initializing user-interface for single channel access.\n" );
         pUserInterface->pOnOpen   = onChannelOpen;
         pUserInterface->pOnClose  = onChannelClose;
         pUserInterface->pOnRead   = onChannelRead;
         pUserInterface->pOnWrite  = onChannelWrite;
         pUserInterface->pOnPoll   = onChannelPoll;
         pUserInterface->pOnIoctrl = onChannelIoctrl;
         break;
      }
      case NOT_FOUND:
      {
         ERROR_MESSAGE( ": Corrupt minor: %d\n", minor );
         kfree( pUserInterface );
         return -EIO;
      }
      default:
      {
         BUG_ON( true );
         break;
      }
   }
   pInstance->private_data = pUserInterface;

   return pUserInterface->pOnOpen( pInode, pInstance );
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function close() from the
 *        user-space.
 */
static int onClose( struct inode *pInode, struct file* pInstance )
{
   int ret;
   DEBUG_MESSAGE( ": Minor-number: %d\n", MINOR(pInode->i_rdev) );
   BUG_ON( pInstance->private_data == NULL );
   BUG_ON((((USER_INRTEFACE_T*)pInstance->private_data)->pOnClose != onChipClose) &&
          (((USER_INRTEFACE_T*)pInstance->private_data)->pOnClose != onChannelClose)
         );

   ret = ((USER_INRTEFACE_T*)pInstance->private_data)->pOnClose( pInode, pInstance );

   kfree( pInstance->private_data );
   pInstance->private_data = NULL;

   return ret;
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function read() from the
 *        user-space.
 * @note The kernel invokes onRead as many times till it returns 0 !!!
 */
static ssize_t onRead( struct file* pInstance,   /*!< @see include/linux/fs.h   */
                       char __user* pBuffer,     /*!< buffer to fill with data */
                       size_t len,               /*!< length of the buffer     */
                       loff_t* pOffset )
{
   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   BUG_ON((((USER_INRTEFACE_T*)pInstance->private_data)->pOnRead != onChipRead) &&
          (((USER_INRTEFACE_T*)pInstance->private_data)->pOnRead != onChannelRead)
         );

   return ((USER_INRTEFACE_T*)pInstance->private_data)->pOnRead( pInstance, pBuffer, len, pOffset );
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function write() from the
 *        user-space.
 */
static ssize_t onWrite( struct file *pInstance,
                        const char __user* pBuffer,
                        size_t len,
                        loff_t* pOffset )
{
   DEBUG_MESSAGE( ": len = %ld, offset = %lld\n", (long int)len, *pOffset );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   BUG_ON((((USER_INRTEFACE_T*)pInstance->private_data)->pOnWrite != onChipWrite) &&
          (((USER_INRTEFACE_T*)pInstance->private_data)->pOnWrite != onChannelWrite)
         );

   return ((USER_INRTEFACE_T*)pInstance->private_data)->pOnWrite( pInstance, pBuffer, len, pOffset );
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function select() from the
 *        user-space.
 */
static unsigned int onPoll( struct file* pInstance, poll_table* pPollTable )
{
#ifdef _DEBUG_POLL
   DEBUG_MESSAGE( "\n" );
#endif
   BUG_ON( pInstance->private_data == NULL );
   BUG_ON((((USER_INRTEFACE_T*)pInstance->private_data)->pOnPoll != onChipPoll) &&
          (((USER_INRTEFACE_T*)pInstance->private_data)->pOnPoll != onChannelPoll)
         );
   return ((USER_INRTEFACE_T*)pInstance->private_data)->pOnPoll( pInstance, pPollTable );
}

/*!----------------------------------------------------------------------------
 * @brief Callback function becomes invoked by the function ioctrl() from the
 *        user-space.
 */
static long onIoctrl( struct file* pInstance,
                      unsigned int cmd,
                      unsigned long arg )
{
   DEBUG_MESSAGE( ": cmd = %d arg = %08lX\n", cmd, arg );
   DEBUG_ACCESSMODE( pInstance );
   BUG_ON( pInstance->private_data == NULL );
   BUG_ON((((USER_INRTEFACE_T*)pInstance->private_data)->pOnIoctrl != onChipIoctrl) &&
          (((USER_INRTEFACE_T*)pInstance->private_data)->pOnIoctrl != onChannelIoctrl)
         );

   return ((USER_INRTEFACE_T*)pInstance->private_data)->pOnIoctrl( pInstance, cmd, arg );
}

/*-----------------------------------------------------------------------------
 */
const struct file_operations mg_fops =
{
  .owner          = THIS_MODULE,
  .open           = onOpen,
  .release        = onClose,
  .read           = onRead,
  .write          = onWrite,
  .poll           = onPoll,
  .unlocked_ioctl = onIoctrl
};

/* Device file operations end ************************************************/
/*================================== EOF ====================================*/
