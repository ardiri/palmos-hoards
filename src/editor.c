/*
 * @(#)editor.c
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

// global variable structure
typedef struct
{
  UInt16    card;
  LocalID   dbID;
  DmOpenRef databaseRef;
  Boolean   needBackup;

  UInt32    keyMask;
  Boolean   inEditor;
  WinHandle winTools;
} EditorGlobals;

// interface
static void EditorDrawTile(GameTileType, Coord, Coord)                __GAME__;

/**
 * Initialize the Editor.
 *
 * @param strDatabaseName the name of the level database.
 * @return true if successful, false otherwise.
 */  
Boolean   
EditorInitialize(Char *strDatabaseName, Boolean inEditor)
{
  EditorGlobals *globals;
  Boolean       result = true;

  // create the globals object, and register it
  globals = (EditorGlobals *)MemPtrNew(sizeof(EditorGlobals));
  MemSet(globals, sizeof(EditorGlobals), 0);
  FtrSet(appCreator, ftrEditorGlobals, (UInt32)globals);

  // config
  globals->inEditor = inEditor;

  // we ONLY do this if we are in the editor
  if (globals->inEditor) {

    Err err = errNone;

    // setup the valid keys available at this point in time
    globals->keyMask = KeySetMask(~(keyBitsAll ^
                                   (keyBitPower   | keyBitCradle   |
                                    keyBitPageUp  | keyBitPageDown |
                                    keyBitHard1   | keyBitHard2    |
                                    keyBitHard3   | keyBitHard4    |
                                    keyBitAntenna | keyBitContrast)));

    // initialize our "bitmap" windows
    globals->winTools =
      WinCreateOffscreenWindow(150, 8, screenFormat, &err);
    err |= (globals->winTools == NULL);

    // no problem creating back buffers?
    if (err == errNone) {
    
      WinHandle currWindow;
      MemHandle bitmapHandle;

      currWindow = WinGetDrawWindow();

      WinSetDrawWindow(globals->winTools);
      bitmapHandle = DmGet1Resource('Tbmp', bitmapTools);
      WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
      MemHandleUnlock(bitmapHandle);
      DmReleaseResource(bitmapHandle);

      WinSetDrawWindow(currWindow);
    }

    // something went wrong, display memory error
    else {

      ApplicationDisplayDialog(xmemForm);
      result = false;
    }
  }

  // open the database
  EditorOpenDatabase(strDatabaseName);

  // do we have a valid "database" reference?
  if (globals->databaseRef == NULL) {

    // display message
    ApplicationDisplayDialog(xlevForm);
    result = false;
  }

  return result;
}

/**
 * Create a new database on the device.
 *
 * @param strDatabaseName the name of the level database.
 * @return true if successful, false otherwise
 */
Boolean 
EditorCreateDatabase(Char *strDatabaseName)
{
  EditorGlobals *globals;
  Boolean       result = true;
  UInt16        card, err;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // search for database...
  if ((DmFindDatabase(0, strDatabaseName) == 0) &&
      (DmFindDatabase(1, strDatabaseName) == 0)) {

    // create the database
    card = 0;
    err  = 
      DmCreateDatabase(card, strDatabaseName, appCreator, levelType, false);
    result = (err == errNone);
  }

  // database exists.. barf..
  else 
    result = false;

  return result;
}

/**
 * Open a database for editing or modification
 *
 * @param strDatabaseName the name of the level database.
 */
void
EditorOpenDatabase(Char *strDatabaseName)
{
  EditorGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  globals->card        = 0;
  globals->dbID        = DmFindDatabase(globals->card, strDatabaseName);

  // what? not there.. must be on card 1
  if (globals->dbID == NULL) {
    globals->card      = 1;
    globals->dbID      = DmFindDatabase(globals->card, strDatabaseName);
  }

  // cannot open "read only" image in editor?
  if (globals->inEditor && (MemLocalIDKind(globals->dbID) != memIDHandle))
    globals->databaseRef = NULL;

  // open as required
  else 
    globals->databaseRef = 
      DmOpenDatabase(globals->card, globals->dbID, 
                     (globals->inEditor ? dmModeReadWrite : dmModeReadOnly));
}

/**
 * Determine the number of levels in the current open database.
 *
 * @return the number of levels in the currently open database.
 */  
UInt8
EditorLevelCount()
{
  EditorGlobals *globals;
  UInt16        result = 0;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  result = DmNumRecords(globals->databaseRef) & 0xff;

  return result;
}

/**
 * Create a level in the current open database.
 *
 * @param level a pointer to the level structure.
 */  
void
EditorCreateLevel(LevelType *level)
{
  EditorGlobals *globals;
  MemPtr        *ptrLevel;
  UInt16        index;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // clear level
  MemSet(level, sizeof(LevelType), 0);
  index = EditorLevelCount();

  // create record and save
  ptrLevel = (MemPtr)MemHandleLock(
    DmNewRecord(globals->databaseRef, &index, levelSize));
  DmWrite(ptrLevel, 0, (MemPtr)level->tiles, levelSize);
  MemPtrUnlock(ptrLevel);
  DmReleaseRecord(globals->databaseRef, index, true);

  // flag it for backup purposes
  globals->needBackup = true;
}

/**
 * Load a level from the current open database.
 *
 * @param level a pointer to the level structure.
 * @param levelNumber the record to load (first = 1)
 */  
void
EditorLoadLevel(LevelType *level, 
                UInt8     levelNumber)
{
  EditorGlobals *globals;
  MemPtr        *ptrLevel;
  UInt16        index;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // clear level
  MemSet(level, sizeof(LevelType), 0);

  index = (levelNumber - 1) % EditorLevelCount();

  // copy record into memory
  ptrLevel = (MemPtr)MemHandleLock(
    DmGetRecord(globals->databaseRef, index));
  MemMove((MemPtr)level->tiles, ptrLevel, levelSize);
  MemPtrUnlock(ptrLevel);
  DmReleaseRecord(globals->databaseRef, index, false);
}

/**
 * Save a level to the current open database.
 *
 * @param level a pointer to the level structure.
 * @param levelNumber the record to save (first = 1)
 */  
void
EditorSaveLevel(LevelType *level, 
                UInt8     levelNumber)
{
  EditorGlobals *globals;
  MemPtr        *ptrLevel;
  UInt16        index;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  index = (levelNumber - 1) % EditorLevelCount();

  // save memory into record
  ptrLevel = (MemPtr)MemHandleLock(
    DmGetRecord(globals->databaseRef, index));
  DmWrite(ptrLevel, 0, (MemPtr)level->tiles, levelSize);
  MemPtrUnlock(ptrLevel);
  DmReleaseRecord(globals->databaseRef, index, true);

  // flag it for backup purposes
  globals->needBackup = true;
}

/**
 * Move on level to a desired location in the current open database.
 *
 * @param srcLevelNumber the record to move (first = 1)
 * @param dstLevelNumber the new record location
 */  
void
EditorMoveLevel(UInt8 srcLevelNumber, 
                UInt8 dstLevelNumber)
{
  EditorGlobals *globals;
  UInt16        srcIndex;
  UInt16        dstIndex;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  srcIndex = (srcLevelNumber - 1) % EditorLevelCount();
  dstIndex = (dstLevelNumber - 1) % (EditorLevelCount() + 1);
  DmMoveRecord(globals->databaseRef, srcIndex, dstIndex);

  // flag it for backup purposes
  globals->needBackup = true;
}

/**
 * Process a pen-tap/move from the user on the specific level.
 *
 * @param level a pointer to the level structure.
 * @param tile the current editing tool
 * @param x the x-coordinate of the tap (relative to level area)
 * @param y the y-coordinate of the tap (relative to level area)
 * @param supportXOR does the XOR rule apply?
 */  
void
EditorProcessPenTap(LevelType    *level, 
                    GameTileType tile, 
		    Coord        x, 
		    Coord        y, 
		    Boolean      supportXOR)
{
  EditorGlobals *globals;
  UInt16        i, j;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // determine which tile should be modified
  x = x / 6;
  y = y / 8;

  // XOR effect
  if ((level->tiles[x][y].tool == tile.tool) && supportXOR) {

    // remove the tile
    tile.tool       = toolEmpty;
    tile.properties = 0;
  }

  // only ONE player allowed
  else
  if (tile.tool == toolPlayer) {

    for (j=0; j<GRID_HEIGHT; j++) {
      for (i=0; i<GRID_WIDTH; i++) {
        if (level->tiles[i][j].tool == toolPlayer) {
          level->tiles[i][j].tool       = toolEmpty;
          level->tiles[i][j].properties = 0;
          EditorDrawTile(level->tiles[i][j], i, j);
	}
      }
    }
  }

  // limited number of monks allowed
  else
  if (tile.tool == toolMonk) {

    UInt16 count = 0;

    for (j=0; j<GRID_HEIGHT; j++) {
      for (i=0; i<GRID_WIDTH; i++) {
        if (level->tiles[i][j].tool == toolMonk) {

          count++;

	  // remove the last counted guard (will be closest to bottom)
	  if (count == (MAX_MONKS-1)) {
            level->tiles[i][j].tool       = toolEmpty;
            level->tiles[i][j].properties = 0;
            EditorDrawTile(level->tiles[i][j], i, j);
	  }
	}
      }
    }
  }

  // make the change to the level
  level->tiles[x][y].tool       = tile.tool;
  level->tiles[x][y].properties = tile.properties;

  // update the screen
  EditorDrawTile(level->tiles[x][y], x, y);

  // blit background buffer to the screen
  GraphicsRepaint();
}

/**
 * Draw a level to the screen.
 *
 * @param level a pointer to the level structure.
 */  
void
EditorDrawLevel(LevelType *level)
{
  EditorGlobals *globals;
  UInt16        i, j;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // clear the background buffer
  GraphicsClear();

  // draw the level
  for (j=0; j<GRID_HEIGHT; j++) {
    for (i=0; i<GRID_WIDTH; i++) {
      EditorDrawTile(level->tiles[i][j], i, j);
    }
  }

  // blit background buffer to the screen
  GraphicsRepaint();
}

/**
 * Delete a specific level from the database.
 *
 * @param level a pointer to the level structure.
 * @param levelNumber the record to delete (first = 1)
 */  
void
EditorDeleteLevel(UInt8 levelNumber)
{
  EditorGlobals *globals;
  UInt16        index;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  index = levelNumber - 1;
  DmRemoveRecord(globals->databaseRef, index);

  // flag it for backup purposes
  globals->needBackup = true;
}

/**
 * Close the open database.
 */
void
EditorCloseDatabase()
{
  EditorGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // do we have a database to close?
  if (globals->databaseRef != NULL) {

    // close it
    DmCloseDatabase(globals->databaseRef);

    // do we need to set the backup flag?
    if (globals->needBackup) {

      UInt16 dbAttributes;

      DmDatabaseInfo(globals->card, globals->dbID, NULL, &dbAttributes, 
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
      dbAttributes |= dmHdrAttrBackup | dmHdrAttrBundle;
      DmSetDatabaseInfo(globals->card, globals->dbID, NULL, &dbAttributes, 
                     NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
    }
  }
}

/**
 * Terminate the Editor.
 */
void   
EditorTerminate()
{
  EditorGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  // close the database
  EditorCloseDatabase();

  // we ONLY do this if we are in the editor
  if (globals->inEditor) {

    // return the state of the key processing
    KeySetMask(globals->keyMask);

    // clean up windows
    WinDeleteWindow(globals->winTools, false);
  }

  // clean up memory
  MemPtrFree(globals);                          // mem leak - fixed 24-Dec-2000

  // unregister global data
  FtrUnregister(appCreator, ftrEditorGlobals);
}

/**
 * Draw a tile onto the backgroud buffer.
 *
 * @param tile the tile to draw
 * @param x the x-coordinate in the level array (position)
 * @param y the y-coordinate in the level array (position)
 */
static void
EditorDrawTile(GameTileType tile, 
               Coord        x, 
               Coord        y)
{
  EditorGlobals *globals;
  RectangleType scrRect, rect;

  // get a globals reference
  FtrGet(appCreator, ftrEditorGlobals, (UInt32 *)&globals);

  rect.topLeft.x    = 0;  // will change
  rect.topLeft.y    = 0;
  rect.extent.x     = 6;
  rect.extent.y     = 8;
  scrRect.topLeft.x = x * rect.extent.x + 2;
  scrRect.topLeft.y = y * rect.extent.y;
  scrRect.extent.x  = 6;
  scrRect.extent.y  = 8;

  // update the screen
  rect.topLeft.x = tile.tool * (rect.extent.x + 2);
  WinCopyRectangle(globals->winTools, GraphicsGetDrawWindow(),
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
}
