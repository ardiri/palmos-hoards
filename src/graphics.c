/*
 * @(#)graphics.c
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

#include "palm.h"

// interface
static void GraphicsClear_asm()                                     __DEVICE__;
static void GraphicsClearLCD_asm()                                  __DEVICE__;
static void GraphicsRepaint_asm()                                   __DEVICE__;
static void GraphicsClear_api()                                     __DEVICE__;
static void GraphicsClearLCD_api()                                  __DEVICE__;
static void GraphicsRepaint_api()                                   __DEVICE__;

// global variable structure
typedef struct
{
  UInt32    scrDepth;
  Boolean   scr160x160;
  WinHandle drawWindow;

  void      (*fnClear)(void);
  void      (*fnClearLCD)(void);
  void      (*fnRepaint)(void);
} GraphicsGlobals;

/**
 * Initialize the Graphics engine.
 *
 * @return true if the graphics initialization was successful, false otherwise
 */  
Boolean   
GraphicsInitialize()
{
  GraphicsGlobals *globals;
  Boolean         result = true;
  UInt16          err;

  // create the globals object, and register it
  globals = (GraphicsGlobals *)MemPtrNew(sizeof(GraphicsGlobals));
  MemSet(globals, sizeof(GraphicsGlobals), 0);
  FtrSet(appCreator, ftrGraphicsGlobals, (UInt32)globals);

  // determine the screen depth
  globals->scrDepth   = 1;
  if (DeviceSupportsVersion(romVersion3)) {
    WinScreenMode(winScreenModeGet,NULL,NULL,&globals->scrDepth,NULL);
  }

  // determine screen size (only >= 3.5)
  globals->scr160x160 = true;
  if (DeviceSupportsVersion(romVersion3_5)) {

    UInt32 width, height;
    UInt32 cpuID;

    // determine the current display mode
    WinScreenMode(winScreenModeGetDefaults,&width,&height,NULL,NULL);
    globals->scr160x160 &= ((width == 160) && (height == 160));

    // lets make sure we are running on a "m68k chip" :)
    FtrGet(sysFtrCreator, sysFtrNumProcessorID, &cpuID);
    globals->scr160x160 &= ((cpuID == sysFtrNumProcessor328) || 
                            (cpuID == sysFtrNumProcessorEZ)  || 
                            (cpuID == sysFtrNumProcessorVZ));
  }

  // create the offscreen window
  globals->drawWindow = 
    WinCreateOffscreenWindow(SCREEN_WIDTH, SCREEN_HEIGHT, screenFormat, &err);
  err |= (globals->drawWindow == NULL);

  // something went wrong, display error and exit
  if (err != errNone) {

    ApplicationDisplayDialog(xmemForm);
    result = false;
  }

  // no probs, configure pointers
  else {
    globals->fnClear    = (globals->scr160x160) 
                          ? (void *)GraphicsClear_asm
			  : (void *)GraphicsClear_api; 
    globals->fnClearLCD = (globals->scr160x160) 
                          ? (void *)GraphicsClearLCD_asm 
			  : (void *)GraphicsClearLCD_api; 
    globals->fnRepaint  = (globals->scr160x160) 
                          ? (void *)GraphicsRepaint_asm 
			  : (void *)GraphicsRepaint_api; 
  }

  return result;
}

/**
 * Get the draw window.
 *
 * @return the draw window.
 */
WinHandle
GraphicsGetDrawWindow()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  return globals->drawWindow;
}

/**
 * Clear the offscreen image.
 */
void
GraphicsClear()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // execute the appropriate function
  globals->fnClear();
}

/**
 * Clear the LCD display area (where the graphics area will go)
 */
void
GraphicsClearLCD()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // execute the appropriate function
  globals->fnClearLCD();
}

/**
 * Blit the offscreen image to the screen.
 */
void
GraphicsRepaint()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // execute the appropriate function
  globals->fnRepaint();
}

/**
 * Terminate the Graphics engine.
 */
void   
GraphicsTerminate()
{
  GraphicsGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // clean up windows/memory
  if (globals->drawWindow != NULL) 
    WinDeleteWindow(globals->drawWindow, false);
  MemPtrFree(globals);

  // unregister global data
  FtrUnregister(appCreator, ftrGraphicsGlobals);
}

// 
// 160x160 direct screen display routines [speed on 160x160 devices]
//
// -- Aaron Ardiri, 2000
//

/**
 * Clear the offscreen image.
 */
static void
GraphicsClear_asm()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // clear the buffer
  {
    UInt16 ptrSize;
    
    // how much memory is being used?
    ptrSize = ((SCREEN_WIDTH * SCREEN_HEIGHT) / (8 / globals->scrDepth));

    // clear the memory
    MemSet(DeviceWindowGetPointer(globals->drawWindow), ptrSize, 0);
  }
}

/**
 * Clear the LCD display area (where the graphics area will go)
 */
static void
GraphicsClearLCD_asm()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // blit the buffer to the real screen
  {
    UInt8 *ptrLCDScreen;
    Int16 loop;

    ptrLCDScreen = DeviceWindowGetPointer(WinGetDisplayWindow());
    ptrLCDScreen += ((SCREEN_WIDTH*16) / (8 / globals->scrDepth)); 
                                                             // 160x16 @ depth

    // determine how many "160" byte blits we need :))
    loop = ((SCREEN_WIDTH * SCREEN_HEIGHT) / (8 / globals->scrDepth) / 160);

    // push all registers (except a7) on stack
    asm("movem.l %%d0-%%d7/%%a0-%%a6, -(%%sp)" : : );

    // copy inner 160x144 from back buffer to screen
    asm("move.l  %0, %%a0" : : "g" (ptrLCDScreen));
    asm("move.l  %0, %%d0" : : "g" (loop-1));
    asm("
         move.l  #0, %%d1
         move.l  %%d1, %%d2
         move.l  %%d1, %%d3
         move.l  %%d1, %%d4
         move.l  %%d1, %%d5
         move.l  %%d1, %%d6
         move.l  %%d1, %%d7
         move.l  %%d1, %%a2
         move.l  %%d1, %%a3
         move.l  %%d1, %%a4
ScrLoop1:

         movem.l %%d1-%%d7/%%a2-%%a4, (%%a0)
         movem.l %%d1-%%d7/%%a2-%%a4, 40(%%a0)
         movem.l %%d1-%%d7/%%a2-%%a4, 80(%%a0)
         movem.l %%d1-%%d7/%%a2-%%a4, 120(%%a0)

         adda.l  #160, %%a0                      | blit 160 bytes!!

         dbra    %%d0, ScrLoop1
    " : :);

    // pop all registers (except a7) off stack
    asm("movem.l (%%sp)+, %%d0-%%d7/%%a0-%%a6" : : );
  }
}

/**
 * Blit the offscreen image to the screen.
 */
static void
GraphicsRepaint_asm()
{
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // blit the buffer to the real screen
  {
    UInt8 *ptrLCDScreen, *ptrScreen;
    Int16 loop;

    ptrLCDScreen = DeviceWindowGetPointer(WinGetDisplayWindow());
    ptrLCDScreen += ((SCREEN_WIDTH*16) / (8 / globals->scrDepth)); 
                                                             // 160x16 @ depth
    ptrScreen    = DeviceWindowGetPointer(globals->drawWindow);

    // determine how many "160" byte blits we need :))
    loop = ((SCREEN_WIDTH * SCREEN_HEIGHT) / (8 / globals->scrDepth) / 160);

    // push all registers (except a7) on stack
    asm("movem.l %%d0-%%d7/%%a0-%%a6, -(%%sp)" : : );

    // copy inner 160x144 from back buffer to screen
    asm("move.l  %0, %%a0" : : "g" (ptrScreen));
    asm("move.l  %0, %%a1" : : "g" (ptrLCDScreen));
    asm("move.l  %0, %%d0" : : "g" (loop-1));
    asm("
ScrLoop2:
         movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a4
         movem.l %%d1-%%d7/%%a2-%%a4, (%%a1)

         movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a4
         movem.l %%d1-%%d7/%%a2-%%a4, 40(%%a1)

         movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a4
         movem.l %%d1-%%d7/%%a2-%%a4, 80(%%a1)

         movem.l (%%a0)+, %%d1-%%d7/%%a2-%%a4
         movem.l %%d1-%%d7/%%a2-%%a4, 120(%%a1)

         adda.l  #160, %%a1                      | blit 160 bytes!!

         dbra    %%d0, ScrLoop2
    " : :);

    // pop all registers (except a7) off stack
    asm("movem.l (%%sp)+, %%d0-%%d7/%%a0-%%a6" : : );
  }
}

// 
// 160x160 API call display routines [for future compatability]
//
// -- Aaron Ardiri, 2000
//

/**
 * Clear the offscreen image.
 */
static void
GraphicsClear_api()
{
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  const RectangleType     rect  = {{0,0},{SCREEN_WIDTH,SCREEN_HEIGHT}};
  GraphicsGlobals *globals;
  WinHandle       currWindow;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // clear the buffer
  currWindow = WinSetDrawWindow(globals->drawWindow);
  WinSetPattern(&erase);
  WinFillRectangle(&rect, 0);
  WinSetDrawWindow(currWindow);
}

/**
 * Clear the LCD display area (where the graphics area will go)
 */
static void
GraphicsClearLCD_api()
{
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  const RectangleType     rect  = {{0,16},{SCREEN_WIDTH,SCREEN_HEIGHT}};
  
  // clear the buffer
  WinSetPattern(&erase);
  WinFillRectangle(&rect, 0);
}

/**
 * Blit the offscreen image to the screen.
 */
static void
GraphicsRepaint_api()
{
  const RectangleType rect = {{0,0},{SCREEN_WIDTH,SCREEN_HEIGHT}};
  GraphicsGlobals *globals;
  
  // get a globals reference
  FtrGet(appCreator, ftrGraphicsGlobals, (UInt32 *)&globals);

  // copy the backbuffer to the screen
  WinCopyRectangle(globals->drawWindow, WinGetDisplayWindow(),
                   &rect, 0, 16, winPaint);
}
