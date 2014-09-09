/** @file
  Declare the Platform specific PHY configuration interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _PHY_CONFIG_LIB_H_
#define _PHY_CONFIG_LIB_H_

#include <Library/EthernetAccess.h>
#include <Protocol/PciIo.h>

/**
  Platform specific PHY configuration routines

  @param[in] PciIo      Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[in] PhyAddress Address of the PHY

**/
VOID
PlatformSpecificPhyConfiguration (
  IN EFI_PCI_IO_PROTOCOL *PciIo,
  IN UINTN PhyAddress
  );

#endif  //  _PHY_CONFIG_LIB_H_