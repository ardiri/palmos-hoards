/*
 * @(#)palm.c
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

typedef struct
{
  PreferencesType *prefs;

  Int32           evtTimeOut;
  Int16           timerDiff;
  Int16           ticksPerFrame;
  UInt32          timerPointA;
  UInt32          timerPointB;
  Char            **dataList1;
  Char            **dataList2;
  UInt16          dataListCount;          
  UInt16          dataIndex;          

  UInt8           lgray;
  UInt8           dgray;
} Globals;

// interface
static Boolean mainFormEventHandler(EventType *);
static Boolean editFormEventHandler(EventType *);
static Boolean gameFormEventHandler(EventType *);
static Boolean infoFormEventHandler(EventType *);
static Boolean cfigFormEventHandler(EventType *);
static Boolean grayFormEventHandler(EventType *);
#ifdef PROTECTION_ON
static Boolean regiFormEventHandler(EventType *);
static Boolean rbugFormEventHandler(EventType *);
#endif
static Boolean highFormEventHandler(EventType *);
static Boolean helpFormEventHandler(EventType *);
static Boolean xmemFormEventHandler(EventType *);
static Boolean cr8rFormEventHandler(EventType *);
static Boolean moveFormEventHandler(EventType *);
static Boolean gotoFormEventHandler(EventType *);
static Boolean xlevFormEventHandler(EventType *);

/**
 * The Form:mainForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
mainFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           FormType *frm = FrmGetActiveForm();
           UInt16   i, menuID;

           // dynamically adjust the menu :)
           menuID = mainMenu_nogray_nobeam;    // fail safe :)
           if (DeviceSupportsGrayscale()) {
             if (DeviceSupportsBeaming()) menuID = mainMenu_gray;
             else                         menuID = mainMenu_gray_nobeam;
           }
           else {
             if (DeviceSupportsBeaming()) menuID = mainMenu_nogray;
             else                         menuID = mainMenu_nogray_nobeam;
           }
           FrmSetMenu(frm, menuID);
           FrmDrawForm(frm);

           globals->dataList1 = 
             (Char **)MemPtrNew(MAX_LISTITEMS * sizeof(Char *));
           globals->dataList2 = 
             (Char **)MemPtrNew(MAX_LISTITEMS * sizeof(Char *));
           for (i=0; i<MAX_LISTITEMS; i++) {
             globals->dataList1[i] = (Char *)MemPtrNew(32 * sizeof(Char));
             globals->dataList2[i] = (Char *)MemPtrNew(32 * sizeof(Char));
           }

           globals->dataListCount = 0;
           globals->dataIndex     = noListSelection;

           // update the form (draw stuff)
           {
             EventType event;
             
             MemSet(&event, sizeof(EventType), 0);
             event.eType = frmUpdateEvent;
             event.data.frmUpdate.formID = FrmGetActiveFormID();
             EvtAddEventToQueue(&event);
           }
         }
         processed = true;
         break;

    case frmUpdateEvent:
         FrmDrawForm(FrmGetActiveForm());

         // generate the level sets list
         {
           EventType event;
             
           MemSet(&event, sizeof(EventType), 0);
           event.eType = appGenerateLevelSetList;
           EvtAddEventToQueue(&event);
         }

         RegisterShowMessage(globals->prefs);
         processed = true;
         break;

    case appGenerateLevelSetList:
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           UInt16        i, count;
           UInt16        val, min, max, pge;

           globals->dataIndex = noListSelection;
           count              = 0;
           {
             DmSearchStateType stateInfo;
             Err               error;
             UInt16            card, pos;
             LocalID           dbID;
             UInt32            dbRecCount;
             Char              strDatabaseName[32];
             Char              strDatabaseSize[16];

             error =
               DmGetNextDatabaseByTypeCreator(true, &stateInfo,
                                              levelType, appCreator, false,
                                              &card, &dbID);

             while ((error == 0) && (count < MAX_LISTITEMS)) {
  
               // extract the database information
               DmDatabaseInfo(card, dbID, strDatabaseName, 
                              NULL, NULL, NULL, NULL, NULL,
                              NULL, NULL, NULL, NULL, NULL);
               DmDatabaseSize(card, dbID, &dbRecCount, NULL, NULL);
  
               // strip the "LODE_" from the name
               i = 0;
               while ((strDatabaseName[i+5] != '\0')) {
                 strDatabaseName[i] = strDatabaseName[i+5];
                 i++;
               }
               strDatabaseName[i] = '\0';
  
               // determine the size (round to 8K chunks)
               StrPrintF(strDatabaseSize, "%d", (Int16)dbRecCount);
  
               // find out where it should go
               pos = 0;
               while ((pos < count) &&
                      (StrCompare(strDatabaseName, 
                                  globals->dataList1[pos]) > 0)) {
                 pos++;
               }

               // do we need to shift a few things?
               if (pos < count) {

                 // move em down
                 for (i=count; i>pos; i--) {
                   StrCopy(globals->dataList1[i], globals->dataList1[i-1]);
                   StrCopy(globals->dataList2[i], globals->dataList2[i-1]);
                 }
               }

               // copy it into the list
               StrCopy(globals->dataList1[pos], strDatabaseName);
               StrCopy(globals->dataList2[pos], strDatabaseSize);

               // next sequence
               error =
                 DmGetNextDatabaseByTypeCreator(false, &stateInfo,
                                                levelType, appCreator, false,
                                                &card, &dbID);
               count++;
             }

             // if some databases available, select the first
             if (count != 0) globals->dataIndex = 0;
           }
           globals->dataListCount = count;

           frm = FrmGetActiveForm();

           // adjust the "scrollbar"
           sclBar =  
             (ScrollBarType *)FrmGetObjectPtr(frm, 
               FrmGetObjectIndex(frm, mainFormScrollBar)); 

           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
           max = (count > pge) ? count - pge : 0;
           SclSetScrollBar(sclBar, 0, min, max, pge);

           // make sure the "freespaces" are empty! :)
           for (; count<MAX_LISTITEMS; count++) {
             MemSet(globals->dataList1[count], 32 * sizeof(Char), 0);
             MemSet(globals->dataList2[count], 32 * sizeof(Char), 0);
           }

           // redraw the display
           {
             EventType event;
             
             MemSet(&event, sizeof(EventType), 0);
             event.eType = appRedrawEvent;
             EvtAddEventToQueue(&event);
           }
         }
         processed = true;
         break;

    case appRedrawEvent:
         {
           const CustomPatternType erase = {0,0,0,0,0,0,0,0};
           FormType      *frm;
           ScrollBarType *sclBar;
           RectangleType rctBounds, rctClip;
           UInt16        i, x, y;
           UInt16        val, min, max, pge;

           frm = FrmGetActiveForm();
           WinGetClip(&rctClip);

           // draw seperators
           WinDrawLine(   0,  71, 159,  71);
           WinDrawLine(   0,  72, 159,  72);

           WinDrawLine(   0, 145, 159, 145);
           WinDrawLine(   0, 146, 159, 146);

           // get the "scrollbar" values
           sclBar =  
             (ScrollBarType *)FrmGetObjectPtr(frm, 
               FrmGetObjectIndex(frm, mainFormScrollBar)); 
           SclGetScrollBar(sclBar, &val, &min, &max, &pge);

           // draw the "list"
           FrmGetObjectBounds(frm,
             FrmGetObjectIndex(frm, mainFormGameGadget), &rctBounds);
           WinSetClip(&rctBounds);
           WinSetPattern(&erase);
           WinFillRectangle(&rctBounds, 0);

           x = rctBounds.topLeft.x; 
           y = rctBounds.topLeft.y; 
           for (i=val; i<globals->dataListCount; i++) {

             x = rctBounds.topLeft.x + 4; 
             WinDrawChars(globals->dataList1[i], 
                          StrLen(globals->dataList1[i]), x, y);

             x = rctBounds.topLeft.x +
                 rctBounds.extent.x - 4 -
                 FntCharsWidth(globals->dataList2[i], 
                               StrLen(globals->dataList2[i])); 
             WinDrawChars(globals->dataList2[i], 
                          StrLen(globals->dataList2[i]), x, y);

             y += FntLineHeight();
           }

           // invert the "selected" item
           rctBounds.topLeft.y += FntLineHeight() * 
                                  (globals->dataIndex - val);
           rctBounds.extent.y   = FntLineHeight();
           WinInvertRectangle(&rctBounds, 0); 

           WinSetClip(&rctClip);
         }
         processed = true;
         break;

    case menuEvent:

         // erase the menu status
         MenuEraseStatus(MenuGetActiveMenu());

         // what menu?
         switch (event->data.menu.itemID)
         {
           case mainMenuItemPlay:

                // lets make sure there is a selection :)
                if ((globals->dataIndex != noListSelection) &&
                    (StrCompare(globals->dataList2[globals->dataIndex], "0") != 0)) {

                  // we need to know the selected game 
                  StrPrintF(globals->prefs->game.strLevelSetName, "LODE_%s", 
                            globals->dataList1[globals->dataIndex]);
                  FrmGotoForm(gameForm);
                }
                else 
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case mainMenuItemBeam:
                
                // lets make sure there is a selection :)
                if ((globals->dataIndex != noListSelection) &&
                    (StrCompare(globals->dataList2[globals->dataIndex], "0") != 0)) {

                  // beam the database [errors handled by Palm]
                  DeviceBeamDatabase(globals->dataList1[globals->dataIndex]);
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case mainMenuItemCreate:
                ApplicationDisplayDialog(cr8rForm);

                processed = true;
                break;

           case mainMenuItemEdit:

                // lets make sure there is a selection :)
                if (
                    (globals->dataIndex != noListSelection) &&
                    (
                     (StrCompare(globals->dataList1[globals->dataIndex], "ORIGINAL") != 0) ||
                     globals->prefs->config.cheatsEnabled
                    )) {

                  // we need to know the selected game 
                  StrPrintF(globals->prefs->editor.strLevelSetName, "LODE_%s", 
                            globals->dataList1[globals->dataIndex]);
                  FrmGotoForm(editForm);
                }
                else 
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case mainMenuItemDelete:

                // lets make sure there is a selection :)
                if (globals->dataIndex != noListSelection) {
                
                  Char    dbName[32];
                  UInt16  card;
                  LocalID dbID;

                  // build "search" information
                  card = 0;
                  StrPrintF(dbName, "LODE_%s", 
                            globals->dataList1[globals->dataIndex]);
                  dbID = DmFindDatabase(card, dbName);

                  // lets make sure!!!
                  if ((dbID != NULL) &&
                      (FrmCustomAlert(removeLevelSetAlert, 
                                      globals->dataList1[globals->dataIndex],
                                      NULL, NULL) == 0)) {

                    // kill the database
                    DmDeleteDatabase(card, dbID);

                    // generate the level set list
                    {
                      EventType event;
                    
                      MemSet(&event, sizeof(EventType), 0);
                      event.eType = appGenerateLevelSetList;
                      EvtAddEventToQueue(&event);
                    }
                  }
                }
                else 
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case mainMenuItemHigh:

                if CHECK_SIGNATURE(globals->prefs) {
                  ApplicationDisplayDialog(highForm);
                }
                else
                  SndPlaySystemSound(sndError);
                  
                processed = true;
                break;

           case mainMenuItemResetHigh:

                // lets make sure!!
                if (FrmAlert(resetHighScoresAlert) == 0) {
                  globals->prefs->game.gameHighScore = 0;
                  globals->prefs->game.gameHighLevel = 0;
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case penDownEvent:
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           RectangleType rctBounds, rctClip;
           UInt16        val, min, max, pge;

           frm = FrmGetActiveForm();
           WinGetClip(&rctClip);

           // get the "scrollbar" values
           sclBar =  
             (ScrollBarType *)FrmGetObjectPtr(frm, 
               FrmGetObjectIndex(frm, mainFormScrollBar)); 
           SclGetScrollBar(sclBar, &val, &min, &max, &pge);

           // get the "bounding" box of the gadget
           FrmGetObjectBounds(frm,
             FrmGetObjectIndex(frm, mainFormGameGadget), &rctBounds);
           WinSetClip(&rctBounds);

           // is the "pen" within the list?
           if (RctPtInRectangle(event->screenX, event->screenY, &rctBounds)) {

             UInt16 newValue;

             newValue = val +
                        (event->screenY - rctBounds.topLeft.y) / 
                        FntLineHeight();
             if (
                 (newValue < globals->dataListCount) &&
                 (newValue != globals->dataIndex)
                ) {

               // turn off the old value the "selected" item
               rctBounds.topLeft.y += FntLineHeight() * 
                                      (globals->dataIndex - val);
               rctBounds.extent.y   = FntLineHeight();
               WinInvertRectangle(&rctBounds, 0); 

               // invert the "selected" item
               rctBounds.topLeft.y += FntLineHeight() * 
                                      (newValue - globals->dataIndex);
               rctBounds.extent.y   = FntLineHeight();
               WinInvertRectangle(&rctBounds, 0); 

               globals->dataIndex = newValue;
             }
           }

           WinSetClip(&rctClip);
         }
         break;

    case keyDownEvent:

         switch (event->data.keyDown.chr)
         {
           case pageUpChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  RectangleType rctBounds, rctClip;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  WinGetClip(&rctClip);

                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, mainFormScrollBar));
                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);

                  // are we @ top? if so, move scroll bar up one
                  if (globals->dataIndex == val) {

                    // lets make sure we can do this
                    if (val > 0) {

                      // move up!
                      globals->dataIndex = --val;
                      SclSetScrollBar(sclBar, val, min, max, pge);

                      // redraw the display
                      {
                        EventType event;
             
                        MemSet(&event, sizeof(EventType), 0);
                        event.eType = appRedrawEvent;
                        EvtAddEventToQueue(&event);
                      }
                    }
                  }
 
                  // ok, just move "current" selection
                  else {

                    // get the "bounding" box of the gadget
                    FrmGetObjectBounds(frm,
                      FrmGetObjectIndex(frm, mainFormGameGadget), &rctBounds);
                    WinSetClip(&rctBounds);

                    // turn off the old value the "selected" item
                    rctBounds.topLeft.y += FntLineHeight() * 
                                           (globals->dataIndex - val);
                    rctBounds.extent.y   = FntLineHeight();
                    WinInvertRectangle(&rctBounds, 0); 

                    globals->dataIndex--;

                    // invert the "selected" item
                    rctBounds.topLeft.y -= FntLineHeight();
                    rctBounds.extent.y   = FntLineHeight();
                    WinInvertRectangle(&rctBounds, 0); 
                  }

                  WinSetClip(&rctClip);
                }
                processed = true;
                break;

           case pageDownChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  RectangleType rctBounds, rctClip;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  WinGetClip(&rctClip);

                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, mainFormScrollBar));
                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);

                  // are we @ bottom? if so, move scroll bar down one
                  if (globals->dataIndex == (val + (pge-1))) {

                    // lets make sure we can do this
                    if (val < max) {

                      // move down
                      val++; globals->dataIndex++; 
                      SclSetScrollBar(sclBar, val, min, max, pge);

                      // redraw the display
                      {
                        EventType event;
             
                        MemSet(&event, sizeof(EventType), 0);
                        event.eType = appRedrawEvent;
                        EvtAddEventToQueue(&event);
                      }
                    }
                  }
 
                  // ok, just move "current" selection
                  else 
                  if (globals->dataIndex < (globals->dataListCount-1)) {

                    // get the "bounding" box of the gadget
                    FrmGetObjectBounds(frm,
                      FrmGetObjectIndex(frm, mainFormGameGadget), &rctBounds);
                    WinSetClip(&rctBounds);

                    // turn off the old value the "selected" item
                    rctBounds.topLeft.y += FntLineHeight() * 
                                           (globals->dataIndex - val);
                    rctBounds.extent.y   = FntLineHeight();
                    WinInvertRectangle(&rctBounds, 0); 

                    globals->dataIndex++;

                    // invert the "selected" item
                    rctBounds.topLeft.y += FntLineHeight();
                    rctBounds.extent.y   = FntLineHeight();
                    WinInvertRectangle(&rctBounds, 0); 
                  }

                  WinSetClip(&rctClip);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case mainFormPlayButton:

                // we should "turn" it off :)) it is not a pushbutton here
                CtlSetValue(event->data.ctlSelect.pControl, 0);
                CtlDrawControl(event->data.ctlSelect.pControl);

                // regenerate menu event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = mainMenuItemPlay;
                  EvtAddEventToQueue(&event);
                }           
                processed = true;
                break;

           default:
                break;
         }
         break;

    case sclRepeatEvent:
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           UInt16        val, min, max, pge;

           frm = FrmGetActiveForm();
           sclBar = 
             (ScrollBarType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, mainFormScrollBar));

           SclGetScrollBar(sclBar, &val, &min, &max, &pge);

           // redraw the display
           {
             EventType event;
             
             MemSet(&event, sizeof(EventType), 0);
             event.eType = appRedrawEvent;
             EvtAddEventToQueue(&event);
           }
         }
         break;

    case frmCloseEvent:

         // clean up memory
         {
           UInt8 i;

           // free memory allocated
           for (i=0; i<MAX_LISTITEMS; i++) {
             MemPtrFree(globals->dataList1[i]);
             MemPtrFree(globals->dataList2[i]);
           }
           MemPtrFree(globals->dataList1);
           MemPtrFree(globals->dataList2);
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:editForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
editFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:

         // assume "default" tool is the empty one
         globals->prefs->editor.editTool.tool = toolEmpty;
         globals->prefs->editor.editTool.properties = 0;

         // initialize the editor environemnt
         if (EditorInitialize(globals->prefs->editor.strLevelSetName, true)) {

           // do we need to reset the count?
           if (!globals->prefs->editor.gameEditing) 
             globals->prefs->editor.currLevel = 1;

           // update the form (draw stuff)
           {
             EventType event;
             
             MemSet(&event, sizeof(EventType), 0);
             event.eType = frmUpdateEvent;
             event.data.frmUpdate.formID = FrmGetActiveFormID();
             EvtAddEventToQueue(&event);
           }
         }

         // oops.. get out
         else {
           globals->prefs->editor.gameEditing = false;
           FrmGotoForm(mainForm);
         }

         processed = true;
         break;

    case frmUpdateEvent:
         FrmDrawForm(FrmGetActiveForm());

         // hide the "no-levels" background
         GraphicsClear();
         GraphicsRepaint();

         // redraw the display
         {
           EventType event;
             
           MemSet(&event, sizeof(EventType), 0);
           event.eType = appRedrawEvent;
           EvtAddEventToQueue(&event);
         }
         processed = true;
         break;

    case appRedrawEvent:
         {
           FormType      *frm;
           FontID        currFont;
           RectangleType rect;
           Char          strLevel[8];

           // draw seperators
           WinDrawLine(   0, 145, 159, 145);
           WinDrawLine(   0, 146, 159, 146);

           frm = FrmGetActiveForm();
           FrmGetObjectBounds(frm,
             FrmGetObjectIndex(frm, editFormGotoButton), &rect);

           // case 1: no levels - draw "form"
           if (EditorLevelCount() == 0) {
             GraphicsClear(); GraphicsRepaint();
             FrmDrawForm(frm);
             globals->prefs->editor.currLevel = 0;
           }
           else

           // case 2: currently editing a level.. redraw it
           if (globals->prefs->editor.gameEditing) {
             EditorDrawLevel(&globals->prefs->editor.level);
           }

           // case 3: levels - draw first
           else {
             EditorLoadLevel(&globals->prefs->editor.level, 
                             globals->prefs->editor.currLevel);
             EditorDrawLevel(&globals->prefs->editor.level);
           }

           // we are now editing
           globals->prefs->editor.gameEditing = true;

           // update the "level" button to show the right level number
           StrPrintF(strLevel,
                     (globals->prefs->editor.currLevel < 10)  ? "00%d" :
                     (globals->prefs->editor.currLevel < 100) ? "0%d"  : "%d", 
                     globals->prefs->editor.currLevel); 
           currFont = FntGetFont();
           FntSetFont(boldFont);
           WinDrawChars(strLevel, StrLen(strLevel), 
                        rect.topLeft.x + 2, rect.topLeft.y);
           FntSetFont(currFont);

           // draw the selected tool
           {
             Coord x1, y1, x2, y2;

             FrmGetObjectBounds(frm,
               FrmGetObjectIndex(frm, 
                 editFormEmptyButton + 
                 globals->prefs->editor.editTool.tool), &rect);

             x1 = rect.topLeft.x - 1;
             y1 = rect.topLeft.y - 1;
             x2 = rect.topLeft.x + rect.extent.x;
             y2 = rect.topLeft.y + rect.extent.y;

             // draw rectangle outline
             WinDrawLine(x1, y1, x2, y1);
             WinDrawLine(x2, y1, x2, y2);
             WinDrawLine(x2, y2, x1, y2);
             WinDrawLine(x1, y2, x1, y1);
           }
         }
         processed = true;
         break;

    case menuEvent:

         // erase the menu status
         MenuEraseStatus(MenuGetActiveMenu());

         // what menu?
         switch (event->data.menu.itemID)
         {
           case editMenuItemSave:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 
                  EditorSaveLevel(&globals->prefs->editor.level, 
                                  globals->prefs->editor.currLevel);
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemRevert:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 
                  EditorLoadLevel(&globals->prefs->editor.level, 
                                  globals->prefs->editor.currLevel);
                  EditorDrawLevel(&globals->prefs->editor.level);
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemCreate:

                // we can only do this if there is room for a level
                if (EditorLevelCount() < 0xff) {
                  EditorCreateLevel(&globals->prefs->editor.level);
                  globals->prefs->editor.currLevel = EditorLevelCount();

                  // we are NO LONGER editing
                  globals->prefs->editor.gameEditing = false;

                  // redraw the display
                  {
                    EventType event;
           
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = appRedrawEvent;
                    EvtAddEventToQueue(&event);
                  }
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemNext:

                // only do this if we can :)
                if (globals->prefs->editor.currLevel < EditorLevelCount()) {
                  globals->prefs->editor.currLevel++;

                  // we are NO LONGER editing
                  globals->prefs->editor.gameEditing = false;

                  // redraw the display
                  {
                    EventType event;
           
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = appRedrawEvent;
                    EvtAddEventToQueue(&event);
                  }
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemPrev:

                // only do this if we can :)
                if (globals->prefs->editor.currLevel > 1) {
                  globals->prefs->editor.currLevel--;

                  // we are NO LONGER editing
                  globals->prefs->editor.gameEditing = false;

                  // redraw the display
                  {
                    EventType event;
           
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = appRedrawEvent;
                    EvtAddEventToQueue(&event);
                  }
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemGoto:
                if (EditorLevelCount() > 0) {
                  ApplicationDisplayDialog(gotoForm);
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemMove:
                ApplicationDisplayDialog(moveForm);

                processed = true;
                break;

           case editMenuItemDelete:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 

                  Char strLevel[8];
                  StrPrintF(strLevel,
                            (globals->prefs->editor.currLevel < 10)  ? "00%d" :
                            (globals->prefs->editor.currLevel < 100) ? "0%d"  : "%d", 
                            globals->prefs->editor.currLevel); 

                  // lets make sure!
                  if (FrmCustomAlert(removeLevelAlert, 
                                     strLevel, NULL, NULL) == 0) {

                    EditorDeleteLevel(globals->prefs->editor.currLevel);

                    // decrement the level if required
                    if (globals->prefs->editor.currLevel > EditorLevelCount()) 
                      globals->prefs->editor.currLevel--;

                    // redraw the display
                    {
                      EventType event;
           
                      MemSet(&event, sizeof(EventType), 0);
                      event.eType = appRedrawEvent;
                      EvtAddEventToQueue(&event);
                    }
                  }
                }
                else  
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemPanLeft:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 
                
                  UInt16 i,j;
                  for (j=0; j<GRID_HEIGHT; j++) {
                    for (i=0; i<(GRID_WIDTH-1); i++) {
                      globals->prefs->editor.level.tiles[i][j] = 
                        globals->prefs->editor.level.tiles[i+1][j];
                    }
                    globals->prefs->editor.level.tiles[GRID_WIDTH-1][j].tool = toolEmpty;
                    globals->prefs->editor.level.tiles[GRID_WIDTH-1][j].properties = 0;
                  }

                  EditorDrawLevel(&globals->prefs->editor.level);
                }
                else  
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemPanRight:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 

                  UInt16 i,j;
                  for (j=0; j<GRID_HEIGHT; j++) {
                    for (i=(GRID_WIDTH-1); i>0; i--) {
                      globals->prefs->editor.level.tiles[i][j] = 
                        globals->prefs->editor.level.tiles[i-1][j];
                    }
                    globals->prefs->editor.level.tiles[0][j].tool = toolEmpty;
                    globals->prefs->editor.level.tiles[0][j].properties = 0;
                  }

                  EditorDrawLevel(&globals->prefs->editor.level);
                }
                else  
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemPanUp:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 

                  UInt16 i,j;
                  for (i=0; i<GRID_WIDTH; i++) {
                    for (j=0; j<(GRID_HEIGHT-1); j++) {
                      globals->prefs->editor.level.tiles[i][j] = 
                        globals->prefs->editor.level.tiles[i][j+1];
                    }
                    globals->prefs->editor.level.tiles[i][GRID_HEIGHT-1].tool = toolEmpty;
                    globals->prefs->editor.level.tiles[i][GRID_HEIGHT-1].properties = 0;
                  }

                  EditorDrawLevel(&globals->prefs->editor.level);
                }
                else  
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemPanDown:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 
                
                  UInt16 i,j;
                  for (i=0; i<GRID_WIDTH; i++) {
                    for (j=(GRID_HEIGHT-1); j > 0; j--) {
                      globals->prefs->editor.level.tiles[i][j] = 
                        globals->prefs->editor.level.tiles[i][j-1];
                    }
                    globals->prefs->editor.level.tiles[i][0].tool = toolEmpty;
                    globals->prefs->editor.level.tiles[i][0].properties = 0;
                  }

                  EditorDrawLevel(&globals->prefs->editor.level);
                }
                else  
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemFill:

                // we can only do this if there is a level loaded
                if (globals->prefs->editor.currLevel != 0) { 
                
                  // lets make sure it is valid to fill
                  if (
                      (globals->prefs->editor.editTool.tool != toolPlayer) &&
                      (globals->prefs->editor.editTool.tool != toolMonk  )
                     ) {

                    UInt8 value = (globals->prefs->editor.editTool.tool << 3);

                    MemSet(&globals->prefs->editor.level, 
                           sizeof(LevelType), value);
                    EditorDrawLevel(&globals->prefs->editor.level);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                else
                  SndPlaySystemSound(sndError);

                processed = true;
                break;

           case editMenuItemExit:
                FrmGotoForm(mainForm);

                // we are NO LONGER editing
                globals->prefs->editor.gameEditing = false;

                processed = true;
                break;

           default:
                break;
         }
         break;

    case penDownEvent:
    case penMoveEvent:
         {
           RectangleType rect = { {  2,  16 }, { 156, 128 } };

           // did they tap inside the "editing" area?
           if (RctPtInRectangle(event->screenX, event->screenY, &rect)) {

             // we can only do this if there is a level loaded
             if (globals->prefs->editor.currLevel != 0) {

               EditorProcessPenTap(&globals->prefs->editor.level, 
                                   globals->prefs->editor.editTool, 
                                   event->screenX - rect.topLeft.x, 
                                   event->screenY - rect.topLeft.y,
                                   (event->eType == penDownEvent));
             }
             else
               SndPlaySystemSound(sndError);

             processed = true;
           }
         }
         break;

    case keyDownEvent:

         switch (event->data.keyDown.chr)
         {
           case pageUpChr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemPanUp;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case pageDownChr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemPanDown;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case hard1Chr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemPrev;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case hard2Chr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemPanLeft;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case hard3Chr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemPanRight;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case hard4Chr:
                {
                  // we can only do this if there is a level loaded
                  if (globals->prefs->editor.currLevel != 0) {
                  
                    EventType event;
           
                    // confirm it with a click
                    SndPlaySystemSound(sndClick);

                    // regenerate menu event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType            = menuEvent;
                    event.data.menu.itemID = editMenuItemNext;
                    EvtAddEventToQueue(&event);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case editFormPrevButton:

                // generate the menu event
                {
                  EventType event;
             
                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = editMenuItemPrev;
                  EvtAddEventToQueue(&event);
                }

                processed = true;
                break;

           case editFormGotoButton:

                // generate the menu event
                {
                  EventType event;
             
                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = editMenuItemGoto;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           case editFormNextButton:

                // generate the menu event
                {
                  EventType event;
             
                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = editMenuItemNext;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           case editFormEmptyButton:
           case editFormPlayerButton:
           case editFormMonkButton:
           case editFormGoldButton:
           case editFormLadderButton:
           case editFormRopeButton:
           case editFormBrickButton:
           case editFormRockButton:
           case editFormFallThruButton:
           case editFormEndLadderButton:

                // erase the current tool
                {
                  FormType      *frm;
                  RectangleType rect;
                  Coord         x1, y1, x2, y2;

                  frm = FrmGetActiveForm();
                  FrmGetObjectBounds(frm,
                    FrmGetObjectIndex(frm, 
                      editFormEmptyButton + 
                      globals->prefs->editor.editTool.tool), &rect);

                  x1 = rect.topLeft.x - 1;
                  y1 = rect.topLeft.y - 1;
                  x2 = rect.topLeft.x + rect.extent.x;
                  y2 = rect.topLeft.y + rect.extent.y;

                  // erase rectangle outline
                  WinEraseLine(x1, y1, x2, y1);
                  WinEraseLine(x2, y1, x2, y2);
                  WinEraseLine(x2, y2, x1, y2);
                  WinEraseLine(x1, y2, x1, y1);
                }

                // adjust tool as appropriate
                globals->prefs->editor.editTool.tool = 
                  event->data.ctlSelect.controlID - editFormEmptyButton;
                globals->prefs->editor.editTool.properties = 0;

                // draw the new tool
                {
                  FormType      *frm;
                  RectangleType rect;
                  Coord         x1, y1, x2, y2;

                  frm = FrmGetActiveForm();
                  FrmGetObjectBounds(frm,
                    FrmGetObjectIndex(frm, 
                      editFormEmptyButton + 
                      globals->prefs->editor.editTool.tool), &rect);

                  x1 = rect.topLeft.x - 1;
                  y1 = rect.topLeft.y - 1;
                  x2 = rect.topLeft.x + rect.extent.x;
                  y2 = rect.topLeft.y + rect.extent.y;

                  // draw rectangle outline
                  WinDrawLine(x1, y1, x2, y1);
                  WinDrawLine(x2, y1, x2, y2);
                  WinDrawLine(x2, y2, x1, y2);
                  WinDrawLine(x1, y2, x1, y1);
                }
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:

         // terminate the editor environemnt
         EditorTerminate();

         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:gameForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
gameFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           FormType *frm = FrmGetActiveForm();
           UInt16   menuID;

           // dynamically adjust the menu :)
           menuID = gameMenu_nogray;
           if (DeviceSupportsGrayscale()) menuID = gameMenu_gray;
           FrmSetMenu(frm, menuID);
         }

         // initialize the game environemnt
         if (GameInitialize(globals->prefs)) {

           // reset everything: if not playing
           if (!globals->prefs->game.gamePlaying) {
             GameResetPreferences(globals->prefs);
             globals->prefs->game.gamePaused  = false;
           }

           // init form
           FrmDrawForm(FrmGetActiveForm());
           GraphicsClear();
           GameDrawLevel(globals->prefs);
           GraphicsRepaint();

           // update the form (draw stuff)
           {
             EventType event;
             
             MemSet(&event, sizeof(EventType), 0);
             event.eType = frmUpdateEvent;
             event.data.frmUpdate.formID = FrmGetActiveFormID();
             EvtAddEventToQueue(&event);
           }
         }

         // oops.. get out
         else {
           globals->prefs->game.gamePlaying = false;
           FrmGotoForm(mainForm);
         }

         processed = true;
         break;

    case frmUpdateEvent:
         FrmDrawForm(FrmGetActiveForm());
         GameDraw(globals->prefs);

         // redraw the display
         {
           EventType event;
           
           MemSet(&event, sizeof(EventType), 0);
           event.eType = appRedrawEvent;
           EvtAddEventToQueue(&event);
         }
         processed = true;
         break;

    case appRedrawEvent:
         {
           // draw seperators
           WinDrawLine(   0, 145, 159, 145);
           WinDrawLine(   0, 146, 159, 146);
         }
         processed = true;
         break;

    case penDownEvent:
    case penMoveEvent:

         // within the game play area?
         if (
             (globals->prefs->game.gamePlaying) &&
             (event->screenX >   2) && (event->screenX < 158) &&
             (event->screenY >  16) && (event->screenY < 144)
            ) {

           // if the player has tapped this area, they wants to move
           GameProcessStylusInput(globals->prefs,
                                  event->screenX - 2, event->screenY - 16);

           // we have handled this event, lets continue
           processed = true;
         }
         break;

    case menuEvent:

         // erase the menu status
         MenuEraseStatus(MenuGetActiveMenu());

         // what menu?
         switch (event->data.menu.itemID)
         {
           case gameMenuItemPause:
                globals->prefs->game.gamePaused = 
                  !globals->prefs->game.gamePaused;

                processed = true;
                break;

           case gameMenuItemSuicide:

                // commit suicide
                GameKillPlayer(globals->prefs);
                processed = true;
                break;

           case gameMenuItemExit:

                // is the current game valid for high scores?
                if (GameHighScoreCandidate(globals->prefs)) {
                
                  GameAdjustmentType adjustType;

                  // define the adjustment
                  adjustType.adjustMode = gameHighScore;
                  if (RegisterAdjustGame(globals->prefs, &adjustType)) {
                    ApplicationDisplayDialog(highForm);
                  }
                }
                globals->prefs->game.gamePlaying = false;
                FrmGotoForm(mainForm);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID) 
         {
           case gameFormSoundButton:

                // regenerate menu event
                {
                  EventType event;
           
                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = menuItemConfig;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           case gameFormPauseButton:

                // regenerate menu event
                {
                  EventType event;
           
                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = gameMenuItemPause;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case nilEvent:

         // make sure the active window is the form 
         if (WinGetActiveWindow() == (WinHandle)FrmGetActiveForm()) {

           // play the game
           GameProcessKeyInput(globals->prefs, KeyCurrentState());
           GameMovement(globals->prefs);

           // is the pen being held down? if so, lets post event
           {
             Coord   x, y;
             Boolean penDown;

             EvtGetPen(&x, &y, &penDown);

             if (penDown) {
               EventType event;
               MemSet(&event, sizeof(EventType), 0);
               event.eType   = penMoveEvent;
               event.penDown = true;
               event.screenX = x;
               event.screenY = y;
               EvtAddEventToQueue(&event);
             }
           }
         }
         processed = true;
         break;

    case frmCloseEvent:

         // terminate the game environemnt
         GameTerminate();

         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:cfigForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
cfigFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           UInt16      index;
           ControlType *cheatCheckbox, *muteCheckbox, *volumeControl;
           ListType    *lstHard1, *lstHard2, *lstHard3, *lstHard4;
           ListType    *lstUp, *lstDown, *lstStage[3];
           ControlType *ctlHard1, *ctlHard2, *ctlHard3, *ctlHard4;
           ControlType *ctlUp, *ctlDown, *ctlStage[3];
           UInt16      *choices[] = { 
                                      &(globals->prefs->config.ctlKeyLeft), 
                                      &(globals->prefs->config.ctlKeyRight), 
                                      &(globals->prefs->config.ctlKeyUp), 
                                      &(globals->prefs->config.ctlKeyDown), 
                                      &(globals->prefs->config.ctlKeyLeftDig),
                                      &(globals->prefs->config.ctlKeyRightDig) 
                                    };

           frm = FrmGetActiveForm();
           ctlHard1 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey1Trigger));
           ctlHard2 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey2Trigger));
           ctlHard3 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey3Trigger));
           ctlHard4 =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey4Trigger));
           ctlUp =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageUpTrigger));
           ctlDown =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageDownTrigger));
         
           lstHard1 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey1List));
           lstHard2 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey2List));
           lstHard3 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey3List));
           lstHard4 =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormHardKey4List));
           lstUp =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageUpList));
           lstDown =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormPageDownList));

           LstSetSelection(lstHard1, 0);
           CtlSetLabel(ctlHard1, LstGetSelectionText(lstHard1, 0));
           LstSetSelection(lstHard2, 0);
           CtlSetLabel(ctlHard2, LstGetSelectionText(lstHard2, 0));
           LstSetSelection(lstHard3, 0);
           CtlSetLabel(ctlHard3, LstGetSelectionText(lstHard3, 0));
           LstSetSelection(lstHard4, 0);
           CtlSetLabel(ctlHard4, LstGetSelectionText(lstHard4, 0));
           LstSetSelection(lstDown, 0);
           CtlSetLabel(ctlDown, LstGetSelectionText(lstDown, 0));
           LstSetSelection(lstUp, 0);
           CtlSetLabel(ctlUp, LstGetSelectionText(lstUp, 0));

           // show the "current" settings
           for (index=0; index<6; index++) {

             if ((*(choices[index]) & keyBitHard1) != 0) { 
               LstSetSelection(lstHard1, index);
               CtlSetLabel(ctlHard1, LstGetSelectionText(lstHard1, index));
             }  

             if ((*(choices[index]) & keyBitHard2) != 0) { 
               LstSetSelection(lstHard2, index);
               CtlSetLabel(ctlHard2, LstGetSelectionText(lstHard2, index));
             }  

             if ((*(choices[index]) & keyBitHard3) != 0) { 
               LstSetSelection(lstHard3, index);
               CtlSetLabel(ctlHard3, LstGetSelectionText(lstHard3, index));
             }  

             if ((*(choices[index]) & keyBitHard4) != 0) { 
               LstSetSelection(lstHard4, index);
               CtlSetLabel(ctlHard4, LstGetSelectionText(lstHard4, index));
             }  

             if ((*(choices[index]) & keyBitPageUp) != 0) { 
               LstSetSelection(lstUp, index);
               CtlSetLabel(ctlUp, LstGetSelectionText(lstUp, index));
             }  

             if ((*(choices[index]) & keyBitPageDown) != 0) { 
               LstSetSelection(lstDown, index);
               CtlSetLabel(ctlDown, LstGetSelectionText(lstDown, index));
             }  
           } 

           ctlStage[0] =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage0Trigger));
           ctlStage[1] =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage1Trigger));
           ctlStage[2] =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage2Trigger));
           lstStage[0] =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage0List));
           lstStage[1] =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage1List));
           lstStage[2] =
             (ListType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormStage2List));

           LstSetSelection(lstStage[0],
                        (globals->prefs->config.startLevel / 100) % 10);
           CtlSetLabel(ctlStage[0],
             LstGetSelectionText(lstStage[0], LstGetSelection(lstStage[0])));
           LstSetSelection(lstStage[1],
                       (globals->prefs->config.startLevel / 10) % 10);
           CtlSetLabel(ctlStage[1],
             LstGetSelectionText(lstStage[1], LstGetSelection(lstStage[1])));
           LstSetSelection(lstStage[2],
                        globals->prefs->config.startLevel % 10);
           CtlSetLabel(ctlStage[2],
             LstGetSelectionText(lstStage[2], LstGetSelection(lstStage[2])));

           cheatCheckbox = 
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormCheatCheckbox));
           muteCheckbox = 
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, cfigFormMuteCheckbox));
           volumeControl = 
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, 
                 cfigFormSound0Button + DeviceGetVolume()));

           CtlSetValue(cheatCheckbox, 
             (globals->prefs->config.cheatsEnabled) ? 1 : 0); 
           CtlSetValue(muteCheckbox, DeviceGetMute() ? 1 : 0); 
           CtlSetValue(volumeControl, 1);

           // save this
           FtrSet(appCreator, ftrGlobalsCfgActiveVol, (UInt32)volumeControl);
         }
         processed = true;
         break;

    case ctlEnterEvent:

         switch (event->data.ctlEnter.controlID)
         {
           case cfigFormSound0Button:
           case cfigFormSound1Button:
           case cfigFormSound2Button:
           case cfigFormSound3Button:
           case cfigFormSound4Button:
           case cfigFormSound5Button:
           case cfigFormSound6Button:
           case cfigFormSound7Button:
                {
                  ControlType *newCtl, *oldCtl;

                  newCtl = event->data.ctlEnter.pControl;

                  // we dont want an audible beep from the system
                  FtrGet(appCreator, ftrGlobalsCfgActiveVol, (UInt32 *)&oldCtl);
                  CtlSetValue(oldCtl, 0);
                  CtlSetValue(newCtl, 1);
                  FtrSet(appCreator, ftrGlobalsCfgActiveVol, (UInt32)newCtl);

                  // act as we needed :)
                  DeviceSetVolume(
                     (event->data.ctlEnter.controlID - cfigFormSound0Button));
                  {
                    SndCommandType testBeep =
                      {sndCmdFreqDurationAmp,0,512,32,sndMaxAmp};
                    DevicePlaySound(&testBeep);
                  }
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case cfigFormMuteCheckbox:
                DeviceSetMute(CtlGetValue(event->data.ctlEnter.pControl) != 0);
                processed = true;
                break;

           case cfigFormOkButton:
                {
                  ListType    *lstHard1, *lstHard2, *lstHard3, *lstHard4;
                  ListType    *lstUp, *lstDown, *lstStage[3];
                  ControlType *cheatCheckbox, *muteCheckbox;
                  UInt16      index;
                  FormType    *frm;
                  UInt16      keySignature, keyLevel;
                  UInt16      *choices[] = { 
                                       &(globals->prefs->config.ctlKeyLeft), 
                                       &(globals->prefs->config.ctlKeyRight), 
                                       &(globals->prefs->config.ctlKeyUp), 
                                       &(globals->prefs->config.ctlKeyDown), 
                                       &(globals->prefs->config.ctlKeyLeftDig),
                                       &(globals->prefs->config.ctlKeyRightDig) 
                                           };

                  frm = FrmGetActiveForm();
                  lstHard1 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey1List));
                  lstHard2 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey2List));
                  lstHard3 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey3List));
                  lstHard4 = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormHardKey4List));
                  lstUp = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormPageUpList));
                  lstDown = 
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormPageDownList));

                  keySignature = 
                    (
                      (0x01 << LstGetSelection(lstHard1)) |
                      (0x01 << LstGetSelection(lstHard2)) |
                      (0x01 << LstGetSelection(lstHard3)) |
                      (0x01 << LstGetSelection(lstHard4)) |
                      (0x01 << LstGetSelection(lstUp))    |
                      (0x01 << LstGetSelection(lstDown))
                    );

                  lstStage[0] =
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormStage0List));
                  lstStage[1] =
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormStage1List));
                  lstStage[2] =
                    (ListType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, cfigFormStage2List));

                  keyLevel = 
                    (
                      (LstGetSelection(lstStage[0]) * 100) + 
                      (LstGetSelection(lstStage[1]) * 10) + 
                       LstGetSelection(lstStage[2]) 
                    );

                  // only process if good setting is selected.
                  if (
                      (keySignature == 0x3F) &&
                      (keyLevel      > 0x00) && 
                      (keyLevel     <= 0xff) 
                     ) {

                    globals->prefs->config.startLevel = keyLevel;

                    // key preferences
                    for (index=0; index<6; index++) {
                      *(choices[index]) = 0;
                    }
                    if (LstGetSelection(lstHard1) != noListSelection) 
                      *(choices[LstGetSelection(lstHard1)]) |= keyBitHard1;
                    if (LstGetSelection(lstHard2) != noListSelection) 
                      *(choices[LstGetSelection(lstHard2)]) |= keyBitHard2;
                    if (LstGetSelection(lstHard3) != noListSelection) 
                      *(choices[LstGetSelection(lstHard3)]) |= keyBitHard3;
                    if (LstGetSelection(lstHard4) != noListSelection) 
                      *(choices[LstGetSelection(lstHard4)]) |= keyBitHard4;
                    if (LstGetSelection(lstUp) != noListSelection) 
                      *(choices[LstGetSelection(lstUp)])    |= keyBitPageUp;
                    if (LstGetSelection(lstDown) != noListSelection) 
                      *(choices[LstGetSelection(lstDown)])  |= keyBitPageDown;

                    cheatCheckbox = 
                      (ControlType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, cfigFormCheatCheckbox));
                    globals->prefs->config.cheatsEnabled = 
                      (CtlGetValue(cheatCheckbox) == 1);

                    muteCheckbox = 
                      (ControlType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, cfigFormMuteCheckbox));
                    globals->prefs->config.sndMute = 
                      (CtlGetValue(muteCheckbox) == 1);

                    // send a close event
                    {
                      EventType event;

                      MemSet(&event, sizeof(EventType), 0);
                      event.eType = frmCloseEvent;
                      event.data.frmClose.formID = FrmGetActiveFormID();
                      EvtAddEventToQueue(&event);
                    }
                  }
                  else 
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case cfigFormCancelButton:

                // revert changes
                DeviceSetMute(globals->prefs->config.sndMute);
                DeviceSetVolume(globals->prefs->config.sndVolume);

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }

                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:

         // clean up
         FtrUnregister(appCreator, ftrGlobalsCfgActiveVol);

         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:infoForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
infoFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case infoFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:grayForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean
grayFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType)
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           ControlType *ctlWhite, *ctlLGray, *ctlDGray, *ctlBlack;

           frm = FrmGetActiveForm();
           ctlWhite =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, grayWhite1Button));
           ctlLGray =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm,
                 grayLightGray1Button + globals->prefs->config.lgray));
           ctlDGray =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm,
                 grayDarkGray1Button + globals->prefs->config.dgray));
           ctlBlack =
             (ControlType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, grayBlack7Button));

           CtlSetValue(ctlWhite, 1); CtlDrawControl(ctlWhite);
           CtlSetValue(ctlLGray, 1); CtlDrawControl(ctlLGray);
           CtlSetValue(ctlDGray, 1); CtlDrawControl(ctlDGray);
           CtlSetValue(ctlBlack, 1); CtlDrawControl(ctlBlack);
         }

         // pre 3.5 - we must 'redraw' form to actually display PUSHBUTTONS
         if (!DeviceSupportsVersion(romVersion3_5)) {
           FrmDrawForm(FrmGetActiveForm());
         }

         processed = true;
         break;

    case ctlEnterEvent:

         switch (event->data.ctlEnter.controlID) 
         {
           case grayLightGray1Button:
           case grayLightGray7Button:
           case grayDarkGray1Button:
           case grayDarkGray7Button:

           // stupid user, they must select one of the other options
           SndPlaySystemSound(sndError);
           processed = true;
         }
         break;

    case ctlSelectEvent:
         
         switch (event->data.ctlSelect.controlID) 
         {
           case grayLightGray2Button:
           case grayLightGray3Button:
           case grayLightGray4Button:
           case grayLightGray5Button:
           case grayLightGray6Button:

                globals->prefs->config.lgray = event->data.ctlEnter.controlID -
                                               grayLightGray1Button;
                DeviceGrayscale(graySet, 
                                &globals->prefs->config.lgray, 
                                &globals->prefs->config.dgray);
                processed = true;
                break;

           case grayDarkGray2Button:
           case grayDarkGray3Button:
           case grayDarkGray4Button:
           case grayDarkGray5Button:
           case grayDarkGray6Button:

                globals->prefs->config.dgray = event->data.ctlEnter.controlID -
                                               grayDarkGray1Button;
                DeviceGrayscale(graySet, 
                                &globals->prefs->config.lgray, 
                                &globals->prefs->config.dgray);
                processed = true;
                break;

           case grayFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;
               
           default:
                break;
         } 
         break;

    default:
         break;
  }

  return processed;
} 

#ifdef PROTECTION_ON
/**
 * The Form:regiForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
regiFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());

         // display the HotSync username on the screen (in HEX)
         {
           Char  ID[40];
           Char  tmp[10], num[3];
           Coord x;
           UInt8 i, checksum;

           FontID font = FntGetFont();

           // initialize
           StrCopy(ID, "");

           // generate strings
           checksum = 0;
           for (i=0; i<MAX_IDLENGTH; i++) {

             checksum ^= (UInt8)globals->prefs->system.hotSyncUsername[i];
             StrIToH(tmp, (UInt8)globals->prefs->system.hotSyncUsername[i]);
             StrNCopy(num, &tmp[StrLen(tmp)-2], 2); num[2] = '\0';
             StrCat(ID, num); StrCat(ID, ":");
           }
           StrIToH(tmp, checksum);
           StrNCopy(num, &tmp[StrLen(tmp)-2], 2); num[2] = '\0';
           StrCat(ID, num);

           FntSetFont(boldFont);
           x = (160 - FntCharsWidth(ID, StrLen(ID))) >> 1;
           WinDrawChars(ID, StrLen(ID), x, 68);
           FntSetFont(font);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case regiFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:rbugForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
rbugFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case rbugFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}
#endif

/**
 * The Form:highForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
highFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType  *frm;
           FieldType *fldScore, *fldLevel, *fldCode;
           MemHandle memHandle;
           Char      scoreStr[10];

           frm = FrmGetActiveForm();

           fldScore = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormScoreField));
           fldLevel = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormLevelField));
           fldCode = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormCodeField));

           // score
           memHandle = MemHandleNew(16);
           if (globals->prefs->game.gameHighScore != 0)
             StrIToA(MemHandleLock(memHandle), 
                     globals->prefs->game.gameHighScore);
           else
             StrCopy(MemHandleLock(memHandle), "N/A");
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldScore, memHandle);
           FldDrawField(fldScore);

           // level (> 140 = all)
           memHandle = MemHandleNew(16);
           if (globals->prefs->game.gameHighLevel == 255)
             StrCopy(MemHandleLock(memHandle), "ALL");
           else
           if (globals->prefs->game.gameHighLevel == 0)
             StrCopy(MemHandleLock(memHandle), "N/A");
           else 
             StrIToA(MemHandleLock(memHandle), 
                     globals->prefs->game.gameHighLevel);
           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldLevel, memHandle);
           FldDrawField(fldLevel);

           // code 
           memHandle = MemHandleNew(16);
           if ((globals->prefs->game.gameHighScore != 0) &&
               (globals->prefs->game.gameHighLevel != 0)) {
           
             GameAdjustmentType adjustType;

             StrIToA(scoreStr, 
                     globals->prefs->game.gameHighScore +
                     globals->prefs->game.gameHighLevel);

             // define the adjustment
             adjustType.adjustMode         = gameScoreCode;
             adjustType.data.scoreCode.key = scoreStr;
             RegisterAdjustGame(globals->prefs, &adjustType);
          
             StrIToA(MemHandleLock(memHandle), adjustType.data.scoreCode.code);
           }
           else 
             StrCopy(MemHandleLock(memHandle), "N/A");

           MemHandleUnlock(memHandle);
           FldSetTextHandle(fldCode, memHandle);
           FldDrawField(fldCode);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case highFormOkButton:

                // send a close event
                MemSet(event, sizeof(EventType), 0);
                event->eType = frmCloseEvent;
                event->data.frmClose.formID = FrmGetActiveFormID();
                EvtAddEventToQueue(event);

                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:
         {
           FormType  *frm;
           FieldType *fldScore, *fldLevel, *fldCode;
           MemHandle memHandle;

           frm = FrmGetActiveForm();

           fldScore = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormScoreField));
           fldLevel = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormLevelField));
           fldCode = 
             (FieldType *)FrmGetObjectPtr(frm,
                FrmGetObjectIndex(frm, highFormCodeField));

           // score
           memHandle = FldGetTextHandle(fldScore);
           FldSetTextHandle(fldScore, NULL);
           MemHandleFree(memHandle);

           // level 
           memHandle = FldGetTextHandle(fldLevel);
           FldSetTextHandle(fldLevel, NULL);
           MemHandleFree(memHandle);

           // code 
           memHandle = FldGetTextHandle(fldCode);
           FldSetTextHandle(fldCode, NULL);
           MemHandleFree(memHandle);
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:helpForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
helpFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           UInt16 helpHeight;

           helpHeight = InitInstructions();

           // help exists
           if (helpHeight != 0) 
           {
             FormType      *frm;
             ScrollBarType *sclBar;
             UInt16        val, min, max, pge;

             frm    = FrmGetActiveForm();
             FrmDrawForm(frm);

             sclBar =  
               (ScrollBarType *)FrmGetObjectPtr(frm, 
                 FrmGetObjectIndex(frm, helpFormScrollBar)); 

             SclGetScrollBar(sclBar, &val, &min, &max, &pge);
             val = helpHeight;
             max = (val > pge) ? (val-(pge+16)) : 0;
             SclSetScrollBar(sclBar, 0, min, max, pge);

             DrawInstructions(0);
           }

           // no help, close form
           else
           {
             EventType newEvent;

             MemSet(&newEvent, sizeof(EventType), 0);
             newEvent.eType = frmCloseEvent;
             newEvent.data.frmClose.formID = FrmGetActiveFormID();
             EvtAddEventToQueue(&newEvent);
           }
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case helpFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case sclRepeatEvent:
         {
           FormType      *frm;
           ScrollBarType *sclBar;
           UInt16        val, min, max, pge;

           frm = FrmGetActiveForm();
           sclBar = 
             (ScrollBarType *)FrmGetObjectPtr(frm,
               FrmGetObjectIndex(frm, helpFormScrollBar));

           SclGetScrollBar(sclBar, &val, &min, &max, &pge);
           DrawInstructions(val);
         }
         break;

    case keyDownEvent:

         switch (event->data.keyDown.chr)
         {
           case pageUpChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, helpFormScrollBar));

                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                  val = (pge > val) ? 0 : (val-pge); 
                  SclSetScrollBar(sclBar, val, min, max, pge);
                  DrawInstructions(val);
                }
                processed = true;
                break;

           case pageDownChr:
                {
                  FormType      *frm;
                  ScrollBarType *sclBar;
                  UInt16        val, min, max, pge;

                  frm = FrmGetActiveForm();
                  sclBar = 
                    (ScrollBarType *)FrmGetObjectPtr(frm,
                      FrmGetObjectIndex(frm, helpFormScrollBar));

                  SclGetScrollBar(sclBar, &val, &min, &max, &pge);
                  val = (max < (val+pge)) ? max : (val+pge); 
                  SclSetScrollBar(sclBar, val, min, max, pge);
                  DrawInstructions(val);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:

         // clean up
         QuitInstructions();
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:xmemForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
xmemFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case xmemFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:cr8rForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
cr8rFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         {
           FormType  *frm;
           FieldType *fldName;
           MemHandle memHandle;

           frm = FrmGetActiveForm();
           FrmDrawForm(frm);

           // initialize memory for field
           fldName =
             (FieldType *)FrmGetObjectPtr(frm, 
                 FrmGetObjectIndex(frm, cr8rFormDBNameField));
           memHandle = MemHandleNew(20);

           StrCopy(MemHandleLock(memHandle), ""); MemHandleUnlock(memHandle);
           FldSetTextHandle(fldName, memHandle);
           FrmSetFocus(frm, FrmGetObjectIndex(frm, cr8rFormDBNameField));
           FldDrawField(fldName);
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case cr8rFormOkButton:

                // create the new database
                {
                  FormType  *frm;
                  FieldType *fldName;
                  Char      *strName;

                  frm = FrmGetActiveForm();

                  // get the text from the field
                  fldName =
                    (FieldType *)FrmGetObjectPtr(frm, 
                        FrmGetObjectIndex(frm, cr8rFormDBNameField));
                  strName = FldGetTextPtr(fldName);

                  if (
                      (strName != NULL) &&
                      (StrCompare(strName, "") != 0)
                     ) {

                    Char dbName[32];

                    StrPrintF(dbName, "LODE_%s", strName);
                    if (EditorCreateDatabase(dbName)) {

                      EventType event;

                      // send a close event
                      MemSet(&event, sizeof(EventType), 0);
                      event.eType = frmCloseEvent;
                      event.data.frmClose.formID = FrmGetActiveFormID();
                      EvtAddEventToQueue(&event);
                      
                      // send a "regenerate" list event
                      MemSet(&event, sizeof(EventType), 0);
                      event.eType = appGenerateLevelSetList;
                      EvtAddEventToQueue(&event);
                    }
                    else 
                      SndPlaySystemSound(sndError);
                  }
                  else
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case cr8rFormCancelButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case frmCloseEvent:
         {
           FormType  *frm;
           FieldType *fldName;
           MemHandle memHandle;

           frm = FrmGetActiveForm();

           // free memory for field
           fldName =
             (FieldType *)FrmGetObjectPtr(frm, 
                 FrmGetObjectIndex(frm, cr8rFormDBNameField));
           memHandle = FldGetTextHandle(fldName);
           FldSetTextHandle(fldName, NULL);
           MemHandleFree(memHandle);
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:moveForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
moveFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           ListType    *lstLevel[3], *lstDestination[3];
           ControlType *ctlLevel[3], *ctlDestination[3];

           frm = FrmGetActiveForm();

           ctlLevel[0] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage0Trigger));
           ctlLevel[1] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage1Trigger));
           ctlLevel[2] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage2Trigger));
           lstLevel[0] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage0List));
           lstLevel[1] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage1List));
           lstLevel[2] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormSStage2List));

           ctlDestination[0] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage0Trigger));
           ctlDestination[1] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage1Trigger));
           ctlDestination[2] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage2Trigger));
           lstDestination[0] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage0List));
           lstDestination[1] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage1List));
           lstDestination[2] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, moveFormDStage2List));

           // setup the values
           LstSetSelection(lstLevel[0], (globals->prefs->editor.currLevel / 100) % 10);
           CtlSetLabel(ctlLevel[0],
             LstGetSelectionText(lstLevel[0], LstGetSelection(lstLevel[0])));
           LstSetSelection(lstLevel[1], (globals->prefs->editor.currLevel / 10) % 10);
           CtlSetLabel(ctlLevel[1],
             LstGetSelectionText(lstLevel[1], LstGetSelection(lstLevel[1])));
           LstSetSelection(lstLevel[2], globals->prefs->editor.currLevel % 10);
           CtlSetLabel(ctlLevel[2],
             LstGetSelectionText(lstLevel[2], LstGetSelection(lstLevel[2])));

           LstSetSelection(lstDestination[0], (globals->prefs->editor.currLevel / 100) % 10);
           CtlSetLabel(ctlDestination[0],
             LstGetSelectionText(lstDestination[0], 
               LstGetSelection(lstDestination[0])));
           LstSetSelection(lstDestination[1], (globals->prefs->editor.currLevel / 10) % 10);
           CtlSetLabel(ctlDestination[1],
             LstGetSelectionText(lstDestination[1], 
               LstGetSelection(lstDestination[1])));
           LstSetSelection(lstDestination[2], globals->prefs->editor.currLevel % 10);
           CtlSetLabel(ctlDestination[2],
             LstGetSelectionText(lstDestination[2], 
               LstGetSelection(lstDestination[2])));
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case moveFormOkButton:
                {
                  FormType *frm;
                  ListType *lstLevel[3], *lstDestination[3];
                  UInt16   keyLevel, keyDestination;

                  frm = FrmGetActiveForm();

                  lstLevel[0] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormSStage0List));
                  lstLevel[1] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormSStage1List));
                  lstLevel[2] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormSStage2List));

                  lstDestination[0] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormDStage0List));
                  lstDestination[1] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormDStage1List));
                  lstDestination[2] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, moveFormDStage2List));

                  keyLevel =
                    (
                      (LstGetSelection(lstLevel[0]) * 100) + 
                      (LstGetSelection(lstLevel[1]) * 10) + 
                       LstGetSelection(lstLevel[2]) 
                    );
                  keyDestination =
                    (
                      (LstGetSelection(lstDestination[0]) * 100) + 
                      (LstGetSelection(lstDestination[1]) * 10) + 
                       LstGetSelection(lstDestination[2]) 
                    );

                  // lets make sure the configuration is correct
                  if (
                      (keyLevel       > 0) &&
                      (keyDestination > 0) &&
                      (keyLevel       <= EditorLevelCount()) &&
                      (keyDestination <= EditorLevelCount()) 
                     ) {

                    EventType event;

                    // perform the move
                    if (keyDestination > keyLevel) 
                      EditorMoveLevel(keyLevel, keyDestination+1);
                    else
                      EditorMoveLevel(keyLevel, keyDestination);
                    globals->prefs->editor.currLevel = keyDestination;

                    // send a close event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = frmCloseEvent;
                    event.data.frmClose.formID = FrmGetActiveFormID();
                    EvtAddEventToQueue(&event);
                      
                    // we are NO LONGER editing
                    globals->prefs->editor.gameEditing = false;

                    // send a repaint event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = appRedrawEvent;
                    EvtAddEventToQueue(&event);
                  }
                  else 
                    SndPlaySystemSound(sndError);
                }
                processed = true;
                break;

           case moveFormCancelButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:gotoForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
gotoFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         {
           FormType    *frm;
           ListType    *lstLevel[3];
           ControlType *ctlLevel[3];

           frm = FrmGetActiveForm();

           ctlLevel[0] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage0Trigger));
           ctlLevel[1] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage1Trigger));
           ctlLevel[2] =
             (ControlType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage2Trigger));
           lstLevel[0] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage0List));
           lstLevel[1] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage1List));
           lstLevel[2] =
             (ListType *)FrmGetObjectPtr(frm,
                 FrmGetObjectIndex(frm, gotoFormStage2List));

           // setup the values
           LstSetSelection(lstLevel[0], (globals->prefs->editor.currLevel / 100) % 10);
           CtlSetLabel(ctlLevel[0],
             LstGetSelectionText(lstLevel[0], LstGetSelection(lstLevel[0])));
           LstSetSelection(lstLevel[1], (globals->prefs->editor.currLevel / 10) % 10);
           CtlSetLabel(ctlLevel[1],
             LstGetSelectionText(lstLevel[1], LstGetSelection(lstLevel[1])));
           LstSetSelection(lstLevel[2], globals->prefs->editor.currLevel % 10);
           CtlSetLabel(ctlLevel[2],
             LstGetSelectionText(lstLevel[2], LstGetSelection(lstLevel[2])));
         }
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case gotoFormOkButton:
                {
                  FormType *frm;
                  ListType *lstLevel[3];
                  UInt16   keyLevel;

                  frm = FrmGetActiveForm();

                  lstLevel[0] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, gotoFormStage0List));
                  lstLevel[1] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, gotoFormStage1List));
                  lstLevel[2] =
                    (ListType *)FrmGetObjectPtr(frm,
                        FrmGetObjectIndex(frm, gotoFormStage2List));

                  keyLevel =
                    (
                      (LstGetSelection(lstLevel[0]) * 100) + 
                      (LstGetSelection(lstLevel[1]) * 10) + 
                       LstGetSelection(lstLevel[2]) 
                    );

                  // lets make sure the configuration is correct
                  if (
                      (keyLevel > 0) &&
                      (keyLevel <= EditorLevelCount())
                     ) {

                    EventType event;

                    // perform the goto :)
                    globals->prefs->editor.currLevel = keyLevel;

                    // send a close event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = frmCloseEvent;
                    event.data.frmClose.formID = FrmGetActiveFormID();
                    EvtAddEventToQueue(&event);
                      
                    // we are NO LONGER editing
                    globals->prefs->editor.gameEditing = false;

                    // send a repaint event
                    MemSet(&event, sizeof(EventType), 0);
                    event.eType = appRedrawEvent;
                    EvtAddEventToQueue(&event);
                  }
                  else 
                    SndPlaySystemSound(sndError);
                }

                processed = true;
                break;

           case gotoFormCancelButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Form:xlevForm event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
static Boolean 
xlevFormEventHandler(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType) 
  {
    case frmOpenEvent:
         FrmDrawForm(FrmGetActiveForm());
         processed = true;
         break;
   
    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case xlevFormOkButton:

                // send a close event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType = frmCloseEvent;
                  event.data.frmClose.formID = FrmGetActiveFormID();
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * The Palm Computing Platform initialization routine.
 */
void  
InitApplication()
{
  Globals *globals;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // load preferences
  {
    Boolean reset;
    UInt16  prefSize;
    Int16   flag;

    // allocate memory for preferences
    globals->prefs = (PreferencesType *)MemPtrNew(sizeof(PreferencesType));
    reset          = true;

    // lets see how large the preference is (if it is there)
    prefSize = 0;
    flag     = PrefGetAppPreferences(appCreator, 0, NULL, &prefSize, true);

    // we have some preferences, maybe a match :)
    if ((flag != noPreferenceFound) && (prefSize == sizeof(PreferencesType))) {

      // extract all the bytes
      PrefGetAppPreferences(appCreator, 0, globals->prefs, &prefSize, true);

      // decrypt the memory chunk (based on the @message)
      RegisterDecryptChunk((UInt8 *)globals->prefs, prefSize, 0x0007, 0x01);

      reset = !CHECK_SIGNATURE(globals->prefs) ||
              (globals->prefs->system.signatureVersion != VERSION);
    }

    // do we need to reset the preferences
    if (reset) {

      // set default values
      prefSize = sizeof(PreferencesType);
      MemSet(globals->prefs, prefSize, 0);

      globals->prefs->system.signatureVersion = VERSION;
      StrCopy(globals->prefs->system.signature, "|HaCkMe|");

      globals->prefs->config.ctlKeyLeftDig    = keyBitHard1;
      globals->prefs->config.ctlKeyRightDig   = keyBitHard4;
      globals->prefs->config.ctlKeyUp         = keyBitPageUp;
      globals->prefs->config.ctlKeyDown       = keyBitPageDown;
      globals->prefs->config.ctlKeyLeft       = keyBitHard2;
      globals->prefs->config.ctlKeyRight      = keyBitHard3;

      globals->prefs->config.sndMute          = false;
      globals->prefs->config.sndVolume        = 7;

      globals->prefs->config.cheatsEnabled    = false;
      globals->prefs->config.startLevel       = 1;

      globals->prefs->game.gamePlaying        = false;
      globals->prefs->editor.gameEditing      = false;

      globals->prefs->game.gameHighScore      = 0;
      globals->prefs->game.gameHighLevel      = 0;

      if (DeviceSupportsGrayscale()) {
        DeviceGrayscale(grayGet,
                        &globals->prefs->config.lgray, 
                        &globals->prefs->config.dgray);
      }
    }
  }

  // get the current grayscale settings - as appropriate
  if (DeviceSupportsGrayscale()) {
    DeviceGrayscale(grayGet, &globals->lgray, &globals->dgray);
  }

  // get the HotSync user name
  globals->prefs->system.hotSyncUsername = 
    (Char *)MemPtrNew(dlkUserNameBufSize * sizeof(Char));
  MemSet(globals->prefs->system.hotSyncUsername, dlkUserNameBufSize, 0);
  DlkGetSyncInfo(NULL, NULL, NULL,
                 globals->prefs->system.hotSyncUsername, NULL, NULL);
  {
    Char *ptrStr;
  
    ptrStr = StrChr(globals->prefs->system.hotSyncUsername, spaceChr);
    if (ptrStr != NULL) { 

      // erase everything after the FIRST space
      UInt8 index = ((UInt32)ptrStr - 
                     (UInt32)globals->prefs->system.hotSyncUsername);
      MemSet(ptrStr, dlkUserNameBufSize - index, 0);
    }

    ptrStr = StrChr(globals->prefs->system.hotSyncUsername, '\0');
    if (ptrStr != NULL) { 

      // erase everything after the FIRST null char
      UInt8 index = ((UInt32)ptrStr - 
                     (UInt32)globals->prefs->system.hotSyncUsername);
      MemSet(ptrStr, dlkUserNameBufSize - index, 0);
    }
  }
  
  // configure the grayscale settings - as appropriate
  if (DeviceSupportsGrayscale()) {
    DeviceGrayscale(graySet,
                    &globals->prefs->config.lgray,
                    &globals->prefs->config.dgray);
  }

  // setup sound config
  DeviceSetMute(globals->prefs->config.sndMute);
  DeviceSetVolume(globals->prefs->config.sndVolume);

  // setup the valid keys available while the app is running
  KeySetMask(~(keyBitsAll ^ 
             (keyBitPower   | keyBitCradle   | 
              keyBitPageUp  | keyBitPageDown |
              keyBitAntenna | keyBitContrast)));

  globals->evtTimeOut    = evtWaitForever;
  globals->ticksPerFrame = SysTicksPerSecond() / GAME_FPS;

  // initialize the game environment
  RegisterInitialize(globals->prefs);

  // in the middle of a game?
  if ((globals->prefs->game.gamePlaying) &&   // DB still there?
      ((DmFindDatabase(0, globals->prefs->game.strLevelSetName) != 0) ||
       (DmFindDatabase(1, globals->prefs->game.strLevelSetName) != 0))) {
    globals->prefs->game.gamePaused = true;
    FrmGotoForm(gameForm);
  }
  else 

  // editing?
  if ((globals->prefs->editor.gameEditing) && // DB still there?
      ((DmFindDatabase(0, globals->prefs->editor.strLevelSetName) != 0) ||
       (DmFindDatabase(1, globals->prefs->editor.strLevelSetName) != 0))) {
    FrmGotoForm(editForm);
  }

  // doing nothing else, default to the "main screen"
  else 
    FrmGotoForm(mainForm);
}

/**
 * The Palm Computing Platform entry routine (mainline).
 *
 * @param cmd         a word value specifying the launch code.
 * @param cmdPBP      pointer to a structure associated with the launch code.
 * @param launchFlags additional launch flags.
 * @return zero if launch successful, non zero otherwise.
 */
UInt32  
PilotMain(UInt16 cmd, MemPtr cmdPBP, UInt16 launchFlags)
{
  UInt32 result = 0;

  // what type of launch was this?
  switch (cmd) 
  {
    case sysAppLaunchCmdNormalLaunch:
         {
           // is this device compatable?
           if (DeviceCheckCompatability()) {

             Globals *globals;
             Boolean ok;

             // initialize device
             DeviceInitialize();

             // create the globals object, and register it
             globals = (Globals *)MemPtrNew(sizeof(Globals));
             MemSet(globals, sizeof(Globals), 0);
             FtrSet(appCreator, ftrGlobals, (UInt32)globals);

             // initialize the graphics engine
             ok = GraphicsInitialize();

             // everything worked out ok?
             if (ok) {

               // run the application :))
               InitApplication();
               EventLoop();
               EndApplication();
             }

             // terminate the graphics engine
             GraphicsTerminate();
            
             // must tell user no memory left :(
             if (!ok)
               ApplicationDisplayDialog(xmemForm);
 
             // unregister the feature
             MemPtrFree(globals);
             FtrUnregister(appCreator, ftrGlobals);

             // restore device state
             DeviceTerminate();
           }
         }
         break;

    default:
         break;
  }

  return result;
}

/**
 * The application event handling routine.
 *
 * @param event the event to process.
 * @return true if the event was handled, false otherwise.
 */
Boolean 
ApplicationHandleEvent(EventType *event)
{
  Globals *globals;
  Boolean processed = false;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  switch (event->eType)
  {
    case frmLoadEvent:
         {
           UInt16   formID = event->data.frmLoad.formID;
           FormType *frm   = FrmInitForm(formID);

           FrmSetActiveForm(frm);
           switch (formID) 
           {
             case mainForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)mainFormEventHandler);

                  processed = true;
                  break;

             case gameForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)gameFormEventHandler);

                  processed = true;
                  break;

             case editForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)editFormEventHandler);

                  processed = true;
                  break;

             case infoForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)infoFormEventHandler);

                  processed = true;
                  break;

             case cfigForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)cfigFormEventHandler);

                  processed = true;
                  break;

             case grayForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)grayFormEventHandler);

                  processed = true;
                  break;

#ifdef PROTECTION_ON
             case regiForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)regiFormEventHandler);

                  processed = true;
                  break;

             case rbugForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)rbugFormEventHandler);

                  processed = true;
                  break;
#endif

             case highForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)highFormEventHandler);

                  processed = true;
                  break;

             case helpForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)helpFormEventHandler);
                  processed = true;
                  break;

             case xmemForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)xmemFormEventHandler);

                  processed = true;
                  break;

             case cr8rForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)cr8rFormEventHandler);

                  processed = true;
                  break;

             case moveForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)moveFormEventHandler);

                  processed = true;
                  break;

             case gotoForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)gotoFormEventHandler);

                  processed = true;
                  break;

             case xlevForm:
                  FrmSetEventHandler(frm, 
                    (FormEventHandlerPtr)xlevFormEventHandler);

                  processed = true;
                  break;

             default:
                  break;
           }
         }
         break;

    case keyDownEvent:
         {
           // we have to handle this case specially for the editForm :))
           if (WinGetActiveWindow() == (WinHandle)FrmGetFormPtr(editForm)) {
             processed = editFormEventHandler(event);
           }
         }
         break;

    case ctlSelectEvent:

         switch (event->data.ctlSelect.controlID)
         {
           case globalFormHelpButton:

                // regenerate menu event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = menuItemHelp;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           case globalFormAboutButton:

                // regenerate menu event
                {
                  EventType event;

                  MemSet(&event, sizeof(EventType), 0);
                  event.eType            = menuEvent;
                  event.data.menu.itemID = menuItemAbout;
                  EvtAddEventToQueue(&event);
                }
                processed = true;
                break;

           default:
                break;
         }
         break;

    case winEnterEvent:
         {
           if (event->data.winEnter.enterWindow ==
                (WinHandle)FrmGetFormPtr(gameForm)) {

             // when game screen is active, animate
             globals->evtTimeOut = 1;
             processed           = true;
           }
         }
         break;

    case winExitEvent:
         {
           if (event->data.winExit.exitWindow ==
                (WinHandle)FrmGetFormPtr(gameForm)) {

             // when game screen is not active, stop animation
             globals->evtTimeOut = evtWaitForever;
             processed           = true;
           }
         }
         break;
         
    case menuEvent:

         // erase the menu status
         MenuEraseStatus(MenuGetActiveMenu());

         // what menu?
         switch (event->data.menu.itemID) 
         {
           case menuItemGrayscale:
                ApplicationDisplayDialog(grayForm);

                processed = true;
                break;

           case menuItemConfig:
                ApplicationDisplayDialog(cfigForm);

                processed = true;
                break;

#ifdef PROTECTION_ON
           case menuItemRegistration:
                ApplicationDisplayDialog(regiForm);
                RegisterShowMessage(globals->prefs);
                processed = true;
                break;
#endif

           case menuItemHelp:

                // need to know whuch form is calling for help
                FtrSet(appCreator, ftrHelpGlobalsActiveForm,
                       (UInt32)FrmGetActiveFormID());

                ApplicationDisplayDialog(helpForm);
                FtrUnregister(appCreator, ftrHelpGlobalsActiveForm);

                processed = true;
                break;

           case menuItemAbout:
                ApplicationDisplayDialog(infoForm);

                processed = true;
                break;

           default:
                break;
         }
         break;

    default:
         break;
  }

  return processed;
}

/**
 * Display a MODAL dialog to the user (this is a modified FrmDoDialog)
 *
 * @param formID the ID of the form to display.
 */
void
ApplicationDisplayDialog(UInt16 formID)
{
  FormActiveStateType frmCurrState;
  FormType            *frmActive      = NULL;
  WinHandle           winDrawWindow   = NULL;
  WinHandle           winActiveWindow = NULL;
  Globals             *globals;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // save the active form/window
  if (DeviceSupportsVersion(romVersion3)) {
    FrmSaveActiveState(&frmCurrState);
  }
  else {
    frmActive       = FrmGetActiveForm();
    winDrawWindow   = WinGetDrawWindow();
    winActiveWindow = WinGetActiveWindow();  // < palmos3.0, manual work
  }

  {
    EventType event;
    UInt16    err;
    Boolean   keepFormOpen;

    MemSet(&event, sizeof(EventType), 0);

    // send a load form event
    event.eType = frmLoadEvent;
    event.data.frmLoad.formID = formID;
    EvtAddEventToQueue(&event);

    // send a open form event
    event.eType = frmOpenEvent;
    event.data.frmLoad.formID = formID;
    EvtAddEventToQueue(&event);

    // handle all events here (trap them before the OS does) :)
    keepFormOpen = true;
    while (keepFormOpen) {

      EvtGetEvent(&event, globals->evtTimeOut);

      // this is our exit condition! :)
      keepFormOpen = (event.eType != frmCloseEvent);

      if (!ApplicationHandleEvent(&event))
        if (!SysHandleEvent(&event)) 
          if (!MenuHandleEvent(0, &event, &err)) 
            FrmDispatchEvent(&event);

      if (event.eType == appStopEvent) {
        keepFormOpen = false;
        EvtAddEventToQueue(&event);  // tap "applications", need to exit
      }
    }
  }

  // restore the active form/window
  if (DeviceSupportsVersion(romVersion3)) {
    FrmRestoreActiveState(&frmCurrState);
  }
  else {
    FrmSetActiveForm(frmActive);
    WinSetDrawWindow(winDrawWindow);
    WinSetActiveWindow(winActiveWindow);     // < palmos3.0, manual work
  }
}

/**
 * The Palm Computing Platform event processing loop.
 */
void  
EventLoop()
{
  EventType event;
  UInt16    err;
  FormType  *frm;
  Globals   *globals;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // reset the timer (just in case)
  globals->timerPointA = TimGetTicks();

  do 
  {
    EvtGetEvent(&event, globals->evtTimeOut);
    frm = FrmGetActiveForm();

    //
    // ANIMATION HANDLING:
    //

    // nil event occured, register time state
    if (
        (WinGetActiveWindow() == (WinHandle)frm) &&
        (frm == FrmGetFormPtr(gameForm)) &&
        (event.eType == nilEvent)
       )
    {
      globals->timerPointA = TimGetTicks();
    }

    //
    // EVENT HANDLING:
    //

    if (!ApplicationHandleEvent(&event)) 
      if (!SysHandleEvent(&event)) 
        if (!MenuHandleEvent(0, &event, &err)) 
          FrmDispatchEvent(&event);

    //
    // ANIMATION HANDLING:
    //

    // on a form that requires animations, calc time since last nilEvent
    if (
        (WinGetActiveWindow() == (WinHandle)frm) &&
        (frm == FrmGetFormPtr(gameForm))
       )
    {
      globals->timerPointB = TimGetTicks();

      // calculate the delay required
      globals->timerDiff = (globals->timerPointB - globals->timerPointA);
      globals->evtTimeOut = (globals->timerDiff > globals->ticksPerFrame) ?
        1 : (globals->ticksPerFrame - globals->timerDiff);

      // manually add nilEvent if needed (only when pen held down)
      if ((globals->evtTimeOut <= 1) && (event.eType == penMoveEvent))
      {
        EventType event;

        MemSet(&event, sizeof(EventType), 0);
        event.eType = nilEvent;
        EvtAddEventToQueue(&event);
      }
    }
  } while (event.eType != appStopEvent);
}

/**
 * The Palm Computing Platform termination routine.
 */
void  
EndApplication()
{
  Globals *globals;
  UInt16  prefSize;

  // get globals reference
  FtrGet(appCreator, ftrGlobals, (UInt32 *)&globals);

  // restore the key state
  KeySetMask(keyBitsAll);

  // terminate the game environment
  RegisterTerminate();

  // ensure all forms are closed
  FrmCloseAllForms();

  // reset the grayscale settings - as appropriate
  if (DeviceSupportsGrayscale()) {
    DeviceGrayscale(graySet, &globals->lgray, &globals->dgray);
  }

  // save preferences
  MemPtrFree(globals->prefs->system.hotSyncUsername);
  globals->prefs->system.hotSyncUsername = NULL;

  // lets add our 'check' data chunk
  prefSize = sizeof(PreferencesType);
  StrCopy(globals->prefs->system.signature, "|HaCkMe|");
  RegisterDecryptChunk((UInt8 *)globals->prefs, prefSize, 0x0007, 0x01);
  PrefSetAppPreferences(appCreator, 0, 1, globals->prefs, prefSize, true);
  MemPtrFree(globals->prefs);
}
