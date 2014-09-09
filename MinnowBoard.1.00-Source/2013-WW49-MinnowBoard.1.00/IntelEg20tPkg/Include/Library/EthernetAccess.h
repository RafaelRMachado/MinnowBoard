/** @file
  Declare the Ethernet access interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _ETHERNET_ACCESS_H_
#define _ETHERNET_ACCESS_H_

///
/// Macro to reference controller registers
///
#define EthRead32(PciIo,Offset,Data)                \
            PciIo->Mem.Read ( PciIo,                \
                              EfiPciIoWidthUint32,  \
                              BAR_MEMORY,           \
                              Offset,               \
                              1,                    \
                              Data )
#define EthWrite32(PciIo,Offset,Data)               \
            PciIo->Mem.Write ( PciIo,               \
                               EfiPciIoWidthUint32, \
                               BAR_MEMORY,          \
                               Offset,              \
                               1,                   \
                               Data )

#endif  //  _ETHERNET_ACCESS_H_
