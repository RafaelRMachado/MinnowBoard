/** @file
  Implement the Ethernet controller access routines
  
  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Ethernet.h"

UINT32
PciIoMemRead32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset
 )
{
  EFI_STATUS  Status;
  UINT32      Result;

  Status = EthernetContext->PciIo->Mem.Read (
                                         EthernetContext->PciIo,
                                         EfiPciIoWidthUint32,
                                         BAR_MEMORY,
                                         Offset,
                                         1,
                                         &Result
                                         );
  ASSERT_EFI_ERROR (Status);
  return Result;
}

UINT32
PciIoMemWrite32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
 )
{
  EFI_STATUS  Status;

  Status = EthernetContext->PciIo->Mem.Write (
                                         EthernetContext->PciIo,
                                         EfiPciIoWidthUint32,
                                         BAR_MEMORY,
                                         Offset,
                                         1,
                                         &Value
                                         );
  ASSERT_EFI_ERROR (Status);
  return Value;
}

UINT32
PciIoMemOr32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
 )
{
  return PciIoMemWrite32 (EthernetContext, Offset, PciIoMemRead32 (EthernetContext, Offset) | Value);
}

UINT32
PciIoMemAnd32 (
  IN ETHERNET_CONTEXT  *EthernetContext,
  IN UINTN             Offset,
  IN UINT32            Value
 )
{
  return PciIoMemWrite32 (EthernetContext, Offset, PciIoMemRead32 (EthernetContext, Offset) & Value);
}
