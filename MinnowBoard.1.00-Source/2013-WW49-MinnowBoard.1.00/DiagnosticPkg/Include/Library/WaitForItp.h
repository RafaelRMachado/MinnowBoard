/** @file
  Wait for ITP declaration
  
  Copyright (c) 2013, Intel Corporation
  All rights reserved. This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef _WAIT_FOR_ITP_H_
#define _WAIT_FOR_ITP_H_

/**
  Wait for the ITP to break the execution loop

  @param[in] LoopForever  Address of a boolean indicating if the routine should
                          loop forever

**/
VOID
SecWaitForItp (
  IN volatile BOOLEAN *LoopForever
  );

#endif  //  _WAIT_FOR_ITP_H_
