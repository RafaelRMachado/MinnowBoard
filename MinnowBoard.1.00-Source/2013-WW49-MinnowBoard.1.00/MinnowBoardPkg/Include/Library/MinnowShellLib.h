/** @file
  Missing portion of ShellLib

  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __MINNOW_SHELL_LIB__
#define __MINNOW_SHELL_LIB__

/**
  Function to print help file / man page content in the spec from the UEFI Shell protocol GetHelpText function.

  @param[in] CommandToGetHelpOn  Pointer to a string containing the command name of help file to be printed.
  @param[in] SectionToGetHelpOn  Pointer to the section specifier(s).
  @param[in] PrintCommandText    If TRUE, prints the command followed by the help content, otherwise prints 
                                 the help content only.
  @retval EFI_DEVICE_ERROR       The help data format was incorrect.
  @retval EFI_NOT_FOUND          The help data could not be found.
  @retval EFI_SUCCESS            The operation was successful.
**/
EFI_STATUS
EFIAPI
ShellPrintHelp (
  IN CONST CHAR16     *CommandToGetHelpOn,
  IN CONST CHAR16     *SectionToGetHelpOn,
  IN BOOLEAN          PrintCommandText
  );

#endif // __MINNOW_SHELL_LIB__