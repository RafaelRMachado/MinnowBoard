/** @file
  Implement the platform specific PHY configuration routine.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <Library/EthernetAccess.h>
#include <Library/EthernetPhy.h>
#include <Library/PhyConfigLib.h>


/**
  Platform specific PHY configuration routines

  @param[in] PciIo      Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[in] PhyAddress Address of the PHY

**/
VOID
PlatformSpecificPhyConfiguration (
  IN EFI_PCI_IO_PROTOCOL *PciIo,
  IN UINTN PhyAddress
  )
{
  UINT16 PhyData;

  //
  //  Add transmit clock delay
  //
  EthernetPhyWrite ( PciIo,
                     PhyAddress,
                     0x1d,
                     5 );
  PhyData = EthernetPhyRead ( PciIo,
                              PhyAddress,
                              0x1e );
  PhyData |= 0x0100;
  EthernetPhyWrite ( PciIo,
                     PhyAddress,
                     0x1e,
                     PhyData );

  //
  //  Disable hibernation mode, keep the PHY powered up
  //
  EthernetPhyWrite ( PciIo,
                     PhyAddress,
                     0x1d,
                     0x0b );
  PhyData = EthernetPhyRead ( PciIo,
                              PhyAddress,
                              0x1e );
  PhyData |= 0x8000;
  EthernetPhyWrite ( PciIo,
                     PhyAddress,
                     0x1e,
                     PhyData );
}
