//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/** @file

Copyright (c) 2011 - 2012, Intel Corporation. All rights reserved.<BR>
This software and associated documentation (if any) is furnished
under a license and may only be used or copied in accordance
with the terms of the license. Except as permitted by such
license, no part of this software or documentation may be
reproduced, stored in a retrieval system, or transmitted in any
form or by any means without the express written consent of
Intel Corporation.

**/
#ifndef _EFI_SMM_SPI_DEVICE_H_
#define _EFI_SMM_SPI_DEVICE_H_

#include <Protocol/SpiDevice.h>

//
// GUID definition for protocol interface
//
#define SMM_SPI_DEVICE_PROTOCOL_GUID \
  {0xd963c5cd, 0x8cac, 0x498a, {0xbf, 0x78, 0xd1, 0x56, 0x49, 0x1, 0x85, 0x38}};

extern EFI_GUID gSmmSpiDeviceProtocolGuid;

//
// Common definition for regular and SMM versions of the protocol.
//
typedef SPI_DEVICE_PROTOCOL SMM_SPI_DEVICE_PROTOCOL;

#endif
