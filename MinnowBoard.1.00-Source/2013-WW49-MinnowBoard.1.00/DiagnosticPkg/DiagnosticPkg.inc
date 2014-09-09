################################################################
# Boilerplate text to be included by any DSC file using the
# Diagnostic Package.
#
#  Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
#  This program and the accompanying materials
#  are licensed and made available under the terms and conditions of the BSD License
#  which accompanies this distribution. The full text of the license may be found at
#  http://opensource.org/licenses/bsd-license.
#
#  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
################################################################

[LibraryClasses]
  WaitForItpLib|DiagnosticPkg/Library/WaitForItpLib/WaitForItpLib.inf

[Components.common]
  DiagnosticPkg/DiagOutputResetPei/DiagOutputResetPei.inf
  DiagnosticPkg/DiagUartOutputPei/DiagUartOutputPei.inf

