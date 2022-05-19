/*
 * @(#)help_en.c
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
 *
 * --------------------------------------------------------------------
 *             THIS FILE CONTAINS THE ENGLISH LANGUAGE TEXT
 * --------------------------------------------------------------------
 */

#include "palm.h"

// interface
static UInt32 mainHelpInitialize() __HELP__;
static UInt32 editHelpInitialize() __HELP__;
static UInt32 gameHelpInitialize() __HELP__;

typedef struct 
{
  UInt32    keyMask;
  WinHandle helpWindow;
} HelpGlobals;

/**
 * Initialize the instructions screen.
 * 
 * @return the height in pixels of the instructions data area.
 */
UInt16
InitInstructions()
{
  HelpGlobals *globals;
  UInt32      formID;
  UInt16      result = 0;
  DmOpenRef   resourceRef;

  // create the globals object, and register it
  globals = (HelpGlobals *)MemPtrNew(sizeof(HelpGlobals));
  MemSet(globals, sizeof(HelpGlobals), 0);
  FtrSet(appCreator, ftrHelpGlobals, (UInt32)globals);

  // setup the valid keys available at this point in time
  globals->keyMask = KeySetMask(~(keyBitsAll ^ 
                                 (keyBitPower   | keyBitCradle   |
                                  keyBitPageUp  | keyBitPageDown |
                                  keyBitAntenna | keyBitContrast)));

  // initialize
  globals->helpWindow = NULL;

  // find out which help window we need to open up :)
  FtrGet(appCreator, ftrHelpGlobalsActiveForm, (UInt32 *)&formID);

  // open the application's resource database
  {
    UInt16  card;
    LocalID dbID;

    SysCurAppDatabase(&card, &dbID);
    resourceRef = DmOpenDatabase(card, dbID, dmModeReadOnly);
  }

  // what form are we dealing with? (different screens require different help)
  switch (formID)
  {
    case mainForm:
         result = mainHelpInitialize();
         break;      

    case editForm:
         result = editHelpInitialize();
         break;      

    case gameForm:
         result = gameHelpInitialize();
         break;      

    default:
         break;      
  }

  // close the database we just opened
  DmCloseDatabase(resourceRef);

  // did something go wrong??
  if (globals->helpWindow == NULL) 
  {
    result = 0;
    ApplicationDisplayDialog(xmemForm);
  }

  return result;
}

/**
 * Draw the instructions on the screen.
 * 
 * @param offset the offset height of the window to start copying from.
 */
void 
DrawInstructions(UInt16 offset)
{
  const RectangleType helpArea = {{0,offset},{142,116}};
  HelpGlobals *globals;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&globals);

  // blit the required area
  WinCopyRectangle(globals->helpWindow, 
                   WinGetDrawWindow(), &helpArea, 3, 16, winPaint);
}

/**
 * Terminate the instructions screen.
 */
void
QuitInstructions()
{
  HelpGlobals *globals;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&globals);

  // return the state of the key processing
  KeySetMask(globals->keyMask);

  // clean up memory
  if (globals->helpWindow != NULL)
    WinDeleteWindow(globals->helpWindow, false);
  MemPtrFree(globals);

  // unregister global data
  FtrUnregister(appCreator, ftrHelpGlobals);
}

/**
 * Initialize the help screen for the mainForm.
 *
 * @return the height in pixels of the mainForm help data area.
 */
static UInt32
mainHelpInitialize()
{
  const RectangleType     rect  = {{0,0},{142,207}};
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  HelpGlobals *globals;
  Err         err;
  UInt32      result = 0;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&globals);

  // create the offscreen window
  globals->helpWindow = 
    WinCreateOffscreenWindow(rect.extent.x, rect.extent.y,screenFormat,&err);
  err |= (globals->helpWindow == NULL);

  // if the window creation was successful, draw the help
  if (err == errNone) {

    FontID    font;
    WinHandle winCurrent;

    winCurrent = WinGetDrawWindow();
    font       = FntGetFont();

    // draw to help window
    WinSetDrawWindow(globals->helpWindow);
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0); // clear background

    {
      Char   *str;
      Char   *ptrStr;
      UInt16 x, y;

      // initialize
      str = (Char *)MemPtrNew(512 * sizeof(Char));
      y = 2;

      // draw title
      StrCopy(str, "Hoards of the Deep Realm");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();
      WinSetUnderlineMode(noUnderline);

      // add space (little)
      y += FntLineHeight() >> 1;

      // general text
      x = 4;
      StrCopy(str,
"Select the level set you wish to use from the list presented.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);
	
	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // show the little list sample
      x = 0;
      {
        MemHandle bitmapHandle = DmGet1Resource('Tbmp', bitmapHelpSelectList);
        WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), x, y);
        MemHandleUnlock(bitmapHandle);
        DmReleaseResource(bitmapHandle);
      }

      // add space (little)
      y += 48 + (FntLineHeight() >> 1);

      // explain what you do with the level selected
      x = 4;
      StrCopy(str,
"You can create, play, edit or delete level sets and configure the keypad \
controls to your liking using the menu options.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // describe the context-sensitive help
      x = 4;
      StrCopy(str,
"Use the context-sensitive help! Many more information pages are available \
depending on your position in the application!");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "GOOD LUCK!");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y += FntLineHeight();

      // clean up
      MemPtrFree(str);
    }

    // restore active window
    WinSetDrawWindow(winCurrent);
    FntSetFont(font);

    result = rect.extent.y;
  }
  
  return result;
}

/**
 * Initialize the help screen for the editForm.
 *
 * @return the height in pixels of the editForm help data area.
 */
static UInt32
editHelpInitialize()
{
  const RectangleType     rect  = {{0,0},{142,249}};
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  HelpGlobals *globals;
  Err         err;
  UInt32      result = 0;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&globals);

  // create the offscreen window
  globals->helpWindow = 
    WinCreateOffscreenWindow(rect.extent.x, rect.extent.y,screenFormat,&err);
  err |= (globals->helpWindow == NULL);

  // if the window creation was successful, draw the help
  if (err == errNone) {

    FontID    font;
    WinHandle winCurrent;

    winCurrent = WinGetDrawWindow();
    font       = FntGetFont();

    // draw to help window
    WinSetDrawWindow(globals->helpWindow);
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0); // clear background

    {
      Char   *str;
      Char   *ptrStr;
      UInt16 x, y;

      // initialize
      str = (Char *)MemPtrNew(512 * sizeof(Char));
      y = 2;

      // draw title
      StrCopy(str, "Hoards Level Editor");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();
      WinSetUnderlineMode(noUnderline);

      // add space (little)
      y += FntLineHeight() >> 1;

      // explain the level editing process
      x = 4;
      StrCopy(str,
"Using the stylus, select a tool from the tool bar positioned at the \
bottom of the screen.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str,
"Tapping or dragging the stylus draws/inverts the level tiles.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // draw legend title
      StrCopy(str, "TILE LEGEND");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();
      WinSetUnderlineMode(noUnderline);
      FntSetFont(font);

      // add space (little)
      y += FntLineHeight() >> 1;

      // editor tiles + descriptions
      x = 0;
      {
        MemHandle bitmapHandle = DmGet1Resource('Tbmp', bitmapHelpEditTiles);
        WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), x, y);
        MemHandleUnlock(bitmapHandle);
        DmReleaseResource(bitmapHandle);
      }

      // add space
      y += 80 + (FntLineHeight() >> 1);

      // explain "problems"
      x = 4;
      StrCopy(str,
"User level sets are not verified.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, 
"Create level sets to share with the entire world and keep the \
Hoards tradition alive!");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // draw site link
      StrCopy(str, "http://www.ardiri.com/");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // clean up
      MemPtrFree(str);
    }

    // restore active window
    WinSetDrawWindow(winCurrent);
    FntSetFont(font);

    result = rect.extent.y;
  }
  
  return result;
}

/**
 * Initialize the help screen for the gameForm.
 *
 * @return the height in pixels of the gameForm help data area.
 */
static UInt32
gameHelpInitialize()
{
  const RectangleType     rect  = {{0,0},{142,376}};
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};
  HelpGlobals *globals;
  Err         err;
  UInt32      result = 0;

  // get globals reference
  FtrGet(appCreator, ftrHelpGlobals, (UInt32 *)&globals);

  // create the offscreen window
  globals->helpWindow = 
    WinCreateOffscreenWindow(rect.extent.x, rect.extent.y,screenFormat,&err);
  err |= (globals->helpWindow == NULL);

  // if the window creation was successful, draw the help
  if (err == errNone) {

    FontID    font;
    WinHandle winCurrent;

    winCurrent = WinGetDrawWindow();
    font       = FntGetFont();

    // draw to help window
    WinSetDrawWindow(globals->helpWindow);
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0); // clear background

    {
      Char   *str;
      Char   *ptrStr;
      UInt16 x, y;

      // initialize
      str = (Char *)MemPtrNew(512 * sizeof(Char));
      y = 2;

      // draw title
      StrCopy(str, "Game Overview");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();
      FntSetFont(font);
      WinSetUnderlineMode(noUnderline);

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Help John Bandit collect all the gold in a level without dying.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, 
"Armed with a vapourizer, guide John through each level blasting holes \
in turf for trapping monks, creating routes for escape or paths to buried \
objects.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str,
"When all gold is collected, extra routes may be shown. Guide \
John to the top of the screen to continue to the next level.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, 
"Monks patrol each level and getting caught by one results in death. \
They can pick up objects and may drop them or need to be trapped to \
give them up. They can be killed if trapped for too long and will \
re-appear.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // draw title
      StrCopy(str, "Player Controls");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;

      WinSetUnderlineMode(grayUnderline);
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();
      FntSetFont(font);
      WinSetUnderlineMode(noUnderline);

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Game options are configurable.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "[ Using Keys ]");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Use the six keypad controls to control the movement of John.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "[ Using Stylus ]");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Tap the location you want John to move to. To dig holes tap the brick you \
want removed.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      // draw site link
      StrCopy(str, "http://www.ardiri.com/");
      FntSetFont(boldFont);
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // clean up
      MemPtrFree(str);
    }

    // restore active window
    WinSetDrawWindow(winCurrent);
    FntSetFont(font);

    result = rect.extent.y;
  }
  
  return result;
}
