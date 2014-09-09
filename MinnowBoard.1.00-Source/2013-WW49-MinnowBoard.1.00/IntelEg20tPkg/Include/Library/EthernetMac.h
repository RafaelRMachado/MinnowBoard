/** @file
  Declare the Ethernet MAC library interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _ETHERNET_MAC_H_
#define _ETHERNET_MAC_H_

#include <Uefi/UefiPxe.h>
#include <Protocol/PciRootBridgeIo.h>
#include <Protocol/PciIo.h>

#define BAR_MEMORY                    1

typedef struct {
  UINT32 Mode;
  UINT32 RgmiiControl;
} MAC_CONTROL_REGISTERS;

/**
  Get the MAC address for the Ethernet controller

  @param[in]  PciIo       Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[out] MacAddress  Address of a buffer to receive the MAC address

  @retval EFI_SUCCESS     The function completed successfully.
  @retval EFI_NOT_FOUND   The variable was not found.

**/
EFI_STATUS
EthernetGetMacAddress (
  IN EFI_PCI_IO_PROTOCOL *PciIo,
  OUT EFI_MAC_ADDRESS * MacAddress
  );

/**
  Configure the Ethernet MAC based upon the requested settings

  @param[in, out] MacControl  Pointer to a MAC_CONTROL_REGISTERS structure
  @param[in] LinkSpeed        Speed of the link
  @param[in] LinkFullDuplex   Duplex setting for the link

**/
VOID
EthernetMacConfigure (
  IN OUT MAC_CONTROL_REGISTERS * MacControl,
  IN UINTN LinkSpeed,
  IN BOOLEAN LinkFullDuplex
  );

/**
  Set the MAC address for the Ethernet controller

  @param[in] PciRootBridgeIo Pointer to an EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL structure
  @param[in] PciAddress      Address of the device configuration space
  @param[in] MacAddress      Address of a buffer containing the MAC address

  @retval EFI_SUCCESS     The function completed successfully.

**/
EFI_STATUS
EthernetSetMacAddress (
  IN EFI_PCI_ROOT_BRIDGE_IO_PROTOCOL *PciRootBridgeIo,
  IN UINT64 PciAddress,
  IN EFI_MAC_ADDRESS * MacAddress
  );

#endif  //  _ETHERNET_MAC_H_
