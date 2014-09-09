#
# Copyright (c) 2013, Intel Corporation. All rights reserved.<BR>
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
cd CryptoPkg/Library/OpensslLib
wget http://www.openssl.org/source/openssl-0.9.8w.tar.gz
tar -xf openssl-0.9.8w.tar.gz
cd openssl-0.9.8w
patch -p0 -i ../EDKII_openssl-0.9.8w.patch
cd ..
./Install.sh
cd ../../..

