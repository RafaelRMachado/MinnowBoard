/** @file
  Driver tables
  
  Copyright (c) 2012, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "I2cBus.h"

/**
  Driver protocol GUID
**/
static EFI_GUID DriverGuid = 
{ 0x21f7ecf6, 0x95ac, 0x4810, { 0xbe, 0xd5, 0x2, 0x51, 0xad, 0xae, 0x31, 0x11 } };

EFI_GUID *mpDriverProtocol = &DriverGuid;

/**
  Component Name Protocol support
**/
CONST EFI_COMPONENT_NAME_PROTOCOL mComponentNameProtocol = {
  DlGetDriverName,
  DlGetControllerName,
  "eng"
};

/**
  Controller name string table
**/
static CONST EFI_UNICODE_STRING_TABLE mControllerNameStringTable[] = {
  { "eng", L"SMBus" },
  { NULL , NULL }
};

/**
  Driver name string table
**/
static CONST EFI_UNICODE_STRING_TABLE mDriverNameStringTable[] = {
  { "eng", L"SMBus Driver" },
  { NULL , NULL }
};

/**
  Driver library support
**/
CONST DL_DRIVER_LIB mDriverLib = {
  //
  //  Component name protocol support
  //
  &mComponentNameProtocol,
  NULL,
  &mControllerNameStringTable[0],
  &mDriverNameStringTable[0],

  //
  //  Driver binding protocol support
  //
  &mSmbusDriverBinding,

  //
  //  Loaded image protocol support
  //
  DlDriverUnload,
};
