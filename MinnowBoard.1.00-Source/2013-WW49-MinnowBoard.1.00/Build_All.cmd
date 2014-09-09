@REM @file
@REM   Windows batch file to build the tree for the MinnowBoard.
@REM
@REM Copyright (c) 2012 - 2013, Intel Corporation. All rights reserved.<BR>
@REM This program and the accompanying materials
@REM are licensed and made available under the terms and conditions of the BSD License
@REM which accompanies this distribution.  The full text of the license may be found at
@REM http://opensource.org/licenses/bsd-license.php
@REM
@REM THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
@REM WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
@REM

Title Clean
Call Bsd_And_Binary_Build    -clean
@rem
Title BSD and Binary Build  Debug
Call Bsd_And_Binary_Build    -d32
@rem
Title BSD and Binary Build  Release
Call Bsd_And_Binary_Build    -r32
@rem
Title Edit
