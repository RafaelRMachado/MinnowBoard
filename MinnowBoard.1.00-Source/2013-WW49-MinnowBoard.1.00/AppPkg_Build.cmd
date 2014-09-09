@REM @file
@REM   Windows batch file to build AppPkg for the MinnowBoard.
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

build  -a IA32  -a X64  -t VS2008x86  -p AppPkg/AppPkg.dsc  -b RELEASE  %*
