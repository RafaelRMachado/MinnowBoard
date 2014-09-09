/** @file
  MinnowBoard HD Audio connections

  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <IndustryStandard/HighDefinitionAudio.h>
#include <Library/BaseLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Protocol/PlatformHdAudioConnection.h>

//
//  HD Audio Codec : ALC262
//
#define CODEC_ADDRESS         0
#define CODEC_VENDOR_DEVICE   0x10EC0262

//
//  Describe the HD audio connections to the OS
//
CONST UINT32 mHdAudioVerbTable [] = {
  //
  //  NID 0x01: CODEC_VENDOR_DEVICE
  //
  HD_BOARD_ID ( CODEC_ADDRESS,
                1,
                CODEC_VENDOR_DEVICE >> 16,
                (UINT8)((CODEC_VENDOR_DEVICE >> 8 ) & 0xff),
                (UINT8)(CODEC_VENDOR_DEVICE & 0xff)),

  //
  //  NID 0x14 (Port D): Line Out
  //  Green 1/8" Jack
  //
  HD_CONFIG_NODE ( CODEC_ADDRESS,
                   0x14,
                   (CONFIG_PORT_FIXED
                   | CONFIG_LOCATION_EXTERNAL
                   | CONFIG_LOCATION_FRONT
                   | CONFIG_DEF_DEV_SPEAKER
                   | CONFIG_CONN_TYPE_0_125_JACK
                   | CONFIG_COLOR_GREEN
                   | CONFIG_MISC_JACK_DETECT_OVERRIDE
                   | 0x10) ),

  //
  //  NID 0x15 (Port A): Headphones Out
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x15 ),

  //
  //  NID 0x16: Mono Out
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x16 ),

  //
  //  NID 0x18 (Port B): Mic1
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x18 ),

  //
  //  NID 0x19 (Port F): Mic2
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x19 ),

  //
  //  NID 0x1A (Port C): Line1
  //  Blue 1/8" Jack
  HD_CONFIG_NODE ( CODEC_ADDRESS,
                   0x1a,
                   (CONFIG_PORT_JACK
                   | CONFIG_LOCATION_EXTERNAL
                   | CONFIG_LOCATION_FRONT
                   | CONFIG_DEF_DEV_LINE_IN
                   | CONFIG_CONN_TYPE_0_125_JACK
                   | CONFIG_COLOR_BLUE
                   | CONFIG_MISC_JACK_DETECT_OVERRIDE
                   | 0x20) ),

  //
  //  NID 0x1B (Port E): Line2
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x1b ),

  //
  //  NID 0x1C: CD In
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x1c ),

  //
  //  NID 0x1D: Beep In
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x1d ),

  //
  //  NID 0x1E: SPDIF Out
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x1e ),

  //
  //  NID 0x1F: SPDIF In
  //
  HD_NO_CONNECTION ( CODEC_ADDRESS, 0x1f )
};


PLATFORM_HD_AUDIO_CONNECTION_PROTOCOL mPlatformHdAudioConnection = {
  CODEC_ADDRESS,
  0x03,                 //  Revision ID of codec
  CODEC_VENDOR_DEVICE,  //  Vendor and Device ID of codec

  //
  //  Verb table size and address
  //
  sizeof (mHdAudioVerbTable) / sizeof (mHdAudioVerbTable [0]),
  &mHdAudioVerbTable [0]
};


/**
  Notify the common code of the HD audio connections.

  @retval  EFI_SUCCESS  Platform HD audio connection protocol installed successfully

**/

EFI_STATUS
EFIAPI
HdAudio (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS  Status;

  //
  //  Install the platform HD audio connection protocol
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &gImageHandle,
                  &gPlatformHdAudioConnectionProtocolGuid,
                  &mPlatformHdAudioConnection,
                  NULL
                  );
  return Status;
}
