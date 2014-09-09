//
// This file contains an 'Intel Peripheral Driver' and is
// licensed for Intel CPUs and chipsets under the terms of your
// license agreement with Intel or your vendor.  This file may
// be modified by the user, subject to additional terms of the
// license agreement
//
/** @file

  This file defines the EFI SPI Protocol which implements the
  Intel(R) ICH SPI Host Controller Compatibility Interface.

  Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
  This software and associated documentation (if any) is furnished
  under a license and may only be used or copied in accordance
  with the terms of the license. Except as permitted by such
  license, no part of this software or documentation may be
  reproduced, stored in a retrieval system, or transmitted in any
  form or by any means without the express written consent of
  Intel Corporation.

**/
#ifndef _EFI_SMM_SPI_H_
#define _EFI_SMM_SPI_H_

#include <Protocol/Spi.h>

//
// Define the SMM SPI protocol GUID information
//
#define EFI_SPI_SMM_PROTOCOL_GUID \
  {0x12f214f8, 0x407a, 0x41fa, {0x86, 0x37, 0x53, 0xa2, 0x3d, 0x70, 0x7b, 0x82}}
extern EFI_GUID gEfiSmmSpiProtocolGuid;

//
// SMM version of the protocol is the same as the normal protocol.
//
typedef EFI_SPI_PROTOCOL  EFI_SPI_SMM_PROTOCOL;

#endif
