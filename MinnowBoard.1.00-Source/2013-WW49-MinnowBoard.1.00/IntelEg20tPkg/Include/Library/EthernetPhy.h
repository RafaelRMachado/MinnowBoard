/** @file
  Declare the Ethernet PHY library interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _ETHERNET_PHY_H_
#define _ETHERNET_PHY_H_

#include <Library/EthernetAccess.h>
#include <Protocol/PciIo.h>

typedef struct {
  UINT16 Control;                       //  0
  UINT16 AutoNegotiationAdvertisement;  //  4
  UINT16 MasterSlaveControl;            //  9
} PHY_CONTROL_REGISTERS;

typedef struct {
  UINT16 Status;                        //  1
  UINT16 AutoNegotiationStatus;         //  5
  UINT16 MasterSlaveStatus;             //  10
} PHY_STATUS_REGISTERS;

/**
  Configure the PHY based upon the requested settings

  @param[in, out] LinkControl Pointer to a PHY_CONTROL_REGISTERS structure
  @param[in] LinkSpeed        Speed of the link
  @param[in] LinkFullDuplex   Duplex setting for the link

**/
VOID
EthernetPhyConfigure (
  IN OUT PHY_CONTROL_REGISTERS * LinkControl,
  IN UINTN LinkSpeed,
  IN BOOLEAN LinkFullDuplex
  );

/**
  Decode the PHY's the auto-negotiation results

  @param[in]  PhyStatus       Pointer to a PHY_STATUS_REGISTERS structure
  @param[out] LinkUp          Pointer to the link up value
  @param[out] LinkSpeed       Pointer to the link speed value
  @param[out] LinkFullDuplex  Pointer to the link duplex value

**/
VOID
EthernetPhyDecode (
  IN PHY_STATUS_REGISTERS * PhyStatus,
  OUT BOOLEAN * LinkUp,
  OUT UINTN * LinkSpeed,
  OUT BOOLEAN * LinkFullDuplex
  );

/**
  Determine the PHY address

  @param[in]  PciIo       Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[out] PhyId       Optional buffer to receive the OID, model
                          and revision

  @return  Return the PHY address.  A valid PHY address is in the
           range of 0-31.  If a PHY was not found the return value
           is 0xffffffff.

**/
UINTN
EthernetPhyGetAddress (
  IN EFI_PCI_IO_PROTOCOL * PciIo,
  OUT UINT32 * PhyId OPTIONAL
  );

/**
  Reset the PHY

  @param[in] PciIo        Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[in] PhyAddress   Address of the PHY
  @param[in] ResetActive  New state for the PHY reset line

**/
VOID
EthernetPhyReset (
  IN EFI_PCI_IO_PROTOCOL * PciIo,
  IN UINTN PhyAddress,
  BOOLEAN ResetActive
  );

/**
  Read data from the PHY

  @param[in] PciIo        Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[in] PhyAddress   Address of the PHY
  @param[in] PhyRegister  PHY register address

  @retval EFI_SUCCESS     Driver API properly initialized
  
**/
UINT16
EthernetPhyRead (
  IN EFI_PCI_IO_PROTOCOL * PciIo,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister
  );

/**
  Write data to the PHY

  @param[in] PciIo        Pointer to an EFI_PCI_IO_PROTOCOL structure
  @param[in] PhyAddress   Address of the PHY
  @param[in] PhyRegister  PHY register address
  @param[in] Value        Data to write to the PHY

**/
VOID
EthernetPhyWrite (
  IN EFI_PCI_IO_PROTOCOL * PciIo,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister,
  IN UINTN Value
  );

#endif  //  _ETHERNET_PHY_H_
