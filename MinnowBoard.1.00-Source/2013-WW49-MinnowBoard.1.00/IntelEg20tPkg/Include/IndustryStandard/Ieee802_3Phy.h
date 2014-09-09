/** @file
  Declare the IEEE 802.3 PHY interface.
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _IEEE_802_3_PHY_H_
#define _IEEE_802_3_PHY_H_

///
/// The IEEE 802.3 PHY interface is defined in the "Part 3: Carrier sense multiple
/// access with Collision Detection (CSMA/CD) Access Method and Physical Layer
/// Specifications" found at http://standards.ieee.org/about/get/802/802.3.html.
/// The Media Independent Interface (MII) registers are documented in section 22.2.4.
/// GMII registers are also documented in sections 28.2.4 and 37.2.5.1.
///

///
/// PHY Registers
///

#define PHY_CONTROL                                    0
#define PHY_STATUS                                     1
#define PHY_PHY_IDENTIFIER_1                           2
#define PHY_PHY_IDENTIFIER_2                           3
#define PHY_AUTO_NEGOTIATION_ADVERTISEMENT             4
#define PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY  5
#define PHY_AUTO_NEGOTIATION_EXPANSION                 6
#define PHY_AUTO_NEGOTIATION_NEXT_PAGE_TRANSMIT        7
#define PHY_AUTO_NEGOTIATION_NEXT_PAGE_RECEIVE         8
#define PHY_MASTER_SLAVE_CONTROL                       9
#define PHY_MASTER_SLAVE_STATUS                        10
#define PHY_PSE_CONTROL                                11
#define PHY_PSE_STATUS                                 12
#define PHY_MMD_ACCESS_CONTROL                         13
#define PHY_MMD_ACCESS_ADDRESS_DATA                    14
#define PHY_EXTENDED_STATUS                            15

///
/// 0: Control
///
/// Section 22.2.4.1
///

#define CONTROL_RESET                     BIT15           /// Reset the PHY
#define CONTROL_LOOPBACK                  BIT14           /// Enable loopback mode
#define CONTROL_SPEED_SELECTION           (BIT13 | BIT6)  /// Speed selection
#define CONTROL_AUTO_NEGOTIATION_ENABLE   BIT12           /// Enable auto negotiation
#define CONTROL_POWER_DOWN                BIT11           /// Power down the PHY
#define CONTROL_ISOLATE                   BIT10           /// Electrically isolate PHY from MII/GMII
#define CONTROL_RESTART_AUTO_NEGOTIATION  BIT9            /// Restart the auto negotiation process
#define CONTROL_DUPLEX_MODE               BIT8            /// Full duplex operation
#define CONTROL_COLLISION_TEST            BIT7            /// Enable COL signal test
#define CONTROL_UNIDIRECTIONAL_ENABLE     BIT5            /// Use MII even when link is down

#define CONTROL_SPEED_10_MBPS             0      /// 10 Mb/s
#define CONTROL_SPEED_100_MBPS            BIT13  /// 100 Mb/s
#define CONTROL_SPEED_1000_MBPS           BIT6   /// 1000 Mb/s

///
/// 1: Status
///
/// Section 22.2.4.2
///

#define STATUS_100BASE_T4                 BIT15  /// Supports 100BASE-T4
#define STATUS_100BASE_X_FULL_DUPLEX      BIT14  /// Supports 100BASE-X full duplex
#define STATUS_100BASE_X_HALF_DUPLEX      BIT13  /// Supports 100BASE-X half duplex
#define STATUS_10MBPS_FULL_DUPLEX         BIT12  /// Supports 10 Mb/s full duplex
#define STATUS_10MBPS_HALF_DUPLEX         BIT11  /// Supports 10 Mb/s half duplex
#define STATUS_100BASE_T2_FULL_DUPLEX     BIT10  /// Supports 100BASE-T2 full duplex
#define STATUS_100BASE_T2_HALF_DUPLEX     BIT9   /// Supports 100BASE-T2 half duplex
#define STATUS_EXTENDED_STATUS            BIT8   /// Extended status in R15
#define STATUS_UNIDIRECTION_ABILITY       BIT7   /// PHY can transmit without link
#define STATUS_MF_PREAMBLE_SUPPRESSION    BIT6   /// PHY will accept management frames with preamble suppressed
#define STATUS_AUTO_NEGOTIATION_COMPLETE  BIT5   /// Auto negotiation process is complete
#define STATUS_REMOTE_FAULT               BIT4   /// Remote fault condition detected
#define STATUS_AUTO_NEGOTIATION_ABILITY   BIT3   /// PHY is able to perform auto-negotiation
#define STATUS_LINK_STATUS                BIT2   /// Status of the link
#define STATUS_JABBER_DETECT              BIT1   /// Jabber condition detected
#define STATUS_EXTENDED_COMPABILITY       BIT0   /// Extended register capabilities

#define STATUS_FULL_DUPLEX                ( STATUS_100BASE_X_FULL_DUPLEX | STATUS_10MBPS_FULL_DUPLEX | STATUS_100BASE_T2_FULL_DUPLEX )
#define STATUS_SPEED_100MBPS              ( STATUS_100BASE_T4 | STATUS_100BASE_X_FULL_DUPLEX | STATUS_100BASE_X_HALF_DUPLEX | STATUS_100BASE_T2_FULL_DUPLEX | STATUS_100BASE_T2_HALF_DUPLEX )
#define STATUS_SPEED_10MBPS               ( STATUS_10MBPS_FULL_DUPLEX | STATUS_10MBPS_HALF_DUPLEX )
#define STATUS_100BASE_X                  ( STATUS_100BASE_X_FULL_DUPLEX | STATUS_100BASE_X_HALF_DUPLEX )
#define STATUS_100BASE_T2                 ( STATUS_100BASE_T2_FULL_DUPLEX | STATUS_100BASE_T2_HALF_DUPLEX )
#define STATUS_10MBPS                     ( STATUS_10MBPS_FULL_DUPLEX | STATUS_10MBPS_HALF_DUPLEX )
#define STATUS_LINK_UP                    STATUS_LINK_STATUS

///
/// 4: PHY_AUTO_NEGOTIATION_ADVERTISEMENT
/// 5: PHY_AUTO_NEGOTIATION_LINK_PARTNER_AVILABILITY
///

#define ANA_NEXT_PAGE                     BIT15   /// 
#define ANA_ACKNOWLEDGE                   BIT14   /// Acknowledge
#define ANA_REMOTE_FAULT                  BIT13   /// Remove fault detected
#define ANA_EXTENDED_NEXT_PAGE            BIT12   /// Extended next page
#define ANA_ASYMMETRIC_PAUSE              BIT11   /// Supports asymmetric pause for full duplex links
#define ANA_PAUSE                         BIT10   /// Supports pause operation for full duplex links
#define ANA_100BASE_T4                    BIT9    /// Supports 100BASE-T4
#define ANA_100BASE_TX_FD                 BIT8    /// Supports 100BASE-TX full duplex
#define ANA_100BASE_TX                    BIT7    /// Supports 100BASE-TX half duplex
#define ANA_10BASE_T_FD                   BIT6    /// Supports 10BASE-T full duplex
#define ANA_10BASE_T                      BIT5    /// Supports 10BASE-T half duplex
#define ANA_SELECTOR                      0x001f  /// Interface selector

#define ANA_SELECTOR_IEEE_802_3           1

///
/// 9: 100Base-T2 Control
///

#define MSC_TRANSMITTER_TEST_MODE         (BIT15 | BIT14)  /// Transmitter test mode
#define MSC_RECEIVER_TEST_MODE            BIT13            /// Receiver test mode
#define MSC_MANUAL_CONFIGURATION_ENABLE   BIT12            /// Master-slave manual configuration enable
#define MSC_PHY_MASTER                    BIT11            /// PHY is master in manual configuration
#define MSC_T2_REPEATER                   BIT10            /// Repeater device port
#define MSC_1000BASE_T_FD                 BIT9             /// Supports 1000BASE-T
#define MSC_1000BASE_T                    BIT8             /// Supports 1000BASE-T

///
/// 10: Masster Slave Status
///

#define MSS_STATUS_CONFIGURATION_FAULT    BIT15  /// Master slave configuration fault detected
#define MSS_STATUS_PHY_MASTER             BIT14  /// Local PHY is the masster
#define MSS_STATUS_LOCAL_RECEIVER_OK      BIT13  /// Local receiver is OK
#define MSS_STATUS_REMOTE_RECEIVER_OK     BIT12  /// Remote receiver is OK
#define MSS_STATUS_LP_1000_FD             BIT11  /// Supports 1000 Mb/s full duplex
#define MSS_STATUS_LP_1000_HD             BIT10  /// Supports 1000 Mb/s half duplex

#define MSS_STATUS_RECEIVER_OK            (MSS_STATUS_LOCAL_RECEIVER_OK | MSS_STATUS_REMOTE_RECEIVER_OK)
#define MSS_STATUS_LP_1000                (MSS_STATUS_LP_1000_FD | MSS_STATUS_LP_1000_HD)

///
/// 15: Extended Status
///

#define EX_STATUS_1000BASE_X_FULL_DUPLEX  BIT15   /// Supports 1000BASE-X full duplex
#define EX_STATUS_1000BASE_X_HALF_DUPLEX  BIT14   /// Supports 1000BASE-X half duplex
#define EX_STATUS_1000BASE_T_FULL_DUPLEX  BIT13   /// Supports 1000BASE-T full duplex
#define EX_STATUS_1000BASE_T_HALF_DUPLEX  BIT12   /// Supports 1000BASE-T half duplex

#define EX_STATUS_SPEED_1000MBPS          (EX_STATUS_1000BASE_X_FULL_DUPLEX | EX_STATUS_1000BASE_X_HALF_DUPLEX | EX_STATUS_1000BASE_T_FULL_DUPLEX | EX_STATUS_1000BASE_T_HALF_DUPLEX)
#define EX_STATUS_1000BASE_X              (EX_STATUS_1000BASE_X_FULL_DUPLEX | EX_STATUS_1000BASE_X_HALF_DUPLEX)
#define EX_STATUS_1000BASE_T              (EX_STATUS_1000BASE_T_FULL_DUPLEX | EX_STATUS_1000BASE_T_HALF_DUPLEX)
#define EX_STATUS_FULL_DUPLEX             (EX_STATUS_1000BASE_X_FULL_DUPLEX | EX_STATUS_1000BASE_T_FULL_DUPLEX)

#endif  //  _IEEE_802_3_PHY_H_
