/*
 * @(#)help_de.c
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
 *             THIS FILE CONTAINS THE GERMAN LANGUAGE TEXT
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

  // find out which help window we need to open up
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
  const RectangleType     rect  = {{0,0},{142,229}};
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
"Wählen Sie den gewünschten Level von der Liste aus.");
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
"Sie können durch die Menü Optionen Level entwickeln, spielen, bearbeiten oder \
löschen. Auch die Tastenbelegung können Sie dort konfigurieren.");
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
"Benutzen Sie die Kontextsensitive Hilfe! Weitere Informationen sind so \
erhältlich, abhängig von Ihrer jeweiligen Spielposition.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "Viel Erfolg!");
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
  const RectangleType     rect  = {{0,0},{142,260}};
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
"Benutzen Sie den Stylus um ein Werkzeug aus der Liste am unterem \
Bildschirm auszuwählen.");
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
"Tippen oder ziehen mit dem Stylus zeichnet oder invertiert die Level Stufen.");
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
"Benutzer Level Einstellungen sind nicht verifiziert.");
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
"Erschaffen Sie Level und teilen Sie diese mit anderen.");
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
  const RectangleType     rect  = {{0,0},{142,464}};
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
      StrCopy(str, "Spiel Überblick");
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
"Helfen Sie John Bandit alles Gold in einem Level zu sammeln ohne zu sterben.");
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
"Bewaffnet mit einem Vapourizer führen Sie John durch jeden Level, \
schiessen Sie Löcher in den Torf, schaffen Sie Fluchtwege oder \
Wege zu vergrabenen Objekten.");
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
"Wenn alles Gold eingesammelt ist, erscheinen weitere Wege. Führen Sie \
John zum oberen Bildschirmrand um in den nächsten Level zu wechseln.");
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
"Mönche überwachen jeden Level und von ihnen gefangen zu werden bedeutet \
den Tod. Sie können Objekte aufnehmen und wegwerfen. Geben die Mönche \
durch von Ihnen gestellte Fallen Dinge auf, können Sie getötet werden. \
Aber Vorsicht, Sie kommen zurück.");
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
      StrCopy(str, "Spiel Einstellungen");
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
"Spiel Optionen sind konfigurierbar.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "[ Tasten benutzen ]");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Benutzen Sie die sechs Tasten um Johns Bewegungen zu kontrollieren.");
      ptrStr = str;
      while (StrLen(ptrStr) != 0) {
        UInt8 count = FntWordWrap(ptrStr, rect.extent.x-x);

	x = (rect.extent.x - FntCharsWidth(ptrStr, count)) >> 1;
	WinDrawChars(ptrStr, count, x, y); y += FntLineHeight(); x = 4;

	ptrStr += count;
      }

      // add space (little)
      y += FntLineHeight() >> 1;

      StrCopy(str, "[ Stylus benutzen ]");
      x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1;
      WinDrawChars(str, StrLen(str), x, y); y+= FntLineHeight();

      // add space (little)
      y += FntLineHeight() >> 1;

      x = 4;
      StrCopy(str, 
"Tippen Sie auf den Ort, wohin sich John bewegen soll. Um Löcher \
zu graben tippen Sie auf den Stein der entfernt werden soll.");
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
