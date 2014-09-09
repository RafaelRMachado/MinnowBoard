/** @file
Declare the E6xx register offsets

Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _E6XX_H_
#define _E6XX_H_

///
/// Define the PCI-PCIe bridges
///
#define E6XX_PCIE_PORT_0        23
#define E6XX_PCIE_PORT_1        24
#define E6XX_PCIE_PORT_2        25
#define E6XX_PCIE_PORT_3        26

///
/// Define the GPIO register offsets
///

//  Core Well
#define GPIO_CGEN       0     /// GPIO enable
#define GPIO_CGIO       0x04  /// Input/output select
#define GPIO_CGLV       0x08  /// Input/output level
#define GPIO_CGTPE      0x0c  /// Positive edge interrupt enable
#define GPIO_CGTNE      0x10  /// Negative edge interrupt enable
#define GPIO_CGGPE      0x14  /// GPE enable
#define GPIO_CGSMI      0x18  /// SMI enable
#define GPIO_CGTS       0x1c  /// Interrupt status

//  Resume Well
#define GPIO_RGEN       0     /// GPIO enable
#define GPIO_RGIO       0x04  /// Input/output select
#define GPIO_RGLV       0x08  /// Input/output level
#define GPIO_RGTPE      0x0c  /// Positive edge interrupt enable
#define GPIO_RGTNE      0x10  /// Negative edge interrupt enable
#define GPIO_RGGPE      0x14  /// GPE enable
#define GPIO_RGSMI      0x18  /// SMI enable
#define GPIO_RGTS       0x1c  /// Interrupt status

#endif  //  _E6XX_H_
