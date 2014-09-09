/** @file
  Implement the Simple Network protocol.
  
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
  Reads the current interrupt status and recycled transmit buffer status from 
  a network interface.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  InterruptStatus A pointer to the bit mask of the currently active interrupts
                          If this is NULL, the interrupt status will not be read from
                          the device. If this is not NULL, the interrupt status will
                          be read from the device. When the  interrupt status is read,
                          it will also be cleared. Clearing the transmit  interrupt
                          does not empty the recycled transmit buffer array.
  @param  TxBuf           Recycled transmit buffer address. The network interface will
                          not transmit if its internal recycled transmit buffer array
                          is full. Reading the transmit buffer does not clear the
                          transmit interrupt. If this is NULL, then the transmit buffer
                          status will not be read. If there are no transmit buffers to
                          recycle and TxBuf is not NULL, * TxBuf will be set to NULL.

  @retval EFI_SUCCESS           The status of the network interface was retrieved.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpGetStatus (
  IN EFI_SIMPLE_NETWORK_PROTOCOL  *SimpleNetworkProtocol,
  OUT UINT32                      *InterruptStatus OPTIONAL,
  OUT VOID                        **TxBuf OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_SIMPLE_NETWORK_MODE * ModeData;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    return EFI_INVALID_PARAMETER;
  }
 if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

  //
  //  Verify that the Ethernet controller is running
  //
  if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
    DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  GetStatus!\n", TplPrevious));
    gBS->RestoreTPL (TplPrevious);
    return EFI_DEVICE_ERROR;
  }

  if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
    DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
    gBS->RestoreTPL (TplPrevious);
    return EFI_NOT_STARTED;
  }

  //
  //  Update the interrupt status
  //
  EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
  EthernetInterrupts (EthernetContext);

  //
  //  Return the interrupt status
  //
  if (NULL != InterruptStatus) {
    *InterruptStatus = EthernetContext->InterruptStatus;
  }

  //
  //  Return the next free transmit buffer
  //
  if (NULL != TxBuf) {
    *TxBuf = EthernetTransmitComplete (EthernetContext);
  }

  //
  //  Return the link status
  //
  ModeData = &EthernetContext->ModeData;
  ModeData->MediaPresent = (BOOLEAN)(LINK_STATE_UP == EthernetContext->LinkState);
  
  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL (TplPrevious);

  return EFI_SUCCESS;
}


/**
  Resets a network adapter and allocates the transmit and receive buffers 
  required by the network interface; optionally, also requests allocation 
  of additional transmit and receive buffers.

  @param  SimpleNetworkProtocol    The protocol instance pointer.
  @param  ExtraRxBufferSize The size, in bytes, of the extra receive buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the extra
                            buffer, and the caller will not know if it is actually
                            being used.
  @param  ExtraTxBufferSize The size, in bytes, of the extra transmit buffer space
                            that the driver should allocate for the network interface.
                            Some network interfaces will not be able to use the extra
                            buffer, and the caller will not know if it is actually
                            being used.

  @retval EFI_SUCCESS           The network interface was initialized.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_OUT_OF_RESOURCES  There was not enough memory for the transmit and
                                receive buffers.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpInitialize (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN UINTN ExtraRxBufferSize  OPTIONAL,
  IN UINTN ExtraTxBufferSize  OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkInitialized == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Already initialized, must shutdown first!\n"));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Stopped, must start first!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Allocate the resources
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      Status = EthernetResourceAllocate (EthernetContext);

      //
      //  Reset the controller
      //
      if (!EFI_ERROR (Status)) {
        Status = SimpleNetworkProtocol->Reset (SimpleNetworkProtocol, FALSE);
      }

      //
      //  Free the resources on error
      //
      if (EFI_ERROR (Status)) {
        SimpleNetworkProtocol->Shutdown (SimpleNetworkProtocol);
      }
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Converts a multicast IP address to a multicast HW MAC address.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  IPv6 Set to TRUE if the multicast IP address is IPv6 [RFC 2460]. Set
               to FALSE if the multicast IP address is IPv4 [RFC 791].
  @param  IP   The multicast IP address that is to be converted to a multicast
               HW MAC address.
  @param  MAC  The multicast HW MAC address that is to be generated from IP.

  @retval EFI_SUCCESS           The multicast IP address was mapped to the multicast
                                HW MAC address.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_BUFFER_TOO_SMALL  The Statistics buffer was too small. The current buffer
                                size needed to hold the statistics is returned in
                                StatisticsSize.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpMcastIpToMac (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN BOOLEAN IPv6,
  IN EFI_IP_ADDRESS * IP,
  OUT EFI_MAC_ADDRESS * MAC
 )
{
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == IP) {
    DEBUG ((DEBUG_ERROR, "ERROR - IP is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == MAC) {
    DEBUG ((DEBUG_ERROR, "ERROR - MAC is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (IPv6) {
    DEBUG ((DEBUG_ERROR, "ERROR - IPv6 is not supported!\n"));
    Status = EFI_UNSUPPORTED;
  } else if (0xe0 != (IP->v4.Addr[0] & 0xf0)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Must be an IP multicast address!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpMcastIpToMac!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Construct the Ethernet multicast address
      //
      MAC->Addr[0] = 0x01;
      MAC->Addr[1] = 0x00;
      MAC->Addr[2] = 0x5e;
      MAC->Addr[3] = (UINT8) (IP->v4.Addr[1] & 0x7f);
      MAC->Addr[4] = (UINT8) IP->v4.Addr[2];
      MAC->Addr[5] = (UINT8) IP->v4.Addr[3];
      Status = EFI_SUCCESS;
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Performs read and write operations on the NVRAM device attached to a 
  network interface.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  ReadWrite  TRUE for read operations, FALSE for write operations.
  @param  Offset     Byte offset in the NVRAM device at which to start the read or
                     write operation. This must be a multiple of NvRamAccessSize and
                     less than NvRamSize.
  @param  BufferSize The number of bytes to read or write from the NVRAM device.
                     This must also be a multiple of NvramAccessSize.
  @param  Buffer     A pointer to the data buffer.

  @retval EFI_SUCCESS           The NVRAM access was performed.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpNvdata (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN BOOLEAN ReadWrite,
  IN UINTN Offset,
  IN UINTN BufferSize,
  IN OUT VOID * Buffer
 )
{
  //
  // This is not currently supported
  //
  return EFI_UNSUPPORTED;
}


/**
  Receives a packet from a network interface.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  HeaderSize The size, in bytes, of the media header received on the network
                     interface. If this parameter is NULL, then the media header size
                     will not be returned.
  @param  BufferSize On entry, the size, in bytes, of Buffer. On exit, the size, in
                     bytes, of the packet that was received on the network interface.
  @param  Buffer     A pointer to the data buffer to receive both the media header and
                     the data.
  @param  SrcAddr    The source HW MAC address. If this parameter is NULL, the
                     HW MAC source address will not be extracted from the media
                     header.
  @param  DestAddr   The destination HW MAC address. If this parameter is NULL,
                     the HW MAC destination address will not be extracted from the
                     media header.
  @param  Protocol   The media header type. If this parameter is NULL, then the
                     protocol will not be extracted from the media header. See
                     RFC 1700 section "Ether Types" for examples.

  @retval  EFI_SUCCESS           The received data was stored in Buffer, and BufferSize has
                                 been updated to the number of bytes received.
  @retval  EFI_NOT_STARTED       The network interface has not been started.
  @retval  EFI_NOT_READY         No packets have been received on the network interface.
  @retval  EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval  EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval  EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval  EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpReceive (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  OUT UINTN * HeaderSize OPTIONAL,
  IN OUT UINTN * BufferSize,
  OUT VOID * Buffer,
  OUT EFI_MAC_ADDRESS * SrcAddr    OPTIONAL,
  OUT EFI_MAC_ADDRESS * DestAddr   OPTIONAL,
  OUT UINT16 * Protocol   OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  UINT8 * ReceiveBuffer;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == BufferSize) {
    DEBUG ((DEBUG_ERROR, "ERROR - Buffer size is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == Buffer) {
    DEBUG ((DEBUG_ERROR, "ERROR - Buffer is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpReceive!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Get the receive packet
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      Status = EthernetReceiveFrame (EthernetContext, BufferSize, Buffer);
      if (!EFI_ERROR (Status)) {
        //
        //  Fill in the optional values
        //
        ReceiveBuffer = (UINT8 *)Buffer;
        if (NULL != HeaderSize) {
          *HeaderSize = ETHERNET_HEADER_SIZE;
        }
        if (NULL != DestAddr) {
          CopyMem (DestAddr, &ReceiveBuffer [ 0 ], PXE_HWADDR_LEN_ETHER);
        }
        if (NULL != SrcAddr) {
          CopyMem (SrcAddr, &ReceiveBuffer [ PXE_HWADDR_LEN_ETHER ], PXE_HWADDR_LEN_ETHER);
        }
        if (NULL != Protocol) {
          *Protocol = (UINT16)((ReceiveBuffer [ PXE_HWADDR_LEN_ETHER << 1 ] << 8)
                              | ReceiveBuffer [(PXE_HWADDR_LEN_ETHER << 1) + 1 ]);
        }
      }
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Manages the multicast receive filters of a network interface.

  @param  SimpleNetworkProtocol   The protocol instance pointer.
  @param  Enable           A bit mask of receive filters to enable on the network interface.
  @param  Disable          A bit mask of receive filters to disable on the network interface.
  @param  ResetMCastFilter Set to TRUE to reset the contents of the multicast receive
                           filters on the network interface to their default values.
  @param  McastFilterCnt   Number of multicast HW MAC addresses in the new
                           MCastFilter list. This value must be less than or equal to
                           the MCastFilterCnt field of EFI_SIMPLE_NETWORK_MODE. This
                           field is optional if ResetMCastFilter is TRUE.
  @param  MCastFilter      A pointer to a list of new multicast receive filter HW MAC
                           addresses. This list will replace any existing multicast
                           HW MAC address list. This field is optional if
                           ResetMCastFilter is TRUE.

  @retval EFI_SUCCESS           The multicast receive filter list was updated.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpReceiveFilters (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN UINT32 Enable,
  IN UINT32 Disable,
  IN BOOLEAN ResetMCastFilter,
  IN UINTN MCastFilterCnt     OPTIONAL,
  IN EFI_MAC_ADDRESS * MCastFilter OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  UINTN Index;
  CONST EFI_MAC_ADDRESS mNoMulticastAddress = { 0, 0, 0, 0, 0, 0 };
  EFI_SIMPLE_NETWORK_MODE * ModeData;
  UINT32 ReceiveChanges;
  UINT32 ReceiveMask;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;
  BOOLEAN Value;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (0 != (Enable & (~SimpleNetworkProtocol->Mode->ReceiveFilterMask))) {
    DEBUG ((DEBUG_ERROR, "ERROR - Bad bit set in Enable mask!\n", ETH_MAX_MCAST_FILTER_CNT));
    Status = EFI_INVALID_PARAMETER;
  } else if (0 != (Disable & (~SimpleNetworkProtocol->Mode->ReceiveFilterMask))) {
    DEBUG ((DEBUG_ERROR, "ERROR - Bad bit set in Disable mask!\n", ETH_MAX_MCAST_FILTER_CNT));
    Status = EFI_INVALID_PARAMETER;
  } else if (ETH_MAX_MCAST_FILTER_CNT < MCastFilterCnt) {
    DEBUG ((DEBUG_ERROR, "ERROR - MCastFilterCnt > %d\n", ETH_MAX_MCAST_FILTER_CNT));
    Status = EFI_INVALID_PARAMETER;
  } else if ((0 < MCastFilterCnt) && (NULL == MCastFilter)) {
    DEBUG ((DEBUG_ERROR, "ERROR - MCastFilter is NULL\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpReceiveFilters!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  MAC address management
      //
      //               +------------------+
      //               | PermanentAddress |
      //               +------------------+
      //                         |
      //                         | EthernetSnpInitialize or
      //                         | EthernetSnpStationAddress (Reset = TRUE)
      //                         |
      //                         V
      //                +-----------------+
      //                | CurrentAddress  |<---- New  EthernetSnpStationAddress
      //                +-----------------+
      //                              |
      //    EthernetSnpReceiveFilter  | EthernetSnpInitialize or
      //           00-00-00-00-00-00  | EthernetSnpReceiveFilter or
      //                    |         | EthernetSnpStationAddress
      //                    |         |
      //                    V         V
      //                +-----------------+
      //                | MCastFilter [15]|
      //                +-----------------+
      //                         |
      //                         | Registers valid after LINK_STATE_RESET:
      //                         |     EthernetSnpReceiveFilter or
      //                         |     EthernetSnpStationAddress or
      //                         |     EthernetTimer
      //                         | calls EthernetSetAddress
      //                         |
      //                         V
      //                +-----------------+
      //                | Reg MAC_ADDR_1  |
      //                +-----------------+
      //
      //  Multicast address management
      //
      //                           Multicast
      //                           Address
      //                              |
      //    EthernetSnpReceiveFilter  | EthernetSnpReceiveFilter
      //           00-00-00-00-00-00  |
      //                    |         |
      //                    |         |
      //                    V         V
      //                +-----------------+
      //                |MCastFilter[0-14]|
      //                +-----------------+
      //                         |
      //                         | Registers valid after LINK_STATE_RESET:
      //                         |     EthernetSnpReceiveFilter or
      //                         |     EthernetTimer
      //                         | calls EthernetSetAddress
      //                         |
      //                         V
      //                +-----------------+
      //                |Reg MAC_ADDR_2-16|
      //                +-----------------+
      //

      //
      //  Clear the multicast filters
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      ModeData = &EthernetContext->ModeData;
      Status = EFI_SUCCESS;
      ReceiveChanges = Enable | Disable;
      ReceiveMask = (ModeData->ReceiveFilterSetting | Enable)
                  & (~Disable)
                  & ModeData->ReceiveFilterMask;
      if (ResetMCastFilter) {
        for (Index = 0; ETH_MAX_MCAST_FILTER_CNT > Index; Index++) {
          CopyMem (&ModeData->MCastFilter [ Index ],
                    &mNoMulticastAddress,
                    sizeof (mNoMulticastAddress));
          if (LINK_STATE_RESET < EthernetContext->LinkState) {
            EthernetSetAddress (EthernetContext, Index);
          }
        }
        ModeData->MCastFilterCount = 0;
      } else {
        //
        //  Set the multicast filters
        //
        ModeData->MCastFilterCount = (UINT32)MCastFilterCnt;
        if ((0 == MCastFilterCnt)
          || (0 == (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_MULTICAST))) {
          DEBUG ((DEBUG_VERBOSE,
                    "Multicast Addresses cleared\n"));
        } else {
          DEBUG ((DEBUG_VERBOSE,
                    "Enabled Multicast Addresses:\n"));
        }
        for (Index = 0; MCastFilterCnt > Index; Index++) {
          DEBUG ((DEBUG_VERBOSE,
                    "    %02x-%02x-%02x-%02x-%02x-%02x\n",
                    MCastFilter [ Index ].Addr [ 0 ],
                    MCastFilter [ Index ].Addr [ 1 ],
                    MCastFilter [ Index ].Addr [ 2 ],
                    MCastFilter [ Index ].Addr [ 3 ],
                    MCastFilter [ Index ].Addr [ 4 ],
                    MCastFilter [ Index ].Addr [ 5 ]));
          CopyMem (&ModeData->MCastFilter [ Index ],
                    &MCastFilter [ Index ],
                    sizeof (mNoMulticastAddress));
          if (LINK_STATE_RESET < EthernetContext->LinkState) {
            EthernetSetAddress (EthernetContext, Index);
          }
        }
        for (; ETH_MAX_MCAST_FILTER_CNT > Index; Index++) {
          CopyMem (&ModeData->MCastFilter [ Index ],
                    &mNoMulticastAddress,
                    sizeof (mNoMulticastAddress));
          if (LINK_STATE_RESET < EthernetContext->LinkState) {
            EthernetSetAddress (EthernetContext, Index);
          }
        }
      
        //
        //  Set the new receiver settings
        //
        ModeData->ReceiveFilterSetting = ReceiveMask;
      
        //
        //  Update the unicast address
        //
        if (0 != (ReceiveChanges & EFI_SIMPLE_NETWORK_RECEIVE_UNICAST)) {
          if (0 == (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_UNICAST)) {
            CopyMem (&ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ],
                      &mNoMulticastAddress,
                      sizeof (mNoMulticastAddress));
            DEBUG ((DEBUG_VERBOSE, "Unicast: Disabled\n"));
          } else {
            CopyMem (&ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ],
                      &ModeData->CurrentAddress,
                      sizeof (ModeData->CurrentAddress));
            DEBUG ((DEBUG_VERBOSE,
                      "Unicast: %02x-%02x-%02x-%02x-%02x-%02x\n",
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 0 ],
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 1 ],
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 2 ],
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 3 ],
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 4 ],
                      ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 5 ]));
          }
          if (LINK_STATE_RESET < EthernetContext->LinkState) {
            EthernetSetAddress (EthernetContext, UNICAST_ADDRESS_INDEX);
          }
        }
      
        //
        //  Update the state if the link is up
        //
        if (LINK_STATE_RESET < EthernetContext->LinkState) {
          //
          //  Update the all multicast flag
          //
          if (0 != (ReceiveChanges & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST)) {
            Value = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS_MULTICAST));
            EthernetEnableAllMulticast (EthernetContext, Value);
          }
      
          //
          //  Update the broadcast receive flag
          //
          if (0 != (ReceiveChanges & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST)) {
            Value = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_BROADCAST));
            EthernetEnableBroadcast (EthernetContext, Value);
          }
      
          //
          //  Update the promiscuous flag
          //
          if (0 != (ReceiveChanges & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS)) {
            Value = (BOOLEAN)(0 != (ReceiveMask & EFI_SIMPLE_NETWORK_RECEIVE_PROMISCUOUS));
            EthernetEnablePromiscuous (EthernetContext, Value);
          }
        }
      
        //
        //  Update the receiver enable/disable
        //
        if (LINK_STATE_UP == EthernetContext->LinkState) {
          if (0 != ReceiveChanges) {
            Value = (BOOLEAN)(0 != ReceiveMask);
            EthernetEnableReceiver (EthernetContext, Value);
          }
        }
      }
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Resets a network adapter and re-initializes it with the parameters that were 
  provided in the previous call to Initialize().  

  @param  SimpleNetworkProtocol       The protocol instance pointer.
  @param  ExtendedVerification Indicates that the driver may perform a more
                               exhaustive verification operation of the device
                               during reset.

  @retval EFI_SUCCESS           The network interface was reset.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpReset (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN BOOLEAN ExtendedVerification
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpReset!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Reset the MAC
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      EthernetContext->LinkState = LINK_STATE_UNKNOWN;
      EthernetContext->ModeData.MediaPresent = FALSE;
      EthernetMacReset (EthernetContext);
      Status = EFI_SUCCESS;
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Resets a network adapter and leaves it in a state that is safe for 
  another driver to initialize.

  @param  SimpleNetworkProtocol  The protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was shutdown.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpShutdown (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Started, must initialize first!\n"));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Stopped, must start first!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Synchronize with the other threads
      //
      TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

      //
      //  Stop the periodic timer
      //
      Status = EFI_SUCCESS;
      if (NULL != EthernetContext->Timer) {
        Status = gBS->SetTimer (EthernetContext->Timer, TimerCancel, 0);
        ASSERT (EFI_SUCCESS == Status);
      }
      
      //
      //  Reset the Ethernet controller
      //
      SimpleNetworkProtocol->Reset (SimpleNetworkProtocol, FALSE);
      EthernetContext->ModeData.State = EfiSimpleNetworkStarted;
      EthernetContext->LinkState = LINK_STATE_DOWN;
      EthernetContext->ModeData.MediaPresent = FALSE;
      
      //
      //  Free the allocated resources
      //
      EthernetResourceFree (EthernetContext);

      //
      //  Release the thread synchronization
      //
      gBS->RestoreTPL (TplPrevious);
    }
  }

  return Status;
}


/**
  Changes the state of a network interface from "stopped" to "started".

  @param  SimpleNetworkProtocol  The protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was started.
  @retval EFI_ALREADY_STARTED   The network interface is already in the started state.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpStart (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkInitialized == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Already initialized, must shutdown first!\n"));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Already started, must stop first!\n"));
      Status = EFI_ALREADY_STARTED;
    } else {
      Status = EFI_SUCCESS;
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      EthernetContext->ModeData.State = EfiSimpleNetworkStarted;
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Modifies or resets the current station address, if supported.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  Reset Flag used to reset the station address to the network interfaces
                permanent address.
  @param  New   The new station address to be used for the network interface.

  @retval EFI_SUCCESS           The network interfaces station address was updated.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpStationAddress (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN BOOLEAN Reset,
  IN EFI_MAC_ADDRESS * New OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_SIMPLE_NETWORK_MODE * ModeData;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if ((NULL == New) && (!Reset)) {
    DEBUG ((DEBUG_ERROR, "ERROR - New is NULL\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpStationAddress!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Set the new address
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      ModeData = &EthernetContext->ModeData;
      if (Reset) {
        //
        //  Restore the physical address
        //
        CopyMem (&ModeData->CurrentAddress,
                  &ModeData->PermanentAddress,
                  sizeof (ModeData->CurrentAddress));
      } else {
        //
        //  Use the new address
        //
        CopyMem (&ModeData->CurrentAddress,
                  New,
                  sizeof (ModeData->CurrentAddress));
      }
      CopyMem (&ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ],
                &ModeData->CurrentAddress,
                sizeof (ModeData->CurrentAddress));
      DEBUG ((DEBUG_VERBOSE,
                "Current Address: %02x-%02x-%02x-%02x-%02x-%02x\n",
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 0 ],
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 1 ],
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 2 ],
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 3 ],
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 4 ],
                ModeData->MCastFilter [ UNICAST_ADDRESS_INDEX ].Addr [ 5 ]));
      if (LINK_STATE_RESET < EthernetContext->LinkState) {
        EthernetSetAddress (EthernetContext, 0);
      }
      Status = EFI_SUCCESS;
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Resets or collects the statistics on a network interface.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  Reset           Set to TRUE to reset the statistics for the network interface.
  @param  StatisticsSize  On input the size, in bytes, of StatisticsTable. On
                          output the size, in bytes, of the resulting table of
                          statistics.
  @param  StatisticsTable A pointer to the EFI_NETWORK_STATISTICS structure that
                          contains the statistics.

  @retval EFI_SUCCESS           The statistics were collected from the network interface.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_BUFFER_TOO_SMALL  The Statistics buffer was too small. The current buffer
                                size needed to hold the statistics is returned in
                                StatisticsSize.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpStatistics (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN BOOLEAN Reset,
  IN OUT UINTN * StatisticsSize   OPTIONAL,
  OUT EFI_NETWORK_STATISTICS * StatisticsTable  OPTIONAL
 )
{
  ETHERNET_CONTEXT * EthernetContext;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if ((! Reset)
    && (NULL == StatisticsSize)
    && (NULL == StatisticsTable)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Unable to return statistics size!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if ((NULL != StatisticsSize)
    && (NULL == StatisticsTable)
    && (sizeof (EthernetContext->Statistics) <= *StatisticsSize)) {
    DEBUG ((DEBUG_ERROR, "ERROR - StatisticsTable is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if ((NULL != StatisticsSize)
    && (NULL != StatisticsTable)
    && (sizeof (EthernetContext->Statistics) > *StatisticsSize)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Buffer < %d bytes!\n", sizeof (EthernetContext->Statistics)));
    *StatisticsSize = sizeof (EthernetContext->Statistics);
    Status = EFI_BUFFER_TOO_SMALL;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpStatistics!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Return the statistics if possible
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      if (NULL != StatisticsSize) {
        if (NULL != StatisticsTable) {
          CopyMem (StatisticsTable,
                    &EthernetContext->Statistics,
                    sizeof (EthernetContext->Statistics));
        }
      
        //
        //  Return the size of the statistics data
        //
        *StatisticsSize = sizeof (EthernetContext->Statistics);
      }
      
      //
      //  Zero the statistics buffer
      //
      if (Reset) {
        ZeroMem (&EthernetContext->Statistics, sizeof (EthernetContext->Statistics));
      }
      Status = EFI_SUCCESS;
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


/**
  Changes the state of a network interface from "started" to "stopped".

  @param  SimpleNetworkProtocol  The protocol instance pointer.

  @retval EFI_SUCCESS           The network interface was stopped.
  @retval EFI_ALREADY_STARTED   The network interface is already in the stopped state.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpStop (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol
 )
{
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (SimpleNetworkProtocol == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    return EFI_INVALID_PARAMETER;
  }
  if (SimpleNetworkProtocol->Mode == NULL) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    return EFI_INVALID_PARAMETER;
  }

  //
  //  Synchronize with the other threads
  //
  TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

  //
  //  Verify that the Ethernet controller is started
  //
  if (SimpleNetworkProtocol->Mode->State == EfiSimpleNetworkInitialized) {
    DEBUG ((DEBUG_ERROR, "ERROR - Already initialized, must shutdown first!\n"));
    Status = EFI_DEVICE_ERROR;
  } else if (SimpleNetworkProtocol->Mode->State == EfiSimpleNetworkStopped) {
    DEBUG ((DEBUG_ERROR, "ERROR - Already stopped, must start first!\n"));
    Status = EFI_NOT_STARTED;
  } else {
    SimpleNetworkProtocol->Mode->State = EfiSimpleNetworkStopped;
    Status = EFI_SUCCESS;
  }
  
  //
  //  Release the thread synchronization
  //
  gBS->RestoreTPL (TplPrevious);

  return Status;
}

/**
  Places a packet in the transmit queue of a network interface.

  @param  SimpleNetworkProtocol  The protocol instance pointer.
  @param  HeaderSize The size, in bytes, of the media header to be filled in by
                     the Transmit() function. If HeaderSize is non-zero, then it
                     must be equal to This->Mode->MediaHeaderSize and the DestAddr
                     and Protocol parameters must not be NULL.
  @param  BufferSize The size, in bytes, of the entire packet (media header and
                     data) to be transmitted through the network interface.
  @param  Buffer     A pointer to the packet (media header followed by data) to be
                     transmitted. This parameter cannot be NULL. If HeaderSize is zero,
                     then the media header in Buffer must already be filled in by the
                     caller. If HeaderSize is non-zero, then the media header will be
                     filled in by the Transmit() function.
  @param  SrcAddr    The source HW MAC address. If HeaderSize is zero, then this parameter
                     is ignored. If HeaderSize is non-zero and SrcAddr is NULL, then
                     This->Mode->CurrentAddress is used for the source HW MAC address.
  @param  DestAddr   The destination HW MAC address. If HeaderSize is zero, then this
                     parameter is ignored.
  @param  Protocol   The type of header to build. If HeaderSize is zero, then this
                     parameter is ignored. See RFC 1700, section "Ether Types", for
                     examples.

  @retval EFI_SUCCESS           The packet was placed on the transmit queue.
  @retval EFI_NOT_STARTED       The network interface has not been started.
  @retval EFI_NOT_READY         The network interface is too busy to accept this transmit request.                      
  @retval EFI_BUFFER_TOO_SMALL  The BufferSize parameter is too small.
  @retval EFI_INVALID_PARAMETER One or more of the parameters has an unsupported value.
  @retval EFI_DEVICE_ERROR      The command could not be sent to the network interface.
  @retval EFI_UNSUPPORTED       This function is not supported by the network interface.

**/
EFI_STATUS
EFIAPI
EthernetSnpTransmit (
  IN EFI_SIMPLE_NETWORK_PROTOCOL * SimpleNetworkProtocol,
  IN UINTN HeaderSize,
  IN UINTN BufferSize,
  IN VOID * Buffer,
  IN EFI_MAC_ADDRESS * SrcAddr  OPTIONAL,
  IN EFI_MAC_ADDRESS * DestAddr OPTIONAL,
  IN UINT16 * Protocol OPTIONAL
 )
{
  UINTN Data;
  ETHERNET_CONTEXT * EthernetContext;
  UINT8 * TransmitBuffer;
  EFI_STATUS Status;
  EFI_TPL TplPrevious;

  //
  //  Validate the parameters
  //
  if (NULL == SimpleNetworkProtocol) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == SimpleNetworkProtocol->Mode) {
    DEBUG ((DEBUG_ERROR, "ERROR - SimpleNetworkProtocol->Mode is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (NULL == Buffer) {
    DEBUG ((DEBUG_ERROR, "ERROR - Buffer is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if (ETHERNET_HEADER_SIZE > BufferSize) {
    DEBUG ((DEBUG_ERROR, "ERROR - Buffer too small!\n"));
    Status = EFI_BUFFER_TOO_SMALL;
  } else if ((ETH_TRANSMIT_BUFFER_SIZE - 4) < BufferSize) {
    DEBUG ((DEBUG_ERROR, "ERROR - BufferSize > %d bytes!\n", ETH_TRANSMIT_BUFFER_SIZE - 4));
    Status = EFI_INVALID_PARAMETER;
  } else if ((0 != HeaderSize) && (ETHERNET_HEADER_SIZE != HeaderSize)) {
    DEBUG ((DEBUG_ERROR, "ERROR - HeaderSize != %d bytes!\n", ETHERNET_HEADER_SIZE));
    Status = EFI_INVALID_PARAMETER;
  } else if ((0 != HeaderSize) && (NULL == DestAddr)) {
    DEBUG ((DEBUG_ERROR, "ERROR - DestAddr is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else if ((0 != HeaderSize) && (NULL == Protocol)) {
    DEBUG ((DEBUG_ERROR, "ERROR - Protocol is NULL!\n"));
    Status = EFI_INVALID_PARAMETER;
  } else {
    //
    //  Synchronize with the other threads
    //
    TplPrevious = gBS->RaiseTPL (TPL_NOTIFY);

    //
    //  Verify that the Ethernet controller is running
    //
    if (EfiSimpleNetworkStarted == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not initialized  TplPrevious = %d  EthernetSnpTransmit!\n", TplPrevious));
      Status = EFI_DEVICE_ERROR;
    } else if (EfiSimpleNetworkStopped == SimpleNetworkProtocol->Mode->State) {
      DEBUG ((DEBUG_ERROR, "ERROR - Not started!\n"));
      Status = EFI_NOT_STARTED;
    } else {
      //
      //  Build the transmit header if necessary
      //
      EthernetContext = ETHERNET_CONTEXT_FROM_SIMPLE_NETWORK_PROTOCOL (SimpleNetworkProtocol);
      TransmitBuffer = (UINT8 *)Buffer;
      if (0 != HeaderSize) {
        CopyMem (&TransmitBuffer [ 0 ], DestAddr, PXE_HWADDR_LEN_ETHER);
        CopyMem (&TransmitBuffer [ PXE_HWADDR_LEN_ETHER ],
                  (NULL != SrcAddr) ? SrcAddr
                                      : &EthernetContext->ModeData.CurrentAddress,
                  PXE_HWADDR_LEN_ETHER);
        Data = *Protocol;
        TransmitBuffer [ PXE_HWADDR_LEN_ETHER << 1 ] = (UINT8)(Data >> 8);
        TransmitBuffer [(PXE_HWADDR_LEN_ETHER << 1) + 1 ] = (UINT8)Data;
      }
      
      //
      //  Transmit the frame
      //
      Status = EthernetTransmitFrame (EthernetContext,
                                       BufferSize,
                                       Buffer);
    }
    
    //
    //  Release the thread synchronization
    //
    gBS->RestoreTPL (TplPrevious);
  }

  return Status;
}


CONST EFI_SIMPLE_NETWORK_PROTOCOL gEthernetSimpleNetwork = {
  EFI_SIMPLE_NETWORK_PROTOCOL_REVISION,
  EthernetSnpStart,
  EthernetSnpStop,
  EthernetSnpInitialize,
  EthernetSnpReset,
  EthernetSnpShutdown,
  EthernetSnpReceiveFilters,
  EthernetSnpStationAddress,
  EthernetSnpStatistics,
  EthernetSnpMcastIpToMac,
  EthernetSnpNvdata,
  EthernetSnpGetStatus,
  EthernetSnpTransmit,
  EthernetSnpReceive,
  NULL,
  NULL
};
