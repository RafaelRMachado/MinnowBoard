/** @file
  GUID for 16550 UARTs on PCI busses

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php.

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _UART_16550_PCI_TOKEN_SPACE_GUID_H_
#define _UART_16550_PCI_TOKEN_SPACE_GUID_H_

#define UART_16550_PCI_TOKEN_SPACE_GUID \
  { \
     0x355ed05f, 0x1657, 0x487d, { 0xbd, 0xd2, 0x8d, 0xc6, 0xd7, 0xdd, 0x98, 0x2e } \
  }

extern EFI_GUID gUart16550PciTokenSpaceGuid;

#endif
