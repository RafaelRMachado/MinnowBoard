#!/bin/bash
#
# Copyright (c) 2006 - 2010, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
build -p AppPkg/AppPkg.dsc -b RELEASE -a IA32 -a X64 -t GCC44   -D DEBUG_ENABLE_OUTPUT=TRUE   -D DEBUG_PROPERTY_MASK=0x27   -D DEBUG_PRINT_ERROR_LEVEL=0xfcf00040