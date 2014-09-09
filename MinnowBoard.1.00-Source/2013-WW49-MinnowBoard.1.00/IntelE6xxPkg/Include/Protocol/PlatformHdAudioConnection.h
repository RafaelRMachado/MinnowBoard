/** @file
  Platform HD Audio Connection protocol declaration

  This protocol describes the platform specific HD audio connections.

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __PLATFORM_HD_AUDIO_CONNECTION_H__
#define __PLATFORM_HD_AUDIO_CONNECTION_H__

///
/// Platform HD Audio Connection Protocol
///
/// Describe the HD audio codec and the platform
/// specific ports to which the codec connects.
///
typedef struct {
  ///
  /// Address of the codec
  ///
  UINT8 CodecAddress;

  ///
  /// Revision ID of codec
  ///
  UINT8 RevisionId;

  ///
  /// Vendor and Device ID of codec
  ///
  UINT32 VendorDeviceId;

  ///
  /// Number of entries in the verb table
  ///
  UINT32 HdAudioVerbTableEntries;

  ///
  /// Verb table for HD Audio Codec that describes the ports
  ///
  CONST UINT32 *HdAudioVerbTable;
} PLATFORM_HD_AUDIO_CONNECTION_PROTOCOL;

///
/// Reference to variable defined in the .DEC file
///
extern EFI_GUID gPlatformHdAudioConnectionProtocolGuid;

///
/// Tag GUID to indicate that all HD audio codec connection descriptions are available
///
extern EFI_GUID gHdAudioSyncGuid;

#endif  //  __PLATFORM_HD_AUDIO_CONNECTION_H__
