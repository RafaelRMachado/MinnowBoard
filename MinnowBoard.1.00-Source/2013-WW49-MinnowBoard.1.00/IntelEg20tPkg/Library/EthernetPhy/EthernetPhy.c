/** @file
  Implement the PHY support routines.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>

#include <IndustryStandard/Ieee802_3Phy.h>

#include <Library/DebugLib.h>
#include <Library/Eg20tGpioLib.h>
#include <Library/EthernetMac.h>
#include <Library/EthernetPhy.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/UefiLib.h>

#include <Register/Eg20t.h>

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
  Control &= ~( CONTROL_RESET
              | CONTROL_SPEED_SELECTION
              | CONTROL_POWER_DOWN
              | CONTROL_ISOLATE
              | CONTROL_DUPLEX_MODE );
  Control |= CONTROL_AUTO_NEGOTIATION_ENABLE
           | CONTROL_RESTART_AUTO_NEGOTIATION;
  AutoNegotiationAdvertisement &= ~( ANA_100BASE_TX_FD
                                   | ANA_100BASE_TX
                                   | ANA_10BASE_T_FD
                                   | ANA_10BASE_T );
  MasterSlaveControl &= ~( MSC_1000BASE_T_FD
                         | MSC_1000BASE_T );

  //
  //  Determine the connection properties
  //
  if ( 1000 == LinkSpeed ) {
    //
    //  1000 Mb/s
    //
    Control |= CONTROL_SPEED_1000_MBPS;
    MasterSlaveControl |= MSC_1000BASE_T;
    if ( LinkFullDuplex ) {
      MasterSlaveControl |= MSC_1000BASE_T_FD;
    }
    else {
      MasterSlaveControl |= MSC_1000BASE_T;
    }
  }
  else if ( 100 == LinkSpeed ) {
    //
    //  100 Mb/s
    //
    Control |= CONTROL_SPEED_100_MBPS;
    if ( LinkFullDuplex ) {
      AutoNegotiationAdvertisement |= ANA_100BASE_TX_FD;
    }
    else {
      AutoNegotiationAdvertisement |= ANA_100BASE_TX;
    }
  }
  else {
    //
    //  10 Mb/s
    //
    Control |= CONTROL_SPEED_10_MBPS;
    if ( LinkFullDuplex ) {
      AutoNegotiationAdvertisement |= ANA_10BASE_T_FD;
    }
    else {
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
  if (( 0 != ( Status & STATUS_AUTO_NEGOTIATION_COMPLETE ))
    && ( 0 != ( MasterSlaveStatus & MSS_STATUS_LP_1000 ))) {
    //
    //  1000 Mb/s
    ///
    Speed = 1000;
    if ( 0 != ( MasterSlaveStatus & MSS_STATUS_LP_1000_FD )) {
      FullDuplex = TRUE;
    }
  }
  else if (( 0 != ( Status & STATUS_AUTO_NEGOTIATION_COMPLETE ))
          && ( 0 != ( AutoNegotiationStatus & ( ANA_100BASE_TX | ANA_100BASE_TX_FD )))) {
    //
    //  100 Mb/s
    //
    Speed = 100;
    if ( 0 != ( AutoNegotiationStatus & ANA_100BASE_TX_FD )) {
      FullDuplex = TRUE;
    }
  }
  else {
    //
    //  10 Mb/s
    //
    Speed = 10;
    if ( 0 != ( AutoNegotiationStatus & ANA_10BASE_T_FD )) {
      FullDuplex = TRUE;
    }
  }

  //
  //  Return the connection properties
  //
  *LinkUp = (BOOLEAN)( 0 != ( Status & STATUS_LINK_UP ));
  *LinkSpeed = Speed;
  *LinkFullDuplex = FullDuplex;
}


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
  )
{
  UINT32 Data;
  INT32 Index;
  UINT16 PhyData;

  //
  //  Loop through the possible PHY addresses
  //
  Data = 0;
  for ( Index = 31; 0 <= Index; Index-- ) {
    //
    //  Read the ID value from the PHY
    //
    PhyData = EthernetPhyRead ( PciIo,
                                Index,
                                PHY_PHY_IDENTIFIER_1 );
    Data = ((UINT32)PhyData ) << 16;
    PhyData = EthernetPhyRead ( PciIo,
                                Index,
                                PHY_PHY_IDENTIFIER_2 );
    Data |= PhyData;
    if (( 0 != Data ) && ( 0xffffffff != Data )) {
      break;
    }
  }

  //
  //  Return the OID, model and revision
  //
  if ( NULL != PhyId ) {
    *PhyId = Data;
  }

  //
  //  Return the PHY address
  //
  return (UINTN)(UINT32)Index;
}


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
  )
{
  UINT32 Data;
  UINT16 PhyData;
  EFI_STATUS Status;

  //
  //  Make sure all of the setup is complete
  //
  MemoryFence ( );

  //
  //  Read the PHY register
  //
  Data = (( PhyAddress << 21 ) & MIIM_PHY_ADDR )
       | (( PhyRegister << 16 ) & MIIM_REG_ADDR )
       | MIIM_OPER_READ;
  Status = EthWrite32 ( PciIo, ETH_REG_MIIM, &Data );
  MemoryFence ( );
  ASSERT ( EFI_SUCCESS == Status );

  //
  //  Wait for the read to complete
  //
  do {
    Status = EthRead32 ( PciIo, ETH_REG_MIIM, &Data );
    ASSERT ( EFI_SUCCESS == Status );
  } while ( 0 == ( Data & MIIM_OPER ));

  //
  //  Get the PHY data
  //
  PhyData = (UINT16)Data;
  return PhyData;
}


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
  IN BOOLEAN ResetActive
  )
{
  //
  //  Reset the PHY
  //
  EthernetPhyWrite ( PciIo,
                     PhyAddress,
                     PHY_CONTROL,
                     ResetActive ? CONTROL_RESET : 0 );
}


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
  )
{
  UINT32 Data;

  //
  //  Make sure all of the setup is complete
  //
  MemoryFence ( );

  //
  //  Wait for the previous operation to complete
  //
  do {
    EthRead32 ( PciIo, ETH_REG_MIIM, &Data );
  } while ( 0 == ( Data & MIIM_OPER ));

  //
  //  Write the PHY register
  //
  Data = (( PhyAddress << 21 ) & MIIM_PHY_ADDR )
       | (( PhyRegister << 16 ) & MIIM_REG_ADDR )
       | MIIM_OPER_WRITE
       | ( Value & MIIM_DATA );
  EthWrite32 ( PciIo, ETH_REG_MIIM, &Data );
  MemoryFence ( );

  //
  //  Wait for the write to complete
  //
  do {
    EthRead32 ( PciIo, ETH_REG_MIIM, &Data );
  } while ( 0 == ( Data & MIIM_OPER ));
}
