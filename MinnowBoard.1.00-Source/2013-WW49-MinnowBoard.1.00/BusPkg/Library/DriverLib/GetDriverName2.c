/** @file
  Common GetDriverName2 implementation

  Copyright (c) 2011 - 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Library/DriverLib.h>

/**
  Retrieves a string that is the user readable name of
  the EFI Driver.

  @param  This       A pointer to the
                     EFI_COMPONENT_NAME2_PROTOCOL instance.
  
  @param  Language   A pointer to a Null-terminated ASCII string
                     array indicating the language. This is the
                     language of the driver name that the caller
                     is requesting, and it must match one of the
                     languages specified in SupportedLanguages.
                     The number of languages supported by a
                     driver is up to the driver writer. Language
                     is specified in RFC 4646 language code
                     format.
  
  @param  DriverName A pointer to the string to return.
                     This string is the name of the
                     driver specified by This in the language
                     specified by Language.

  @retval EFI_SUCCESS           The string for the
                                Driver specified by This and the
                                language specified by Language
                                was returned in DriverName.
  
  @retval EFI_INVALID_PARAMETER Language is NULL.
  
  @retval EFI_INVALID_PARAMETER DriverName is NULL.
  
  @retval EFI_UNSUPPORTED       The driver specified by This
                                does not support the language
                                specified by Language.

**/
EFI_STATUS
DlGetDriverName2 (
  IN EFI_COMPONENT_NAME2_PROTOCOL          *This,
  IN  CHAR8                                *Language,
  OUT CHAR16                               **DriverName
  )
{
  EFI_STATUS Status;

  Status = LookupUnicodeString2 (
             Language,
             This->SupportedLanguages,
             mDriverLib.DriverNameStringTable,
             DriverName,
             FALSE
             );

  //
  //  Return the lookup status
  //
DEBUG (( DEBUG_ERROR, "DlGetDriverName2 exiting, Status: %r\r\n", Status ));
  return Status;
}
