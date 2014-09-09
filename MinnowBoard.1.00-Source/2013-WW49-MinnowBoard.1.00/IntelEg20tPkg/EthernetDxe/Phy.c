/** @file
  Implement the Ethernet PHY support routines.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ethernet.h"

/**
  Read data from the PHY

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[in] PhyAddress   Address of the PHY
  @param[in] PhyRegister  PHY register address

  @retval EFI_SUCCESS     Driver API properly initialized
  
**/
UINT16
EthernetPhyRead (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister
 )
{
  UINT32 Data;
  UINT16 PhyData;
  UINTN  Count;

  //
  //  Read the PHY register
  //
  PciIoMemWrite32 (
    EthernetContext, 
    ETH_REG_MIIM, 
    ((PhyAddress << 21) & MIIM_PHY_ADDR) | ((PhyRegister << 16) & MIIM_REG_ADDR) | MIIM_OPER_READ
    );

  //
  //  Wait for the read to complete
  //
  Count = 0;
  do {
    gBS->Stall (10);
    Count++;
    Data = PciIoMemRead32 (EthernetContext, ETH_REG_MIIM);
  } while (0 == (Data & MIIM_OPER) && Count < 20);

  //
  //  Get the PHY data
  //
  PhyData = (UINT16)Data;
  return PhyData;
}

/**
  Write data to the PHY

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[in] PhyAddress   Address of the PHY
  @param[in] PhyRegister  PHY register address
  @param[in] Value        Data to write to the PHY

**/
VOID
EthernetPhyWrite (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister,
  IN UINTN Value
 )
{
  UINT32 Data;
  UINTN  Count;

  //
  //  Wait for the previous operation to complete
  //
  Count = 0;
  do {
    gBS->Stall (10);
    Count++;
    Data = PciIoMemRead32 (EthernetContext, ETH_REG_MIIM);
  } while (0 == (Data & MIIM_OPER) && Count < 20);

  //
  //  Write the PHY register
  //
  PciIoMemWrite32 (
    EthernetContext, 
    ETH_REG_MIIM, 
    ((PhyAddress << 21) & MIIM_PHY_ADDR) | ((PhyRegister << 16) & MIIM_REG_ADDR) | MIIM_OPER_WRITE | (Value & MIIM_DATA)
    );

  //
  //  Wait for the write to complete
  //
  Count = 0;
  do {
    gBS->Stall (10);
    Count++;
    Data = PciIoMemRead32 (EthernetContext, ETH_REG_MIIM);
  } while (0 == (Data & MIIM_OPER) && Count < 20);
}


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
 )
{
  UINT16 AutoNegotiationAdvertisement;
  UINT16 Control;
  UINT16 MasterSlaveControl;

  //
  //  Get the PHY registers
  //
  Control = LinkControl->Control;
  AutoNegotiationAdvertisement = LinkControl->AutoNegotiationAdvertisement;
  MasterSlaveControl = LinkControl->MasterSlaveControl;

  //
  //  Set the default values
  //
  Control &= ~(CONTROL_RESET
              | CONTROL_SPEED_SELECTION
              | CONTROL_POWER_DOWN
              | CONTROL_ISOLATE
              | CONTROL_DUPLEX_MODE);
  Control |= CONTROL_AUTO_NEGOTIATION_ENABLE
           | CONTROL_RESTART_AUTO_NEGOTIATION;
  AutoNegotiationAdvertisement &= ~(ANA_100BASE_TX_FD
                                   | ANA_100BASE_TX
                                   | ANA_10BASE_T_FD
                                   | ANA_10BASE_T);
  MasterSlaveControl &= ~(MSC_1000BASE_T_FD
                         | MSC_1000BASE_T);

  //
  //  Determine the connection properties
  //
  if (1000 == LinkSpeed) {
    //
    //  1000 Mb/s
    //
    Control |= CONTROL_SPEED_1000_MBPS;
    MasterSlaveControl |= MSC_1000BASE_T;
    if (LinkFullDuplex) {
      MasterSlaveControl |= MSC_1000BASE_T_FD;
    } else {
      MasterSlaveControl |= MSC_1000BASE_T;
    }
  } else if (100 == LinkSpeed) {
    //
    //  100 Mb/s
    //
    Control |= CONTROL_SPEED_100_MBPS;
    if (LinkFullDuplex) {
      AutoNegotiationAdvertisement |= ANA_100BASE_TX_FD;
    } else {
      AutoNegotiationAdvertisement |= ANA_100BASE_TX;
    }
  } else {
    //
    //  10 Mb/s
    //
    Control |= CONTROL_SPEED_10_MBPS;
    if (LinkFullDuplex) {
      AutoNegotiationAdvertisement |= ANA_10BASE_T_FD;
    } else {
      AutoNegotiationAdvertisement |= ANA_10BASE_T;
    }
  }

  //
  //  Update the PHY register values
  //
  LinkControl->Control = Control;
  LinkControl->AutoNegotiationAdvertisement = AutoNegotiationAdvertisement;
  LinkControl->MasterSlaveControl = MasterSlaveControl;
}

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
 )
{
  UINT16 AutoNegotiationStatus;
  BOOLEAN FullDuplex;
  UINT16 MasterSlaveStatus;
  UINTN Speed;
  UINT16 Status;

  //
  //  Get the PHY registers
  //
  Status = PhyStatus->Status;
  AutoNegotiationStatus = PhyStatus->AutoNegotiationStatus;
  MasterSlaveStatus = PhyStatus->MasterSlaveStatus;

  //
  //  Determine the connection properties
  //
  FullDuplex = FALSE;
  if ((0 != (Status & STATUS_AUTO_NEGOTIATION_COMPLETE))
    && (0 != (MasterSlaveStatus & MSS_STATUS_LP_1000))) {
    //
    //  1000 Mb/s
    ///
    Speed = 1000;
    if (0 != (MasterSlaveStatus & MSS_STATUS_LP_1000_FD)) {
      FullDuplex = TRUE;
    }
  } else if ((0 != (Status & STATUS_AUTO_NEGOTIATION_COMPLETE))
              && (0 != (AutoNegotiationStatus & (ANA_100BASE_TX | ANA_100BASE_TX_FD)))) {
    //
    //  100 Mb/s
    //
    Speed = 100;
    if (0 != (AutoNegotiationStatus & ANA_100BASE_TX_FD)) {
      FullDuplex = TRUE;
    }
  } else {
    //
    //  10 Mb/s
    //
    Speed = 10;
    if (0 != (AutoNegotiationStatus & ANA_10BASE_T_FD)) {
      FullDuplex = TRUE;
    }
  }

  //
  //  Return the connection properties
  //
  *LinkUp = (BOOLEAN)(0 != (Status & STATUS_LINK_UP));
  *LinkSpeed = Speed;
  *LinkFullDuplex = FullDuplex;
}

/**
  Reset the PHY

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure

**/
VOID
EthernetPhyReset (
  IN ETHERNET_CONTEXT  *EthernetContext
  )
{
  UINTN  Index;

  //
  //  Loop through all possible PHY addresses 
  //
  for (Index = 0; Index < 0x20; Index++) {
    //
    // Set RESET bit in PHY CONTROL register.  
    // This bit automatically clears after reset.
    //
    EthernetPhyWrite (EthernetContext, Index, PHY_CONTROL, CONTROL_RESET);
  }
}

/**
  Detect the PHY Identifier bu scanning all PHY Addresses

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[out] Identifier  Buffer to receive the OID, model and revision

  @return  Return the PHY address.  A valid PHY address is in the
           range of 0-31.  If a PHY was not found the return value
           is 0xffffffff.

**/
BOOLEAN
EthernetPhyGetIdentifier (
  IN ETHERNET_CONTEXT  *EthernetContext,
  OUT UINTN            *Address,
  OUT UINT32           *Identifier
 )
{
  UINTN  Index;

  ASSERT (Address != NULL);
  ASSERT (Identifier != NULL);
  for (Index = 0; Index < 32; Index++) {
    *Identifier = (UINT32)(EthernetPhyRead (EthernetContext, Index, PHY_PHY_IDENTIFIER_1) << 16);
    *Identifier = (UINT32)(*Identifier | EthernetPhyRead (EthernetContext, Index, PHY_PHY_IDENTIFIER_2));
    if (*Identifier != 0 && *Identifier != 0xffffffff) {
      *Address = Index;
      return TRUE;
    }
  }
  return FALSE;
}

/**
  Determine the PHY address

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[out] PhyId       Optional buffer to receive the OID, model
                          and revision

  @return  Return the PHY address.  A valid PHY address is in the
           range of 0-31.  If a PHY was not found the return value
           is 0xffffffff.

**/
BOOLEAN
EthernetPhyGetAddress (
  IN  ETHERNET_CONTEXT  *EthernetContext,
  OUT UINTN             *PhyAddress,
  OUT UINT32            *PhyId        
 )
{
  UINTN   Address1;
  UINTN   Address2;
  UINT32  Identifier1;
  UINT32  Identifier2;

  //
  // Detect the PHY Address and PHY Identifier twice
  //
  if (EthernetPhyGetIdentifier (EthernetContext, &Address1, &Identifier1)) {
    if (EthernetPhyGetIdentifier (EthernetContext, &Address2, &Identifier2)) {
      //
      // Address is detected if both addresses and identifiers match 
      //
      if ((Address1 == Address2) && (Identifier1 == Identifier2)) {
        *PhyAddress = Address1;
        *PhyId      = Identifier1;
        return TRUE;
      }
    }
  }

  DEBUG ((DEBUG_VERBOSE, "EthernetPhyGetAddress() - Can not detect Phy Address\n"));

  return FALSE;
}

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
 )
{
  UINT32 Mode;
  UINT32 RgmiiControl;

  //
  //  Set the default values
  //
  Mode = MacControl->Mode
       & (~(MODE_DUPLEX_MODE | MODE_ETHER_MODE));
  RgmiiControl = MacControl->RgmiiControl
               & (~ (RGMII_CTL_RGMII_RATE
                     | RGMII_CTL_CRS_SEL));

  //
  //  Determine the connection properties
  //
  if (1000 == LinkSpeed) {
    //
    //  1000 Mb/s
    //
    Mode |= MODE_ETHER_MODE;
    RgmiiControl |= RGMII_CTL_RGMII_RATE_125_MHZ
                  | RGMII_CTL_RGMII_MODE;
    if (LinkFullDuplex) {
      Mode |= MODE_DUPLEX_MODE;
    }
  } else if (100 == LinkSpeed) {
    //
    //  100 Mb/s
    //
    RgmiiControl |= RGMII_CTL_RGMII_RATE_25_MHZ
                  | RGMII_CTL_RGMII_MODE;
    if (LinkFullDuplex) {
      Mode |= MODE_DUPLEX_MODE;
    }
  } else {
    //
    //  10 Mb/s
    //
    RgmiiControl |= RGMII_CTL_RGMII_RATE_2_5_MHZ
                  | RGMII_CTL_RGMII_MODE
                  | RGMII_CTL_CRS_SEL;
    if (LinkFullDuplex) {
      Mode |= MODE_DUPLEX_MODE;
    }
  }

  //
  //  Update the MAC register values
  //
  MacControl->Mode = Mode;
  MacControl->RgmiiControl = RgmiiControl;
}

/**
  Get the MAC address for the Ethernet controller

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[out] MacAddress  Address of a buffer to receive the MAC address

  @retval EFI_SUCCESS     The function completed successfully.
  @retval EFI_NOT_FOUND   The variable was not found.

**/
EFI_STATUS
EthernetGetMacAddress (
  IN ETHERNET_CONTEXT  *EthernetContext,
  OUT EFI_MAC_ADDRESS  *MacAddress
  )
{
  UINT32  *Buffer;

  Buffer = (UINT32 *)MacAddress;
  WriteUnaligned32 (Buffer + 0, PciIoMemRead32 (EthernetContext, ETH_REG_MAC_ADDRESS_1_A));
  WriteUnaligned32 (Buffer + 1, PciIoMemRead32 (EthernetContext, ETH_REG_MAC_ADDRESS_1_B));
  return EFI_SUCCESS;
}

/**
  Platform specific PHY configuration routines

  @param[in] EthernetContext  Pointer to an ETHERNET_CONTEXT structure
  @param[in] PhyAddress Address of the PHY

**/
VOID
PlatformSpecificPhyConfiguration (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress
 )
{
  UINT16 PhyData;

  //
  //  Add transmit clock delay
  //
  EthernetPhyWrite (EthernetContext,
                     PhyAddress,
                     0x1d,
                     5);
  PhyData = EthernetPhyRead (EthernetContext,
                              PhyAddress,
                              0x1e);
  PhyData |= 0x0100;
  EthernetPhyWrite (EthernetContext,
                     PhyAddress,
                     0x1e,
                     PhyData);

  //
  //  Disable hibernation mode, keep the PHY powered up
  //
  EthernetPhyWrite (EthernetContext,
                     PhyAddress,
                     0x1d,
                     0x0b);
  PhyData = EthernetPhyRead (EthernetContext,
                              PhyAddress,
                              0x1e);
  PhyData |= 0x8000;
  EthernetPhyWrite (EthernetContext,
                     PhyAddress,
                     0x1e,
                     PhyData);
}
