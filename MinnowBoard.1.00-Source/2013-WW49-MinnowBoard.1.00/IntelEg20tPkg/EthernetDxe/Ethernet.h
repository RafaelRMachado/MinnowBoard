/** @file
  Intel Ethernet Driver Declarations

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _ETHERNET_CONTROLLER_H
#define _ETHERNET_CONTROLLER_H

#include <Uefi.h>

#include <Protocol/PciIo.h>
#include <Protocol/ComponentName.h>
#include <Protocol/ComponentName2.h>
#include <Protocol/DriverBinding.h>
#include <Protocol/SimpleNetwork.h>
#include <Protocol/NetworkInterfaceIdentifier.h>

#include <Guid/EventGroup.h>

#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/DevicePathLib.h>

#include <IndustryStandard/Pci.h>

#include <IndustryStandard/Ieee802_3Phy.h>

#include <Register/Eg20t.h>

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

///
/// Macro to reference controller registers
///
#define BAR_MEMORY 1

typedef struct {
  UINT32 Mode;
  UINT32 RgmiiControl;
} MAC_CONTROL_REGISTERS;



#define ETHERNET_TIMER_TICK       EFI_TIMER_PERIOD_MILLISECONDS (1)
#define LINK_TIMEOUT              EFI_TIMER_PERIOD_SECONDS (5)
#define PHY_REGISTER_COUNT        (( MIIM_REG_ADDR >> 16 ) + 1 )

#define RX_BUFFERS                64
#define RX_BUFFER_PAGES           EFI_SIZE_TO_PAGES (RX_BUFFERS * ETH_RECEIVE_BUFFER_SIZE)
#define RX_DESCRIPTOR_AREA_SIZE   (RX_BUFFERS * sizeof (ETH_RECEIVE_DESCRIPTOR))
#define RX_DESCRIPTOR_PAGES       EFI_SIZE_TO_PAGES (RX_DESCRIPTOR_AREA_SIZE)

#define TX_BUFFERS                8
#define TX_BUFFER_PAGES           EFI_SIZE_TO_PAGES (TX_BUFFERS * ETH_RECEIVE_BUFFER_SIZE)
#define TX_DESCRIPTOR_AREA_SIZE   (TX_BUFFERS * sizeof (ETH_TRANSMIT_DESCRIPTOR))
#define TX_DESCRIPTOR_PAGES       EFI_SIZE_TO_PAGES (TX_DESCRIPTOR_AREA_SIZE)

#define ETHERNET_HEADER_SIZE      (PXE_HWADDR_LEN_ETHER + PXE_HWADDR_LEN_ETHER + 2)

///
/// State of the link
///
typedef enum _LINK_STATE {
  LINK_STATE_UNKNOWN = 0,
  LINK_STATE_DOWN,
  LINK_STATE_RESETTING,
  LINK_STATE_RESET,
  LINK_STATE_PHY_RESET,
  LINK_STATE_CONFIGURE_PHY,
  LINK_STATE_AUTO_NEGOTIATING,
  LINK_STATE_UP
} LINK_STATE;

///
/// Ethernet Controller context
///
typedef struct {
  ///
  /// Structure identification
  ///
  UINTN Signature;

  EFI_HANDLE                 DeviceHandle;
  EFI_DEVICE_PATH_PROTOCOL   *DevPath;

  ///
  /// Allocated buffers
  ///
  UINTN PageCount;
  EFI_PHYSICAL_ADDRESS PageAddress;

  ///
  /// DMA management data
  ///
  VOID * DmaMapping;                  /// Mapping handle for Unmap
  EFI_PHYSICAL_ADDRESS PhysicalReceiveDescriptors;  /// Physical address of receive descriptors
  EFI_PHYSICAL_ADDRESS PhysicalReceiveBuffer; /// Physical address of receive descriptors
  EFI_PHYSICAL_ADDRESS PhysicalTransmitDescriptors; /// Physical address of transmit descriptors
  EFI_PHYSICAL_ADDRESS PhysicalTransmitBuffer;  /// Physical address of transmit descriptors

  ///
  /// Receive buffer management data
  ///
  UINT32 InterruptStatus;             /// Interrupt status
  UINT8 * ReceiveBuffer;              /// Beginning of the receive buffers
  ETH_RECEIVE_DESCRIPTOR * ReceiveDescriptors;  /// Beginning of descriptor array
  UINTN ReceiveIndex;                 /// Index of last receive descriptor
  UINTN ReceiveIndexMask;             /// Mask bits for receive descriptor index
  BOOLEAN ReceiveBroadcast;           /// Broadcast receive enabled

  ///
  /// Transmit queue management data
  ///
  ETH_TRANSMIT_DESCRIPTOR * TransmitDescriptors;  /// Beginning of descriptor area
  UINTN TransmitCompleteIndex;        /// Index of next completed descriptor
  UINTN TransmitIndex;                /// Index of next empty descriptor
  UINTN TransmitIndexMask;            /// Mask bits for transmit descriptor index
  UINT8 * TransmitBuffer;             /// Beginning of the transmit buffers
  UINT8 * UserBuffers [ TX_BUFFERS ]; /// User data buffers

  ///
  /// Controller specific data
  ///
  EFI_PCI_IO_PROTOCOL  *PciIo;                 /// PCI controller access
  UINT64               PciSupports;            /// PCI Supported Attributes saved in Start 
  UINT64               OriginalPciAttributes;  /// PCI Attributes saved in Start and restored in Stop
  UINTN LinkSpeed;                             /// Speed in Mb/s
  BOOLEAN FullDuplex;                          /// Link duplex

  ///
  /// PHY Registers
  ///
  PHY_CONTROL_REGISTERS PhyControl;
  PHY_STATUS_REGISTERS PhyStatus;

  ///
  /// Link maintance data
  ///
  EFI_EVENT Timer;
  volatile LINK_STATE LinkState;
  UINTN PhyAddress;
  UINT64 LinkTimerEventCounter;

  ///
  /// Configuration and management data
  ///
  EFI_SIMPLE_NETWORK_MODE ModeData;
  EFI_NETWORK_STATISTICS Statistics;

  ///
  /// Upper level API
  ///
  EFI_SIMPLE_NETWORK_PROTOCOL SimpleNetworkProtocol;
  
  ///
  /// Event trigged at Exit Boot Services to disable Rx/Tx
  ///
  EFI_EVENT                   ExitBootServicesEvent;
} ETHERNET_CONTEXT;

///
/// Locate ETHERNET_CONTEXT from protocol
///
#define ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL(a)  CR (a, ETHERNET_CONTEXT, SimpleNetworkProtocol,  ETHERNET_CONTEXT_SIGNATURE)

///
/// Number of available multicast addresses
///
#define ETH_MAX_MCAST_FILTER_CNT            ( ETH_MAX_ADDRESSES - 1 )

//
//  Storage location for unicast address
//
#define UNICAST_ADDRESS_INDEX               ( ETH_MAX_ADDRESSES - 1 )

///
/// "EthC"
///
#define ETHERNET_CONTEXT_SIGNATURE        0x43687445

extern EFI_DRIVER_BINDING_PROTOCOL        gEg20tEthernetDriverBinding;
extern EFI_COMPONENT_NAME2_PROTOCOL       gEg20tEthernetComponentName2;
extern EFI_COMPONENT_NAME_PROTOCOL        gEg20tEthernetComponentName;
extern CONST EFI_SIMPLE_NETWORK_PROTOCOL  gEthernetSimpleNetwork;

/**
  Start the Ethernet controller

  This routine allocates the necessary resources for the driver.

  This routine is called by EthernetDriverStart to complete the driver
  initialization.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @retval EFI_SUCCESS         Driver API properly initialized
  
**/
EFI_STATUS
EthernetApiStart (
  IN ETHERNET_CONTEXT *EthernetContext
  );

/**
  Stop the Ethernet controller

  This routine releases the resources allocated by EthernetApiStart.

  This routine is called by EthernetDriverStop to initiate the driver
  shutdown.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetApiStop (
  IN ETHERNET_CONTEXT *EthernetContext
  );

/**
  Enable all multicast addresses

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableAllMulticast (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN BOOLEAN Enable
  );

/**
  Enable broadcast

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableBroadcast (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN BOOLEAN Enable
  );

/**
  Enable all addresses

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnablePromiscuous (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN BOOLEAN Enable
  );

/**
  Enable the receiver

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Enable           TRUE to enable, FALSE to disable

**/
VOID
EthernetEnableReceiver (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN BOOLEAN Enable
  );

/**
  Check for Ethernet interrupts

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @return   This routine returns the interrupt status

**/
UINT32
EthernetInterrupts (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Reset the MAC

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetMacReset (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Receive an Ethernet frame

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param  BufferSize On entry, the size, in bytes, of Buffer. On exit, the size, in
                     bytes, of the packet that was received on the network interface.
  @param  Buffer     A pointer to the data buffer to receive both the media header and
                     the data.

  @retval  EFI_SUCCESS           The received data was stored in Buffer, and BufferSize has
                                 been updated to the number of bytes received.
  @retval  EFI_NOT_READY         No packets have been received on the network interface.
  @retval  EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.

**/
EFI_STATUS
EthernetReceiveFrame (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer
  );

/**
  Determine if the MAC reset is complete

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
BOOLEAN
EthernetMacResetComplete (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Determine if the MAC reset duration is complete

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
BOOLEAN
EthernetMacResetDurationComplete (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Allocate the necessary resources

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @retval  EFI_SUCCESS           The resources were allocated successfully.
  @retval  EFI_OUT_OF_RESOURCES  The resources were not available.

**/
EFI_STATUS
EthernetResourceAllocate (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Free the resources allocated by EthernetResourceAllocate

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

**/
VOID
EthernetResourceFree (
  IN ETHERNET_CONTEXT * EthernetContext
  );

/**
  Set an Ethernet address

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param[in] Index            Index into the list of addresses

**/
VOID
EthernetSetAddress (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN UINTN Index
  );

/**
  Periodic timer event routine

  @param  Event           Event whose notification function is being invoked.
  @param  EthernetContext The pointer to the notification function's context,
                          which is implementation-dependent.

**/
VOID
EFIAPI
EthernetTimer (
  IN  EFI_EVENT  Event,
  IN  VOID       *Context
  );

/**
  Transmit an Ethernet frame

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure
  @param  BufferSize The size, in bytes, of the entire packet (media header and
                     data) to be transmitted through the network interface.
  @param  Buffer     A pointer to the packet (media header followed by data) to be
                     transmitted. This parameter cannot be NULL. If HeaderSize is zero,
                     then the media header in Buffer must already be filled in by the
                     caller. If HeaderSize is non-zero, then the media header will be
                     filled in by the Transmit() function.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_READY         The network interface is too busy to accept this transmit request.                      

**/
EFI_STATUS
EthernetTransmitFrame (
  IN ETHERNET_CONTEXT * EthernetContext,
  IN UINTN BufferSize,
  IN UINT8 * Buffer
  );

/**
  Determine if a transmit is complete

  This routine must be called at TPL_NOTIFY.

  @param[in] EthernetContext  Address of an ETHERNET_CONTEXT structure

  @return  This routine returns the buffer address of the completed frame.

**/
UINT8 *
EthernetTransmitComplete (
  IN ETHERNET_CONTEXT * EthernetContext
  );

  
  
EFI_STATUS
EFIAPI
EthernetDriverSupported (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  );
  
EFI_STATUS
EFIAPI
EthernetDriverStart (
  IN EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN EFI_HANDLE Controller,
  IN EFI_DEVICE_PATH_PROTOCOL *RemainingDevicePath
  );

EFI_STATUS
EFIAPI
EthernetDriverStop (
  IN  EFI_DRIVER_BINDING_PROTOCOL *DriverBinding,
  IN  EFI_HANDLE Controller,
  IN  UINTN NumberOfChildren,
  IN  EFI_HANDLE *ChildHandleBuffer
  );
  
EFI_STATUS
EFIAPI
Eg20tEthernetComponentNameGetDriverName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **DriverName
  );
  
EFI_STATUS
EFIAPI
Eg20tEthernetComponentNameGetControllerName (
  IN  EFI_COMPONENT_NAME2_PROTOCOL  *This,
  IN  EFI_HANDLE                    ControllerHandle,
  IN  EFI_HANDLE                    ChildHandle        OPTIONAL,
  IN  CHAR8                         *Language,
  OUT CHAR16                        **ControllerName
  );

UINT16
EthernetPhyRead (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister
  );

VOID
EthernetPhyWrite (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress,
  IN UINTN PhyRegister,
  IN UINTN Value
  );

VOID
EthernetPhyConfigure (
  IN OUT PHY_CONTROL_REGISTERS * LinkControl,
  IN UINTN LinkSpeed,
  IN BOOLEAN LinkFullDuplex
  );

VOID
EthernetPhyDecode (
  IN PHY_STATUS_REGISTERS * PhyStatus,
  OUT BOOLEAN * LinkUp,
  OUT UINTN * LinkSpeed,
  OUT BOOLEAN * LinkFullDuplex
  );
  
VOID
EthernetPhyReset (
  IN ETHERNET_CONTEXT  *EthernetContext
  );

BOOLEAN
EthernetPhyGetAddress (
  IN  ETHERNET_CONTEXT  *EthernetContext,
  OUT UINTN             *PhyAddress,
  OUT UINT32            *PhyId
  );

VOID
EthernetMacConfigure (
  IN OUT MAC_CONTROL_REGISTERS * MacControl,
  IN UINTN LinkSpeed,
  IN BOOLEAN LinkFullDuplex
  );
  
EFI_STATUS
EthernetGetMacAddress (
  IN ETHERNET_CONTEXT  *EthernetContext,
  OUT EFI_MAC_ADDRESS  *MacAddress
  );
  
VOID
PlatformSpecificPhyConfiguration (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN PhyAddress
  );

UINT32
PciIoMemRead32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset
  );

UINT32
PciIoMemWrite32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
  );

UINT32
PciIoMemOr32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
  );

UINT32
PciIoMemAnd32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
  );

#endif  //  _ETHERNET_CONTROLLER_H
