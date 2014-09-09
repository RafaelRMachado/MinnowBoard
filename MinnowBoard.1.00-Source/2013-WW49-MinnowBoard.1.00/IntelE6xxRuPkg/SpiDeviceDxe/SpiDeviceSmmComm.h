/*++
  This file contains an 'Intel Peripheral Driver' and is
  licensed for Intel CPUs and chipsets under the terms of your
  license agreement with Intel or your vendor.  This file may
  be modified by the user, subject to additional terms of the
  license agreement
--*/
/** @file

  SMM Communication formats for the SPI Device protocols.

  Copyright (c) 2011 - 2013, Intel Corporation. All rights reserved. <BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/

#ifndef _SPI_DEVICE_SMM_COMM_H_
#define _SPI_DEVICE_SMM_COMM_H_

#include <Protocol/SmmSpiDevice.h>

//
// Define communication constants
//
#define SPI_DEV_FUNCTION_READ         1
#define SPI_DEV_FUNCTION_WRITE        2
#define SPI_DEV_FUNCTION_ERASE        3
#define SPI_DEV_FUNCTION_LOCK         4
#define SPI_DEV_FUNCTION_SET_RANGE    5
#define SPI_DEV_FUNCTION_LOCK_RANGES  6

//
// Generic SPI Device communication structure header.
//
typedef struct {
  UINTN       Function;
  EFI_STATUS  ReturnStatus;
  UINT8       Data[1];
} SMM_SPI_DEV_COMMUNICATE_FUNCTION_HEADER;

//
// Macros used to determine size of the headers without data size.
//
#define SMM_COMMUNICATE_HEADER_SIZE  (OFFSET_OF (EFI_SMM_COMMUNICATE_HEADER, Data))
#define SMM_SPI_DEV_COMMUNICATE_FUNCTION_HEADER_SIZE  (OFFSET_OF (SMM_SPI_DEV_COMMUNICATE_FUNCTION_HEADER, Data))

//
// SPI Read, Write and Erase Data.  Erase will not have any extra data.
//
typedef struct {
  UINTN     Offset;
  UINTN     Size;
} SMM_SPI_DEV_READ_WRITE_ERASE_HEADER;

//
// SPI Lock
//
typedef struct {
  UINTN   Offset;
  UINTN   Size;
  BOOLEAN Lock;
} SMM_SPI_DEV_LOCK_HEADER;

//
// SPI Set Range
//
typedef struct {
  UINTN   Offset;
  UINTN   Size;
  BOOLEAN ReadLock;
  BOOLEAN WriteLock;
} SMM_SPI_DEV_SET_RANGE_HEADER;

#endif
