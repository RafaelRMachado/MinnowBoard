/** @file
  Declare the ASCII dump routine

Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _HIGH_DEFINITION_AUDIO_H
#define _HIGH_DEFINITION_AUDIO_H

#define CONFIG_PORT_CONNECTION      0xc0000000  /// How port is connected
#define CONFIG_LOCATION             0x3f000000  /// Location of the port
#define CONFIG_DEFAULT_DEVICE       0x00f00000  /// Default device for port
#define CONFIG_CONNECTION_TYPE      0x000f0000  /// Type of physical connection
#define CONFIG_COLOR                0x0000f000  /// Color of the port
#define CONFIG_MISC                 0x00000f00  /// Other port information
#define CONFIG_DEFAULT_ASSOCIATION  0x000000f0
#define CONFIG_SEQUENCE             0x0000000f  /// Order of the jacks in a group

//
//  Types of port connections
//
#define CONFIG_PORT_FIXED_AND_JACK        0xc0000000
#define CONFIG_PORT_FIXED                 0x80000000
#define CONFIG_PORT_NONE                  0x40000000
#define CONFIG_PORT_JACK                  0

//
//  Location options
//
#define CONFIG_LOCATION_EXTERNAL          0
#define CONFIG_LOCATION_INTERNAL          0x10000000
#define CONFIG_LOCATION_SEPARATE_CHASSIS  0x20000000
#define CONFIG_LOCATION_OTHER             0x30000000

#define CONFIG_LOCATION_NA                0
#define CONFIG_LOCATION_REAR              0x01000000
#define CONFIG_LOCATION_FRONT             0x02000000
#define CONFIG_LOCATION_LEFT              0x03000000
#define CONFIG_LOCATION_RIGHT             0x04000000
#define CONFIG_LOCATION_TOP               0x05000000
#define CONFIG_LOCATION_BOTTOM            0x06000000
#define CONFIG_LOCATION_REAR_PANEL        0x07000000
#define CONFIG_LOCATION_RISER             0x17000000
#define CONFIG_LOCATION_LID_INSIDE        0x37000000
#define CONFIG_LOCATION_DRIVE_BAY         0x08000000
#define CONFIG_LOCATION_DIGITAL_DISPLAY   0x18000000
#define CONFIG_LOCATION_LID_OUTSIDE       0x38000000
#define CONFIG_LOCATION_ATAPI             0x19000000

//
//  Default device
//
#define CONFIG_DEF_DEV_LINE_OUT           0
#define CONFIG_DEF_DEV_SPEAKER            0x00100000
#define CONFIG_DEF_DEV_HP_OUT             0x00200000
#define CONFIG_DEF_DEV_CD                 0x00300000
#define CONFIG_DEF_DEV_SPDIF_OUT          0x00400000
#define CONFIG_DEF_DEV_DIGITAL_OTHER_OUT  0x00500000
#define CONFIG_DEF_DEV_MODEM_LINE_SIDE    0x00600000
#define CONFIG_DEF_DEV_MODEM_HANDSET_SIDE 0x00700000
#define CONFIG_DEF_DEV_LINE_IN            0x00800000
#define CONFIG_DEF_DEV_AUX                0x00900000
#define CONFIG_DEF_DEV_MIC_IN             0x00a00000
#define CONFIG_DEF_DEV_TELEPHONY          0x00b00000
#define CONFIG_DEF_DEV_SPDIF_IN           0x00c00000
#define CONFIG_DEF_DEV_DIGITAL_OTHER_IN   0x00d00000
#define CONFIG_DEF_DEV_RESERVED           0x00e00000
#define CONFIG_DEF_DEV_OTHER              0x00f00000

//
//  Connection type
//
#define CONFIG_CONN_TYPE_UNKNOWN          0
#define CONFIG_CONN_TYPE_0_125_JACK       0x00010000
#define CONFIG_CONN_TYPE_0_25_JACK        0x00020000
#define CONFIG_CONN_TYPE_ATAPI_INTERNAL   0x00030000
#define CONFIG_CONN_TYPE_RCA              0x00040000
#define CONFIG_CONN_TYPE_OPTICAL          0x00050000
#define CONFIG_CONN_TYPE_OTHER_DIGITAL    0x00060000
#define CONFIG_CONN_TYPE_OTHER_ANALOG     0x00070000
#define CONFIG_CONN_TYPE_MULTICHANNEL_ANALOG  0x00080000
#define CONFIG_CONN_TYPE_XLR_PROFESSIONAL 0x00090000
#define CONFIG_CONN_TYPE_RJ11_MODEM       0x000a0000
#define CONFIG_CONN_TYPE_COMBINATION      0x000b0000
#define CONFIG_CONN_TYPE_OTHER            0x000f0000

//
//  Port Color
//
#define CONFIG_COLOR_UNKNOWN              0
#define CONFIG_COLOR_BLACK                0x00001000
#define CONFIG_COLOR_GREY                 0x00002000
#define CONFIG_COLOR_BLUE                 0x00003000
#define CONFIG_COLOR_GREEN                0x00004000
#define CONFIG_COLOR_RED                  0x00005000
#define CONFIG_COLOR_ORANGE               0x00006000
#define CONFIG_COLOR_YELLOW               0x00007000
#define CONFIG_COLOR_PURPLE               0x00008000
#define CONFIG_COLOR_PINK                 0x00009000
#define CONFIG_COLOR_WHITE                0x0000e000
#define CONFIG_COLOR_OTHER                0x0000f000

//
//  Misc
//
#define CONFIG_MISC_JACK_DETECT_OVERRIDE  0x00000100
#define CONFIG_MISC_RESERVED_BIT_1        0x00000200
#define CONFIG_MISC_RESERVED_BIT_2        0x00000400
#define CONFIG_MISC_RESERVED_BIT_3        0x00000800

//
//  Verbs
//
#define VERB_CONFIG_BITS_7_0              0x71c
#define VERB_CONFIG_BITS_15_8             0x71d
#define VERB_CONFIG_BITS_23_16            0x71e
#define VERB_CONFIG_BITS_31_24            0x71f

#define VERB_BOARD_ID_BITS_7_0            0x720
#define VERB_BOARD_ID_BITS_15_8           0x721
#define VERB_BOARD_ID_BITS_23_16          0x722
#define VERB_BOARD_ID_BITS_31_24          0x723

//
//  HD Audio Verb format
//
#define HD_VERB(Address,Node,Verb,Data)   (( Address << 28 ) | ( Node << 20 ) | ( Verb << 8 ) | Data )

//
//  See Section 7.3.3.31 of http://www.intel.com/content/dam/www/public/us/en/documents/product-specifications/high-definition-audio-specification.pdf
//  See Figure 74
//
#define HD_CONFIG_NODE(Address,Node,Data) \
    HD_VERB ( Address, Node, VERB_CONFIG_BITS_7_0,   ( Data & 0xff )), \
    HD_VERB ( Address, Node, VERB_CONFIG_BITS_15_8,  (( Data >> 8 ) & 0xff )), \
    HD_VERB ( Address, Node, VERB_CONFIG_BITS_23_16, (( Data >> 16 ) & 0xff )), \
    HD_VERB ( Address, Node, VERB_CONFIG_BITS_31_24, (( Data >> 24 ) & 0xff ))

//
//  See Section 7.3.3.30 of http://www.intel.com/content/dam/www/public/us/en/documents/product-specifications/high-definition-audio-specification.pdf
//
#define HD_BOARD_ID(Address,Node,Manufacture,Sku,Assembly) \
    HD_VERB ( Address, Node, VERB_BOARD_ID_BITS_7_0,   ( Assembly & 0xff )), \
    HD_VERB ( Address, Node, VERB_BOARD_ID_BITS_15_8,  ( Sku & 0xff )), \
    HD_VERB ( Address, Node, VERB_BOARD_ID_BITS_23_16, ( Manufacture & 0xff )), \
    HD_VERB ( Address, Node, VERB_BOARD_ID_BITS_31_24, (( Manufacture >> 8 ) & 0xff ))

//
//  No connection
//
#define HD_NO_CONNECTION(Address,Node)                    \
    HD_CONFIG_NODE ( Address,                             \
                     Node,                                \
                     (CONFIG_PORT_NONE                    \
                     | CONFIG_LOCATION_REAR               \
                     | CONFIG_DEF_DEV_SPEAKER             \
                     | CONFIG_CONN_TYPE_0_125_JACK        \
                     | CONFIG_COLOR_BLACK                 \
                     | CONFIG_MISC_JACK_DETECT_OVERRIDE   \
                     | 0xF0) )

#endif  //  _HIGH_DEFINITION_AUDIO_H
