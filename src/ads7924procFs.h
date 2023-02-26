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
 * @file ads7924procFs.h
 * @author Ulrich Becker
 * @copyright www.INKATRON.de
 * @date 2017.05.10
 * @brief Representation of the ADS7924 driver status in the
 *                       process file-system.
 * @see ads7924procFs.c
 */
#ifndef _ADS7924FS_H
#define _ADS7924FS_H

#include <linux/proc_fs.h>
#include <linux/seq_file.h>

extern const struct file_operations mg_procFileOps;

#endif /* ifndef _ADS7924FS_H */
/*================================== EOF ====================================*/
