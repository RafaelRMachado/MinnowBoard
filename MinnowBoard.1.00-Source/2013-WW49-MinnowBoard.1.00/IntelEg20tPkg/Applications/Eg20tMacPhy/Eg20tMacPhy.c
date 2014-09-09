/** @file
  Manipulate the Ethernet PHY using the EG20T.
  
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
#include <IndustryStandard/Pci.h>

#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>
#include <Library/EthernetAccess.h>
#include <Library/EthernetMac.h>
#include <Library/EthernetPhy.h>
#include <Library/IoLib.h>
#include <Library/PcdLib.h>
#include <Library/PhyConfigLib.h>
#include <Library/PhyResetLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>

#include <Protocol/PciIo.h>

#include <Register/Eg20t.h>

#define DIM(x)                    ( sizeof ( x ) / sizeof ( x[0] ))
#define AUTO_NEGOTIATION_TIMEOUT  5
#define TPL_ETHERNET              TPL_NOTIFY
#define TIME_MICROSECOND          10ULL
#define TIME_MILLISECOND          ( 1000 * TIME_MICROSECOND )
#define TIME_SECOND               ( 1000 * TIME_MILLISECOND )
#define TIME_MINUTE               ( 60 * TIME_SECOND )
#define ETHERNET_TIMER_TICK       ( 10 * TIME_MILLISECOND )
#define LINK_TIMEOUT              (( 5 * TIME_SECOND ) / ETHERNET_TIMER_TICK )
#define TEST_DURATION             ( 2 * TIME_MINUTE )
#define PHY_REGISTER_COUNT        (( MIIM_REG_ADDR >> 16 ) + 1 )
//#define PHY_REGISTER_COUNT        16
#define PHY_RESET_DURATION        PcdGet32 ( PcdEthernetPhyResetDurationMsec )
#define PHY_RESET_RECOVERY        PcdGet32 ( PcdEthernetPhyResetRecoveryMsec )
#define GPIO_SUS                  1
#define LED_1                     1

#define RX_BUFFERS                64
#define RX_BUFFER_PAGES           (( RX_BUFFERS * ETH_RECEIVE_BUFFER_SIZE ) / SIZE_4KB )
#define RX_DESCRIPTOR_AREA_SIZE   ( RX_BUFFERS * sizeof ( ETH_RECEIVE_DESCRIPTOR ))
#define RX_DESCRIPTOR_PAGES       (( RX_DESCRIPTOR_AREA_SIZE + SIZE_4KB - 1 ) / SIZE_4KB )

#define EthAnd32(PciIo, Offset,Value)                       \
            {                                       \
              UINT32 Data;                          \
                                                    \
              EthPciRead32 ( PciIo, Offset, &Data );           \
              Data &= Value;                        \
              EthPciWrite32 ( PciIo, Offset, &Data );          \
            }
#define EthOr32(PciIo,Offset,Value)                 \
            {                                       \
              UINT32 Data;                          \
                                                    \
              EthPciRead32 ( PciIo, Offset, &Data );    \
              Data |= Value;                        \
              EthPciWrite32 ( PciIo, Offset, &Data );          \
            }

#define EthPciRead32            EthRead32
#define EthPciWrite32           EthWrite32

typedef enum _LINK_STATE {
  LINK_STATE_UNKNOWN = 0,
  LINK_STATE_DOWN,
  LINK_STATE_RESETTING,
  LINK_STATE_RESET,
  LINK_STATE_CONFIGURE_PHY,
  LINK_STATE_AUTO_NEGOTIATING,
  LINK_STATE_UP
} LINK_STATE;

typedef struct {
  UINTN LinkState;
  INTN LinkTimeout;
  EFI_PCI_IO_PROTOCOL * PciIo;
  UINTN ResetDuration;
  UINTN ResetRecovery;
  UINTN PhyAddress;
  UINT32 PhyId;
  EFI_PHYSICAL_ADDRESS PhysicalReceiveBuffer;
  EFI_PHYSICAL_ADDRESS PhysicalReceiveDescriptors;
  VOID * DmaMapping;
  UINT8 * ReceiveBuffer;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptors;
  UINT32 ReceiveIndex;
  UINT32 ReceiveIndexMask;
  UINTN Speed;
  BOOLEAN FullDuplex;
  EFI_EVENT Timer;
} ETHERNET_CONTEXT;



UINTN
Str2Uint(
  CHAR16 *pString
  )
{
  UINTN Value;

  if (( '0' == pString[0])
    && (( 'x' == pString[1]) || ( 'X' == pString[1]))) {
    Value = StrHexToUintn ( pString );
  }
  else {
    Value = StrDecimalToUintn ( pString );
  }
  return Value;
}


VOID
DisableController (
  EFI_PCI_IO_PROTOCOL *PciIo
  )
{
  UINT32 Command;
  UINT32 Mask;
  EFI_STATUS Status;

  //
  //  Get the PCI controller status
  //
  Status = PciIo->Pci.Read ( PciIo,
                             EfiPciIoWidthUint32,
                             PCI_COMMAND_OFFSET,
                             1,
                             &Command );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Disable the PCI controller
    //
    Mask = EFI_PCI_COMMAND_MEMORY_SPACE
         | EFI_PCI_COMMAND_BUS_MASTER
         | PCI_COMMAND_INTERRUPT_DISABLE;
    if ( PCI_COMMAND_INTERRUPT_DISABLE != ( Command & Mask )) {
      Command &= ~( EFI_PCI_COMMAND_MEMORY_SPACE | EFI_PCI_COMMAND_BUS_MASTER );
      Command |= PCI_COMMAND_INTERRUPT_DISABLE;
      Status = PciIo->Pci.Write ( PciIo,
                                  EfiPciIoWidthUint32,
                                  PCI_COMMAND_OFFSET,
                                  1,
                                  &Command );
    }
  }
}


EFI_STATUS
EnableController (
  EFI_PCI_IO_PROTOCOL *PciIo,
  ETHERNET_CONTEXT * EthernetContext,
  UINTN ClockFrequency
  )
{
  UINT32 Command;
  UINT32 Data;
  EFI_MAC_ADDRESS MacAddress;
  UINT32 Mask;
  EFI_STATUS Status;

  PciIo = EthernetContext->PciIo;
  Status = PciIo->Pci.Read ( PciIo,
                             EfiPciIoWidthUint32,
                             PCI_COMMAND_OFFSET,
                             1,
                             &Command );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Enable the PCI controller
    //
    Mask = EFI_PCI_COMMAND_MEMORY_SPACE
         | EFI_PCI_COMMAND_BUS_MASTER
         | PCI_COMMAND_INTERRUPT_DISABLE;
    if ( Mask != ( Command & Mask )) {
      Command |= Mask;
      Status = PciIo->Pci.Write ( PciIo,
                                  EfiPciIoWidthUint32,
                                  PCI_COMMAND_OFFSET,
                                  1,
                                  &Command );
    }
    if ( !EFI_ERROR ( Status )) {
      //
      //  Configure the clock to the PHY
      //
      if ( 125000000 == ClockFrequency ) {
        Print ( L"EG20T provides 125 MHz clock to PHY\r\n" );
        Data = RGMII_CTL_RGMII_RATE_125_MHZ | RGMII_CTL_RGMII_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_RGMII_CONTROL, &Data );
        Data = MODE_ETHER_MODE | MODE_DUPLEX_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_MODE, &Data );
      }
      else if ( 25000000 == ClockFrequency ) {
        Print ( L"EG20T provides 25 MHz clock to PHY\r\n" );
        Data = RGMII_CTL_RGMII_RATE_25_MHZ | RGMII_CTL_RGMII_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_RGMII_CONTROL, &Data );
        Data = MODE_DUPLEX_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_MODE, &Data );
      }
      else if ( 2500000 == ClockFrequency ) {
        Print ( L"EG20T provides 2.5 MHz clock to PHY\r\n" );
        Data = RGMII_CTL_RGMII_RATE_2_5_MHZ | RGMII_CTL_RGMII_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_RGMII_CONTROL, &Data );
        Data = MODE_DUPLEX_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_MODE, &Data );
      }

      //
      //  Display the MAC address
      //
      Status = EthernetGetMacAddress ( PciIo, &MacAddress );
      if ( EFI_ERROR ( Status )) {
        Print ( L"ERROR - MAC address not found, Status: %r\r\n", Status );
      }
      else {
        Print ( L"MAC Address: %02x-%02x-%02x-%02x-%02x-%02x\r\n",
                MacAddress.Addr [ 0 ],
                MacAddress.Addr [ 1 ],
                MacAddress.Addr [ 2 ],
                MacAddress.Addr [ 3 ],
                MacAddress.Addr [ 4 ],
                MacAddress.Addr [ 5 ]);

        //
        //  Locate the PHY
        //
        EthernetContext->PhyAddress = EthernetPhyGetAddress ( PciIo,
                                                              &EthernetContext->PhyId );
      }
    }
  }

  //
  //  Return the operation status
  //
  return Status;
}


VOID
PhyConfigure (
  ETHERNET_CONTEXT * EthernetContext,
  UINT16 * PhyData
  )
{
  MAC_CONTROL_REGISTERS MacControl;
  EFI_PCI_IO_PROTOCOL * PciIo;
  PHY_CONTROL_REGISTERS PhyControl;

  //
  //  Read the MAC control registers
  //
  PciIo = EthernetContext->PciIo;
  EthPciRead32 ( PciIo, ETH_REG_MODE, &MacControl.Mode );
  EthPciRead32 ( PciIo, ETH_REG_RGMII_CONTROL, &MacControl.RgmiiControl );

  //
  //  Display the MAC registers
  //
  Print ( L"MAC: 0x%08x --> 0x%08x\r\n",
          ETH_REG_MODE,
          MacControl.Mode );
  Print ( L"MAC: 0x%08x --> 0x%08x\r\n",
          ETH_REG_RGMII_CONTROL,
          MacControl.RgmiiControl );

  //
  //  Read the PHY control registers
  //
  PhyControl.Control = PhyData [ PHY_CONTROL ];
  PhyControl.AutoNegotiationAdvertisement = PhyData [ PHY_AUTO_NEGOTIATION_ADVERTISEMENT ];
  PhyControl.MasterSlaveControl = PhyData [ PHY_MASTER_SLAVE_CONTROL ];

  //
  //  Update the registers
  //
  EthernetMacConfigure ( &MacControl,
                         EthernetContext->Speed,
                         EthernetContext->FullDuplex );
  EthernetPhyConfigure ( &PhyControl,
                         EthernetContext->Speed,
                         EthernetContext->FullDuplex );

  //
  //  Display the registers
  //
  Print ( L"MAC: 0x%08x <-- 0x%08x\r\n",
          ETH_REG_MODE,
          MacControl.Mode );
  Print ( L"MAC: 0x%08x <-- 0x%08x\r\n",
          ETH_REG_RGMII_CONTROL,
          MacControl.RgmiiControl );
  Print ( L"PHY: %02d <-- 0x%04x\r\n",
          PHY_CONTROL,
          PhyControl.Control );
  Print ( L"PHY: %02d <-- 0x%04x\r\n",
          PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
          PhyControl.AutoNegotiationAdvertisement );
  Print ( L"PHY: %02d <-- 0x%04x\r\n",
          PHY_MASTER_SLAVE_CONTROL,
          PhyControl.MasterSlaveControl );

  //
  //  Configure the link
  //
  EthPciWrite32 ( PciIo, ETH_REG_MODE, &MacControl.Mode );
  EthPciWrite32 ( PciIo, ETH_REG_RGMII_CONTROL, &MacControl.RgmiiControl );
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
                     PhyControl.AutoNegotiationAdvertisement );
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_MASTER_SLAVE_CONTROL,
                     PhyControl.MasterSlaveControl );
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_CONTROL,
                     PhyControl.Control );
}


EFI_STATUS
LocateController (
  EFI_PCI_IO_PROTOCOL **PciIo,
  ETHERNET_CONTEXT * EthernetContext
  )
{
  EFI_HANDLE * HandleArray;
  UINTN HandleCount;
  UINTN Index;
  PCI_TYPE00 Pci;
  EFI_STATUS Status;

  //
  //  Locate the PCI devices
  //
  Status = gBS->LocateHandleBuffer ( ByProtocol,
                                     &gEfiPciIoProtocolGuid,
                                     NULL,
                                     &HandleCount,
                                     &HandleArray );
  if ( !EFI_ERROR ( Status )) {
    //
    //  Walk the list of PCI devices
    //
    Status = EFI_NOT_FOUND;
    for ( Index = 0; HandleCount > Index; Index++ ) {
      //
      //  Get the PCI I/O protocol
      //
      Status = gBS->OpenProtocol ( HandleArray [ Index ],
                                   &gEfiPciIoProtocolGuid,
                                   (VOID **)PciIo,
                                   NULL,
                                   NULL,
                                   EFI_OPEN_PROTOCOL_GET_PROTOCOL );
      if ( !EFI_ERROR ( Status )) {
        //
        //  Found a PCI device
        //  Read the device type
        //
        Status = (*PciIo)->Pci.Read ( *PciIo,
                                      EfiPciIoWidthUint32,
                                      PCI_VENDOR_ID_OFFSET,
                                      sizeof (Pci) / sizeof (UINT32),
                                      &Pci );
        if ( !EFI_ERROR ( Status )) {
          //
          //  Determine if this device requires the Ethernet driver
          //
          if (( EG20T_PCI_VENDOR_ID == Pci.Hdr.VendorId )
            && ( EG20T_ETHERNET_PCI_DEVICE_ID == Pci.Hdr.DeviceId )) {
            //
            //  Return the controller address
            //
            Print ( L"EthernetController Found\r\n" );
            EthernetContext->PciIo = *PciIo;
            Status = EFI_SUCCESS;
            break;
          }
          Status = EFI_NOT_FOUND;
        }
      }
    }

    //
    //  Done with the list of handles
    //
    gBS->FreePool ( HandleArray );
  }

  //
  //  Return the status
  //
  return Status;
}


VOID
EthernetMacReset (
  IN ETHERNET_CONTEXT * EthernetContext
  )
{
  UINT32 Data;
  EFI_PCI_IO_PROTOCOL * PciIo;

  //
  //  Disable the receiver and transmitter
  //
  Data = 0;
  PciIo = EthernetContext->PciIo;
  EthPciWrite32 ( PciIo, ETH_REG_DMA_CONTROL, &Data );
  EthPciWrite32 ( PciIo, ETH_REG_MAC_RX_ENABLE, &Data );
  MemoryFence ( );

  //
  //  Reset the MAC
  //
  Data = ETH_SOFT_RESET;
  EthPciWrite32 ( PciIo, ETH_REG_SOFT_RESET, &Data );
  MemoryFence ( );

  //
  //  Update the context
  //
  EthernetContext->ReceiveIndexMask = RX_BUFFERS - 1;
  EthernetContext->ReceiveIndex = EthernetContext->ReceiveIndexMask;
}


VOID
PhyRegisterDecode (
  UINT16 * PhyData
  )
{
  BOOLEAN LinkFullDuplex;
  UINTN LinkSpeed;
  BOOLEAN LinkUp;
  PHY_STATUS_REGISTERS PhyStatus;
  
  //
  //  Decode the PHY data
  //
  PhyStatus.Status = PhyData [ PHY_STATUS ];
  PhyStatus.AutoNegotiationStatus = PhyData [ PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY ];
  PhyStatus.MasterSlaveStatus = PhyData [ PHY_MASTER_SLAVE_STATUS ];
  EthernetPhyDecode ( &PhyStatus, &LinkUp, &LinkSpeed, &LinkFullDuplex );
  
  if ( !LinkUp ) {
    Print ( L"Link Down\r\n" );
  }
  else {
    Print ( L"Link Up, %d Mb/s %s Duplex\r\n", LinkSpeed, LinkFullDuplex ? L"Full" : L"Half" );
  }
}


VOID
PhyRegisterDisplay (
  ETHERNET_CONTEXT * EthernetContext,
  UINT16 * PhyData
  )
{
  UINTN PhyRegister;

  //
  //  Display the PHY registers
  //
  Print ( L"PHY %d Registers\r\n", EthernetContext->PhyAddress );
  for ( PhyRegister = 0; PHY_REGISTER_COUNT > PhyRegister; PhyRegister++ ) {
    Print ( L"    0x%02x: 0x%04x\r\n", PhyRegister, PhyData [ PhyRegister ]);
  }
}


VOID
PhyRegisterRead (
  ETHERNET_CONTEXT * EthernetContext,
  UINT16 * PhyData
  )
{
  EFI_PCI_IO_PROTOCOL * PciIo;
  UINTN PhyRegister;

  //
  //  Display the PHY registers
  //
  PciIo = EthernetContext->PciIo;
  for ( PhyRegister = 0; PHY_REGISTER_COUNT > PhyRegister; PhyRegister++ ) {
    PhyData [ PhyRegister ] = EthernetPhyRead ( PciIo,
                                                EthernetContext->PhyAddress,
                                                PhyRegister );
  }
}


VOID
PhyReset (
  ETHERNET_CONTEXT * EthernetContext
  )
{
  EFI_PCI_IO_PROTOCOL * PciIo;

  //
  //  Reset the PHY
  //
  PciIo = EthernetContext->PciIo;
  EthernetPhyReset ( PciIo,
                     EthernetContext->PhyAddress,
                     TRUE );
  gBS->Stall ( EthernetContext->ResetDuration );
  EthernetPhyReset ( PciIo,
                     EthernetContext->PhyAddress,
                     FALSE );
  gBS->Stall ( EthernetContext->ResetRecovery );

  //
  //  Initialize the PHY
  //
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_CONTROL,
                     CONTROL_SPEED_1000_MBPS
                     | CONTROL_DUPLEX_MODE
                   );
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
                     ANA_100BASE_TX_FD
                     | ANA_100BASE_TX
                     | ANA_10BASE_T );
  EthernetPhyWrite ( PciIo,
                     EthernetContext->PhyAddress,
                     PHY_CONTROL,
                     CONTROL_SPEED_1000_MBPS
                     | CONTROL_DUPLEX_MODE
                     | CONTROL_AUTO_NEGOTIATION_ENABLE
                     | CONTROL_RESTART_AUTO_NEGOTIATION
                   );
}


VOID
PhyResetChip (
  ETHERNET_CONTEXT * EthernetContext
  )
{
  EthernetPhyResetChip ( TRUE );
  gBS->Stall ( EthernetContext->ResetDuration );
  EthernetPhyResetChip ( FALSE );
  gBS->Stall ( EthernetContext->ResetRecovery );
}


/**
  Periodic timer interrupt routine

  @param  Event                 Event whose notification function is being invoked.
  @param  Context               The pointer to the notification function's context,
                                which is implementation-dependent.

**/
VOID
EFIAPI
EthernetTimer (
  IN  EFI_EVENT Event,
  IN  ETHERNET_CONTEXT * EthernetContext
  )
{
  UINT8 * Buffer;
  UINT16 Data16;
  UINT32 Data;
  UINTN Index;
  UINTN IndexEnd;
  UINTN IndexMask;
  UINT32 InterruptStatus;
  LINK_STATE LinkState;
  EFI_PCI_IO_PROTOCOL * PciIo;
  UINTN PhyAddress;
  UINT16 PhyData [ PHY_REGISTER_COUNT ];
  EFI_PHYSICAL_ADDRESS PhysicalBuffer;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptor;
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptorEnd;

  //
  //  Update the state of the link
  //
  PciIo = EthernetContext->PciIo;
  LinkState = EthernetContext->LinkState;
  PhyAddress = EthernetContext->PhyAddress;
  switch ( LinkState ) {
  default:
  case LINK_STATE_UNKNOWN:
    Print ( L"Link down\r\n" );
    LinkState = LINK_STATE_DOWN;

    //
    //  Fall through
    //      |
    //      |
    //      V

  case LINK_STATE_DOWN:
    //
    //  Reset the PHY
    //
    EthernetPhyResetChip ( TRUE );
    //
    //  Reset the MAC
    //
    EthernetMacReset ( EthernetContext );
    Data = (UINT32)( DivU64x32 ((( PcdGet32 ( PcdEthernetPhyResetDurationMsec )
                                              * 1000 )
                                              + ETHERNET_TIMER_TICK - 1 ),
                                   ETHERNET_TIMER_TICK )
                                   + 1 );
    EthernetContext->LinkTimeout = Data;
    LinkState = LINK_STATE_RESETTING;
    break;

  case LINK_STATE_RESETTING:
    //
    //  Generate a wide enough reset pulse
    //
    if ( Event == EthernetContext->Timer ) {
      EthernetContext->LinkTimeout--;
      if ( 0 >= EthernetContext->LinkTimeout ) {
        //
        //  Allow the MAC to recover from the reset
        //
        Data = 0;
        EthPciWrite32 ( PciIo, ETH_REG_SOFT_RESET, &Data );
        MemoryFence ( );

        //
        //  Allow the PHY to recover from the reset
        //
        EthernetPhyResetChip ( FALSE );
        Data = (UINT32)( DivU64x32 ((( PcdGet32 ( PcdEthernetPhyResetRecoveryMsec )
                                                * 1000 )
                                                + ETHERNET_TIMER_TICK - 1 ),
                                       ETHERNET_TIMER_TICK )
                                       + 1 );
        EthernetContext->LinkTimeout = Data;
        LinkState = LINK_STATE_RESET;
      }
    }
    break;

  case LINK_STATE_RESET:
    //
    //  Ensure enough recovery time after reset
    //
    if ( Event == EthernetContext->Timer ) {
      EthernetContext->LinkTimeout--;
      if ( 0 >= EthernetContext->LinkTimeout ) {
        //
        //  Configure the MII
        //
        Data = RGMII_CTL_RGMII_RATE_125_MHZ | RGMII_CTL_RGMII_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_RGMII_CONTROL, &Data );
        Data = MODE_ETHER_MODE | MODE_DUPLEX_MODE;
        EthPciWrite32 ( PciIo, ETH_REG_MODE, &Data );

        //
        //  Display the PHY registers
        //
        Data16 = EthernetPhyRead ( PciIo,
                                   EthernetContext->PhyAddress,
                                   PHY_CONTROL );
        Print ( L"PHY: %02d --> 0x%04x\r\n", PHY_CONTROL, Data16 );
        Data16 = EthernetPhyRead ( PciIo,
                                   EthernetContext->PhyAddress,
                                   PHY_AUTO_NEGOTIATION_ADVERTISEMENT );
        Print ( L"PHY: %02d --> 0x%04x\r\n", PHY_AUTO_NEGOTIATION_ADVERTISEMENT, Data16 );
        Data16 = EthernetPhyRead ( PciIo,
                                   EthernetContext->PhyAddress,
                                   PHY_MASTER_SLAVE_CONTROL );
        Print ( L"PHY: %02d --> 0x%04x\r\n", PHY_MASTER_SLAVE_CONTROL, Data16 );

        //
        //  Configure the PHY and start the auto-negotiation
        //
        Data16 = (UINT16)( ANA_100BASE_TX_FD
                         | ANA_100BASE_TX
                         | ANA_10BASE_T_FD
                         | ANA_10BASE_T
                         | ANA_SELECTOR_IEEE_802_3 );
        Print ( L"PHY: %02d <-- 0x%04x\r\n", PHY_AUTO_NEGOTIATION_ADVERTISEMENT, Data16 );
        EthernetPhyWrite ( PciIo,
                           EthernetContext->PhyAddress,
                           PHY_AUTO_NEGOTIATION_ADVERTISEMENT,
                           Data16 );
        Data16 = (UINT16)( MSC_MANUAL_CONFIGURATION_ENABLE
                         | MSC_1000BASE_T_FD
                         | MSC_1000BASE_T );
        Print ( L"PHY: %02d <-- 0x%04x\r\n", PHY_MASTER_SLAVE_CONTROL, Data16 );
        EthernetPhyWrite ( PciIo,
                           EthernetContext->PhyAddress,
                           PHY_MASTER_SLAVE_CONTROL,
                           Data16 );
        Data16 = (UINT16)( CONTROL_SPEED_1000_MBPS
                         | CONTROL_AUTO_NEGOTIATION_ENABLE
                         | CONTROL_RESTART_AUTO_NEGOTIATION
                         | CONTROL_DUPLEX_MODE );
        Print ( L"PHY: %02d <-- 0x%04x\r\n", PHY_CONTROL, Data16 );
        EthernetPhyWrite ( PciIo,
                           EthernetContext->PhyAddress,
                           PHY_CONTROL,
                           Data16 );

        //
        //  Initialize the receive descriptors
        //
        ReceiveDescriptor = EthernetContext->ReceiveDescriptors;
        ZeroMem ( ReceiveDescriptor, RX_DESCRIPTOR_PAGES * SIZE_4KB );
        PhysicalBuffer = EthernetContext->PhysicalReceiveBuffer;
        for ( Index = 0; RX_BUFFERS > Index; Index++ )
        {
          ReceiveDescriptor->RxFrameBufferAddress = (UINT32)(UINTN)PhysicalBuffer;
          PhysicalBuffer += ETH_RECEIVE_BUFFER_SIZE;
        }

        //
        //  Initialize the receive DMA engine
        //
        ASSERT (( 0x10 <= RX_DESCRIPTOR_AREA_SIZE ) && ( 0x10000 >= RX_DESCRIPTOR_AREA_SIZE ));
        Data = RX_DESCRIPTOR_AREA_SIZE - 0x10;
        EthPciWrite32 ( PciIo, ETH_REG_RX_DESCR_SIZE, &Data );
        Data = (UINT32)(UINTN)EthernetContext->ReceiveDescriptors;
        EthPciWrite32 ( PciIo, ETH_REG_RX_DESCR_BASE_ADDRESS, &Data );
        EthPciWrite32 ( PciIo, ETH_REG_RX_DESCR_HARD_POINTER, &Data );
        Data = (UINT32)(UINTN)&EthernetContext->ReceiveDescriptors [ EthernetContext->ReceiveIndex ];
        EthPciWrite32 ( PciIo, ETH_REG_RX_DESCR_SOFT_POINTER, &Data );

        //
        //  Configure the receiver to receive all frames
        //
        Data = 0;
        EthPciWrite32 ( PciIo, ETH_REG_RX_MODE, &Data );
        Data = MAC_RX_ENABLE;
        EthPciWrite32 ( PciIo, ETH_REG_MAC_RX_ENABLE, &Data );
        MemoryFence ( );

        //
        //  Wait for the auto-negotation to complete
        EthernetContext->LinkTimeout = LINK_TIMEOUT;
        LinkState = LINK_STATE_CONFIGURE_PHY;
      }
    }
    break;

  case LINK_STATE_CONFIGURE_PHY:
    //
    //  Determine if the link is up
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead ( PciIo,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS );
    if ( 0 != ( PhyData [ PHY_STATUS ] & STATUS_LINK_UP )) {
      Print ( L"Configuring PHY\r\n" );
      LinkState = LINK_STATE_UP;
      PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);

      //
      //  Configure the PHY
      //
      PhyRegisterDecode ( &PhyData [ 0 ]);
      PhyConfigure ( EthernetContext, &PhyData [ 0 ]);
      PlatformSpecificPhyConfiguration ( PciIo,
                                         EthernetContext->PhyAddress );

      //
      //  Wait for the auto-negotiation to complete
      //
      EthernetContext->LinkTimeout = LINK_TIMEOUT;
      LinkState = LINK_STATE_AUTO_NEGOTIATING;
      break;
    }

    //
    //  Determine if the link has timed out
    //
    else if ( 0 >= EthernetContext->LinkTimeout-- ) {
      Print ( L"Link down: Configuring\r\n" );
      PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);
      LinkState = LINK_STATE_DOWN;
    }
    break;

  case LINK_STATE_AUTO_NEGOTIATING:
    //
    //  Determine if the link is up
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead ( PciIo,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS );
    if ( 0 != ( PhyData [ PHY_STATUS ] & STATUS_LINK_UP )) {
      Print ( L"Link up\r\n" );
      LinkState = LINK_STATE_UP;
      PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDecode ( &PhyData [ 0 ]);

      //
      //  Enable the receiver
      //
      MemoryFence ( );
      Data = ETH_DMA_CONTROL_RX_DMA_EN;
      EthPciWrite32 ( PciIo, ETH_REG_DMA_CONTROL, &Data );
      MemoryFence ( );
      Data = MAC_RX_ENABLE;
      EthPciWrite32 ( PciIo, ETH_REG_MAC_RX_ENABLE, &Data );
      MemoryFence ( );
    }

    //
    //  Determine if the link has timed out
    //
    else if ( 0 >= EthernetContext->LinkTimeout-- ) {
      LinkState = LINK_STATE_DOWN;
      Print ( L"Link down: Auto-Negotiating\r\n" );
      PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);
    }
    break;

  case LINK_STATE_UP:
    //
    //  Monitor the link status
    //
    PhyData [ PHY_STATUS ] = EthernetPhyRead ( PciIo,
                                               EthernetContext->PhyAddress,
                                               PHY_STATUS );
    if ( 0 == ( PhyData [ PHY_STATUS ] & STATUS_LINK_UP )) {
      LinkState = LINK_STATE_DOWN;
      Print ( L"Link down\r\n" );
      PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
      PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);
    }
    else {
      //
      //  Receive any frames
      //
      EthPciRead32 ( PciIo, ETH_REG_INTERRUPT_STATUS, &InterruptStatus );
      if ( 0 != ( InterruptStatus & ETH_INT_RX_DMA_CMPLT )) {
        //
        //  Free the receive frames
        //
        IndexMask = EthernetContext->ReceiveIndexMask;
        EthPciRead32 ( PciIo, ETH_REG_RX_DESCR_HARD_POINTER_HOLD, &Data );
        ReceiveDescriptorEnd = (ETH_RECEIVE_DESCRIPTOR *)Data;
        IndexEnd = ReceiveDescriptorEnd - EthernetContext->ReceiveDescriptors;
        Index = EthernetContext->ReceiveIndex;
        while ( IndexEnd != (( Index + 1 ) & IndexMask )) {
          Index = ( Index + 1 ) & IndexMask;
          ReceiveDescriptor = &EthernetContext->ReceiveDescriptors [ Index ];
          if (( 0 == ( ReceiveDescriptor->GmacStatus & ( RX_GMAC_STATUS_CRC_ERR
                                                       | RX_GMAC_STATUS_NBL_ERR
                                                       | RX_GMAC_STATUS_NOT_OCTAL )))
            && ( 0 != ReceiveDescriptor->RxLength )) {
            //
            //  Get the received packet
            //
            PhysicalBuffer = ReceiveDescriptor->RxFrameBufferAddress;
            Buffer = (UINT32)( PhysicalBuffer - EthernetContext->PhysicalReceiveBuffer )
                   + EthernetContext->ReceiveBuffer;
Print ( L"RX: %02x-%02x-%02x-%02x-%02x-%02x <-- %02x-%02x-%02x-%02x-%02x-%02x, %d bytes\r\n",
Buffer [ 0 ],
Buffer [ 1 ],
Buffer [ 2 ],
Buffer [ 3 ],
Buffer [ 4 ],
Buffer [ 5 ],
Buffer [ 6 ],
Buffer [ 7 ],
Buffer [ 8 ],
Buffer [ 9 ],
Buffer [ 10 ],
Buffer [ 11 ],
ETH_RX_BYTES ( ReceiveDescriptor ));
          }

          //
          //  Hand this receive buffer back to the MAC
          //
          Data = (UINT32)(UINTN)&EthernetContext->ReceiveDescriptors [ Index ];
          EthPciWrite32 ( PciIo, ETH_REG_RX_DESCR_SOFT_POINTER, &Data );
          MemoryFence ( );
        }

        //
        //  Update the index
        //
        EthernetContext->ReceiveIndex = Index;
      }
    }
    break;
  }

  //
  //  Update the link state
  //
  EthernetContext->LinkState = LinkState;
}


EFI_STATUS
LinkMonitor (
  ETHERNET_CONTEXT * EthernetContext
  )
{
  UINT8 * Buffer;
  UINTN BufferSize;
  UINTN Index;
  UINTN PageCount;
  EFI_PHYSICAL_ADDRESS PhysicalBuffer;
  EFI_PCI_IO_PROTOCOL * PciIo;
  EFI_STATUS Status;
  UINT64 TestTime;
  EFI_EVENT TestTimer;
  UINT64 TimerDuration;
  EFI_EVENT TimerEvent;

  //
  //  Create the events
  //
  PciIo = EthernetContext->PciIo;
  Status = gBS->CreateEvent ( EVT_TIMER,
                              0,
                              NULL,
                              NULL,
                              &TestTimer );
  if ( !EFI_ERROR ( Status )) {
    Status = gBS->CreateEvent ( EVT_TIMER | EVT_NOTIFY_SIGNAL,
                                TPL_ETHERNET,
                                (EFI_EVENT_NOTIFY)&EthernetTimer,
                                (VOID *)EthernetContext,
                                &TimerEvent );
    if ( !EFI_ERROR ( Status )) {
      EthernetContext->Timer = TimerEvent;

      //
      //  Allocate the receive buffer
      //
      PageCount = RX_DESCRIPTOR_PAGES + RX_BUFFER_PAGES;
      Status = PciIo->AllocateBuffer ( PciIo,
                                       0,
                                       EfiBootServicesData,
                                       PageCount,
                                       (VOID**)&Buffer,
                                       0 );
      if ( !EFI_ERROR ( Status )) {
        BufferSize = PageCount * SIZE_4KB;
        Status = PciIo->Map ( PciIo,
                              EfiPciIoOperationBusMasterCommonBuffer,
                              Buffer,
                              &BufferSize,
                              &EthernetContext->PhysicalReceiveDescriptors,
                              &EthernetContext->DmaMapping );
        if ( !EFI_ERROR ( Status )) {
          if ( BufferSize == ( PageCount * SIZE_4KB )) {
            //
            //  Split the buffer
            //
            PhysicalBuffer = EthernetContext->PhysicalReceiveDescriptors;
            EthernetContext->ReceiveDescriptors = (ETH_RECEIVE_DESCRIPTOR *)Buffer;
            Buffer += RX_DESCRIPTOR_PAGES * SIZE_4KB;
            PhysicalBuffer += RX_DESCRIPTOR_PAGES * SIZE_4KB;
            EthernetContext->ReceiveBuffer = Buffer;
            EthernetContext->PhysicalReceiveBuffer = PhysicalBuffer;

            //
            //  Start the test timer
            //
            TestTime = TEST_DURATION;
            Status = gBS->SetTimer ( TestTimer, TimerRelative, TestTime );
            if ( !EFI_ERROR ( Status )) {
              //
              //  Start the periodic timer
              //
              TimerDuration = ETHERNET_TIMER_TICK;
              Status = gBS->SetTimer ( TimerEvent, TimerPeriodic, TimerDuration );
              if ( !EFI_ERROR ( Status )) {
                //
                //  Wait for the test to complete
                //
                Status = gBS->WaitForEvent ( 1, &TestTimer, &Index );
              }

              //
              //  Stop the periodic timer
              //
              gBS->SetTimer ( TimerEvent, TimerCancel, 0 );

              //
              //  Reset the MAC
              //
              EthernetMacReset ( EthernetContext );
            }
          }
        }

        //
        //  Release the buffer
        //
        PciIo->FreeBuffer ( PciIo,
                            PageCount,
                            (VOID**)EthernetContext->ReceiveDescriptors );
      }

      //
      //  Release the events
      //
      gBS->CloseEvent ( TimerEvent );
      EthernetContext->Timer = NULL;
    }
    gBS->CloseEvent ( TestTimer );
  }

  //
  //  Return the operation status
  //
  return Status;
}


/**
  Eg20tMacPhy shell command

  The ShellCEntryLib library instance wrappers the actual UEFI application
  entry point and calls this ShellAppMain function.

  @param  ImageHandle  The image handle of the UEFI Application.
  @param  SystemTable  A pointer to the EFI System Table.

  @retval  0               The application exited normally.
  @retval  Other           An error occurred.

**/
INTN
EFIAPI 
ShellAppMain (
  IN UINTN Argc, 
  IN CHAR16 **Argv
  )
{
  UINTN ClockFrequency;
  ETHERNET_CONTEXT Context;
  UINTN Data;
  UINTN Data16;
  BOOLEAN DisplayHelp;
  ETHERNET_CONTEXT * EthernetContext;
  UINT32 GpioController;
  UINT32 GpioPort;
  BOOLEAN MonitorLink;
  EFI_PCI_IO_PROTOCOL * PciIo;
  UINTN PhyAddress;
  BOOLEAN PhyAddressSearch;
  UINT16 PhyData [ PHY_REGISTER_COUNT ];
  UINTN PhyRegister;
  BOOLEAN ResetChip;
  UINTN ResetDuration;
  UINTN ResetRecovery;
  EFI_STATUS Status;

  //
  //  Use break instead of goto
  //
  Status = EFI_SUCCESS;
  ClockFrequency = 25000000;
  PhyRegister = 0;
  Data = 0;
  DisplayHelp = TRUE;
  PhyAddressSearch = FALSE;
  ResetChip = FALSE;
  MonitorLink = FALSE;
  ResetDuration = PHY_RESET_DURATION;
  ResetRecovery = PHY_RESET_RECOVERY;

  //
  //  Validate the command syntax
  //
  if ( 2 <= Argc ) {
    //
    //  Check for link
    //
    if (( 0 == StrCmp ( L"link", Argv [ 1 ])) && ( 2 == Argc )) {
      MonitorLink = TRUE;
      DisplayHelp = FALSE;
    }

    //
    //  Check for reset
    //
    else if (( 0 == StrCmp ( L"reset", Argv [ 1 ])) && ( 2 <= Argc )) {
      //
      //  Adjust the reset duration
      //
      if ( 3 <= Argc ) {
        UINT32 Value;
        Value = Str2Uint ( Argv [ 2 ]);
        if ( 0 != Value ) {
          ResetDuration = Value;
        }
      }
      if ( 4 <= Argc ) {
        UINT32 Value;
        Value = Str2Uint ( Argv [ 3 ]);
        if ( 0 != Value ) {
          ResetRecovery = Value;
        }
      }

      //
      //  Display the reset parameters
      //
      GpioController = PcdGet32 ( PcdEthernetPhyResetGpioController );
      GpioPort = PcdGet32 ( PcdEthernetPhyResetGpio );
      Print ( L"GPIO Pin: %s%s%d\r\n", gResetGpioChip, gResetGpioController [ GpioController ], GpioPort );
      Print ( L"Reset duration: %d mSec\r\n", ResetDuration );
      Print ( L"Reset recovery: %d mSec\r\n", ResetRecovery );

      //
      //  Reset the PHY
      //
      Argc = 2;
      ResetChip = TRUE;
      DisplayHelp = FALSE;
    }
    //
    //  Check for PHY address search
    //
    else if (( 0 == StrCmp ( L"address", Argv [ 1 ])) && ( 2 == Argc )) {
      PhyAddressSearch = TRUE;
      DisplayHelp = FALSE;
    }

    //
    //  Validate the clock frequency
    //
    else if ( 0 == StrCmp ( L"2.5", Argv [ 1 ])) {
      ClockFrequency = 2500000;
      DisplayHelp = FALSE;
    }
    else if ( 0 == StrCmp ( L"25", Argv [ 1 ])) {
      ClockFrequency = 25000000;
      DisplayHelp = FALSE;
    }
    else if ( 0 == StrCmp ( L"125", Argv [ 1 ])) {
      ClockFrequency = 125000000;
      DisplayHelp = FALSE;
    }
    else {
      Print ( L"ERROR - Invalid clock frequency specified!\r\n\n" );
      Status = EFI_INVALID_PARAMETER;
    }
  }
  if ( 3 <= Argc ) {
    PhyRegister = Str2Uint ( Argv [ 2 ]);
  }
  if ( 4 <= Argc ) {
    Data = Str2Uint ( Argv [ 3 ]);
  }

  //
  //  Display the help text
  //
  if (( DisplayHelp ) || ( 4 < Argc )) {
    Print ( L"%s  <Clock Frequency>                          - Display PHY registers\n", Argv [ 0 ]);
    Print ( L"%s  <Clock Frequency>  <PHY Register>          - Read PHY register\r\n", Argv [ 0 ]);
    Print ( L"%s  <Clock Frequency>  <PHY Register>  <Data>  - Write PHY register\r\n", Argv [ 0 ]);
    Print ( L"%s  link                                       - Test the link operation\r\n", Argv [ 0 ]);
    Print ( L"%s  reset                                      - Reset the PHY\r\n", Argv [ 0 ]);
    Print ( L"\n" );
    Print ( L"Clock Frequency is specified in MHz and must be one of: 2.5, 25 or 125\n" );
    Print ( L"PhyRegister is in the range of 0 - 15\r\n" );
    Print ( L"Data is in the range of 0 - 65535 (0xFFFF)\r\n" );
  }
  else {
    //
    //  Initialize the Ethernet context
    //
    EthernetContext = &Context;
    ZeroMem ( &Context, sizeof ( Context ));

    EthernetContext->LinkState = LINK_STATE_UNKNOWN;
    EthernetContext->ReceiveIndexMask = RX_BUFFERS - 1;
    EthernetContext->ReceiveIndex = EthernetContext->ReceiveIndexMask;
    EthernetContext->ResetDuration = ResetDuration * 1000;
    EthernetContext->ResetRecovery = ResetRecovery * 1000;

    //
    //  Locate the controller
    //
    Status = LocateController ( &PciIo, EthernetContext );
    if ( !EFI_ERROR ( Status )) {
      PciIo = EthernetContext->PciIo;
      Status = EnableController ( PciIo, EthernetContext, ClockFrequency );
      if ( !EFI_ERROR ( Status )) {
        if ( 2 == Argc ) {
          if ( MonitorLink ) {
            //
            //  Monitor the link state
            //
            LinkMonitor ( EthernetContext );
          }
          else if ( ResetChip ) {
            //
            //  Reset the PHY
            //
            PhyResetChip ( EthernetContext );
//            PhyReset ( EthernetContext );
          }
          else if ( PhyAddressSearch ) {
            //
            //  Reset the PHY
            //
            PhyResetChip ( EthernetContext );

            //
            //  Search for the PHY address
            //
            for ( PhyAddress = 0; 32 > PhyAddress; PhyAddress++ ) {
              Data16 = EthernetPhyRead ( PciIo,
                                         PhyAddress,
                                         PHY_PHY_IDENTIFIER_1 );
              Data = Data16;
              Data <<= 16;
              Data16 = EthernetPhyRead ( PciIo,
                                         PhyAddress,
                                         PHY_PHY_IDENTIFIER_2 );
              Data |= Data16;
              if (( 0 != Data ) && ( 0xffffffff != Data )) {
                Print ( L"%2d: %s responding\r\n",
                        PhyAddress,
                        ( 0 == PhyAddress ) ? L"Broadcast address"
                                            : L"PHY" );
              }
            }
          }
          else {
            //
            //  Reset the PHY
            //
            PhyResetChip ( EthernetContext );

            //
            //  Wait for auto-negotiation to complete
            //
/*
            INTN Timeout;

            Timeout = AUTO_NEGOTIATION_TIMEOUT;
            do {
              PhyData [ PHY_STATUS ] = EthernetPhyRead ( PciIo,
                                                         EthernetContext->PhyAddress,
                                                         PHY_STATUS );
              if ( 0 != ( PhyData [ PHY_STATUS ] & ( STATUS_AUTO_NEGOTIATION_COMPLETE | STATUS_LINK_UP ))) {
                break;
              }
              gBS->Stall ( 1000 );
            } while ( 0 != Timeout-- );
*/

            //
            //  Display the PHY ID
            //
            Print ( L"Phy Address: %d\r\n", EthernetContext->PhyAddress );
            Print ( L"    OID: 0x%06x\r\n", EthernetContext->PhyId >> 10 );
            Print ( L"    Model: 0x%02x\r\n", ( EthernetContext->PhyId >> 4 ) & 0x3f );
            Print ( L"    Revision: 0x%x\r\n", EthernetContext->PhyId & 0xf );

            //
            //  Display the PHY registers
            //
            PhyRegisterRead ( EthernetContext, &PhyData [ 0 ]);
            PhyRegisterDisplay ( EthernetContext, &PhyData [ 0 ]);
            PhyRegisterDecode ( &PhyData [ 0 ]);
          }
        }
        else if ( 3 == Argc ) {
          Data = EthernetPhyRead ( PciIo,
                                   EthernetContext->PhyAddress,
                                   PhyRegister );
          Print ( L"PHY: 0x%02x --> 0x%04x\r\n", PhyRegister, Data );
        }
        else if ( 4 == Argc ) {
          Print ( L"PHY: 0x%02x <-- 0x%04x\r\n", PhyRegister, Data );
          EthernetPhyWrite ( PciIo,
                             EthernetContext->PhyAddress,
                             PhyRegister,
                             Data );
        }

        //
        //  Turn off the controller
        //
        DisableController ( PciIo );
      }
    }
  }

  //
  //  Return the operation status
  //
  return (INTN)Status;
}
