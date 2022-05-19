/*
 * @(#)graphics.h
 *
 * Copyright 1999-2000, Aaron Ardiri (mailto:aaron@ardiri.com)
 * All rights reserved.
 *
 * This file was generated as part of the "hoards" program developed for 
 * the Palm Computing Platform designed by Palm: 
 *
 *   http://www.palm.com/ 
 *
 * The contents of this file is confidential and proprietrary in nature 
 * ("Confidential Information"). Redistribution or modification without 
 * prior consent of the original author is prohibited. 
 */

#ifndef _GRAPHICS_H
#define _GRAPHICS_H

#include "palm.h"

extern Boolean   GraphicsInitialize()                               __DEVICE__;
extern WinHandle GraphicsGetDrawWindow()                            __DEVICE__;
extern void      GraphicsClear()                                    __DEVICE__;
extern void      GraphicsClearLCD()                                 __DEVICE__;
extern void      GraphicsRepaint()                                  __DEVICE__;
extern void      GraphicsTerminate()                                __DEVICE__;

#endif 
