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
 * @file ads7924fileIo.h
 * @brief Declaration of the device-file IO user-interface of ADS7924
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.05.05
 * @see ads7924fileIo.c
 */
#ifndef _ADS7924FILEIO_H
#include "ads7924driver.h"

/*!----------------------------------------------------------------------------
 * @brief Item object of function-table for ioctl of the entire chip-access.
 * @see mg_fTabIoctrlChip
 * @see onChipIoctrl
 */
typedef struct
{
   /*!
    * @brief The name will be used for debug- and/or error-messages and
    *        (if CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS defined,)
    *        in the process-files system.
    */
   const char*        name;

   /*!
    * @brief Operation code, corresponds to the second parameter
    *        of the user-space-function ioctl().
    */
   const unsigned int number;

   /*!
    * @brief Pointer of to the opcode related callback-function.
    * @param pChip Pointer to the addressed ADS7924-chip
    * @param arg Corresponds to the third parameter of the
    *            user-space function ioctl().
    */
   long (*function)( ADS7924_T* pChip, unsigned long arg );
} IOC_CHIP_INFO_T;

extern const IOC_CHIP_INFO_T mg_fTabIoctrlChip[];

/*!----------------------------------------------------------------------------
 * @brief Item object of function-table for ioctl of single channel access.
 * @see mg_fTabIoctrlChannel
 * @see onChannelIoctrl
 */
typedef struct
{
   /*!
    * @brief The name will be used for debug- and/or error-messages and
    *        (if CONFIG_ADS7924_SHOW_IOCTL_COMMANDS_IN_PROC_FS defined,)
    *        in the process-files system.
    */
   const char*        name;

   /*!
    * @brief Operation code, corresponds to the second parameter
    *        of the user-space-function ioctl().
    */
   const unsigned int number;

   /*!
    * @brief Pointer of to the opcode related callback-function.
    * @param pChannel Pointer to the analog-channel of the
    *        addressed ADS7924-chip
    * @param arg Corresponds to the third parameter of the
    *            user-space function ioctl().
    */
   long (*function)( ADC_CHANNEL_T* pChannel, unsigned long arg );
} IOC_CHANNEL_INFO_T;

extern const IOC_CHANNEL_INFO_T mg_fTabIoctrlChannel[];

/*!----------------------------------------------------------------------------
 */
extern const struct file_operations mg_fops;

#endif /* ifndef _ADS7924FILEIO_H */
/*================================== EOF ====================================*/
