/*
 * @(#)game.c
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
  UInt32    keyMask;

  WinHandle winTools;
  WinHandle winHoles;

  WinHandle winPlayers;
  WinHandle winPlayersMask;
  WinHandle winPlayerBackup;

  WinHandle winMonks;
  WinHandle winMonksMask;
  WinHandle winMonkBackup[MAX_MONKS];

  UInt8     startLevel;                     // at what stage did they start?

  Boolean   goldCollected;                  // has all the gold been collected?
  Boolean   playerDied;                     // has the player died?

  struct {

    Boolean    gamePadPresent;              // is the gamepad driver present
    UInt16     gamePadLibRef;               // library reference for gamepad

  } hardware;

} GameGlobals;

// interface
static void    GameAdjustLevel(PreferencesType *)                     __GAME__;
static void    GameMovePlayer(PreferencesType *)                      __GAME__;
static Boolean GameMovePlayerTask(PreferencesType *)                  __GAME__;
static Boolean GameMovePlayerHorizontally(PreferencesType *)          __GAME__;
static Boolean GameMovePlayerVerticlly(PreferencesType *)             __GAME__;
static void    GameMoveMonks(PreferencesType *)                       __GAME__;
static Boolean GameMonkInPath(PreferencesType *, UInt16)              __GAME__;
static Boolean GameMonkStationaryAt(PreferencesType *, Coord, Coord)  __GAME__;
static Boolean GameMonkBelowPlayer(PreferencesType *)                 __GAME__;
static void    GameMoveOthers(PreferencesType *)                      __GAME__;
static Boolean GameTilePosition(PositionType *)                       __GAME__;
static void    GameGoldCheck(PreferencesType *)                       __GAME__;
static void    GameDrawTile(GameTileType, Coord, Coord)               __GAME__;
static void    GameBackupTile(Coord, Coord, WinHandle)                __GAME__;
static void    GameRestoreTile(Coord, Coord, WinHandle)               __GAME__;

/**
 * Initialize the Game.
 *
 * @param prefs the global preference data.
 * @return true if successful, false otherwise.
 */  
Boolean   
GameInitialize(PreferencesType *prefs)
{
  GameGlobals *globals;
  Err         err;
  Boolean     result = true;

  // create the globals object, and register it
  globals = (GameGlobals *)MemPtrNew(sizeof(GameGlobals));
  MemSet(globals, sizeof(GameGlobals), 0);
  FtrSet(appCreator, ftrGameGlobals, (UInt32)globals);

  // setup the valid keys available at this point in time
  globals->keyMask = KeySetMask(~(keyBitsAll ^
                                (keyBitPower   | keyBitCradle | 
                                 keyBitAntenna | keyBitContrast)));

  // load the gamepad driver if available
  {
    Err err;

    // attempt to load the library
    err = SysLibFind(GPD_LIB_NAME,&globals->hardware.gamePadLibRef);
    if (err == sysErrLibNotFound)
      err = SysLibLoad('libr',GPD_LIB_CREATOR,&globals->hardware.gamePadLibRef);

    // lets determine if it is available
    globals->hardware.gamePadPresent = (err == errNone);

    // open the library if available
    if (globals->hardware.gamePadPresent)
      GPDOpen(globals->hardware.gamePadLibRef);
  }

  // initialize our "bitmap" windows
  err = errNone;
  {
    Err    e;
    Int16  i;

    globals->winTools =
      WinCreateOffscreenWindow(150, 8, screenFormat, &e); err |= e;
    err |= (globals->winTools == NULL);

    globals->winHoles =
      WinCreateOffscreenWindow(30, 8, screenFormat, &e); err |= e;
    err |= (globals->winHoles == NULL);

    globals->winPlayers =
      WinCreateOffscreenWindow(48, 32, screenFormat, &e); err |= e;
    err |= (globals->winPlayers == NULL);
    globals->winPlayersMask =
      WinCreateOffscreenWindow(48, 32, screenFormat, &e); err |= e;
    err |= (globals->winPlayersMask == NULL);
    globals->winPlayerBackup = 
      WinCreateOffscreenWindow(6, 8, screenFormat, &e); err |= e;
    err |= (globals->winPlayerBackup == NULL);

    globals->winMonks =
      WinCreateOffscreenWindow(48, 32, screenFormat, &e); err |= e;
    err |= (globals->winMonks == NULL);
    globals->winMonksMask =
      WinCreateOffscreenWindow(48, 32, screenFormat, &e); err |= e;
    err |= (globals->winMonksMask == NULL);
    for (i=0; i<MAX_MONKS; i++) { 
      globals->winMonkBackup[i] = 
        WinCreateOffscreenWindow(6, 8, screenFormat, &e); err |= e;
      err |= (globals->winMonkBackup[i] == NULL);
    }
  }

  // no problems creating back buffers? load images.
  if (err == errNone) {
  
    WinHandle currWindow;
    MemHandle bitmapHandle;

    currWindow = WinGetDrawWindow();

    // tools
    WinSetDrawWindow(globals->winTools);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapTools);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // holes
    WinSetDrawWindow(globals->winHoles);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapHoles);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // players
    WinSetDrawWindow(globals->winPlayers);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapPlayers);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);
    WinSetDrawWindow(globals->winPlayersMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapPlayersMask);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);

    // monks
    WinSetDrawWindow(globals->winMonks);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapMonks);
    WinDrawBitmap((BitmapType *)MemHandleLock(bitmapHandle), 0, 0);
    MemHandleUnlock(bitmapHandle);
    DmReleaseResource(bitmapHandle);
    WinSetDrawWindow(globals->winMonksMask);
    bitmapHandle = DmGet1Resource('Tbmp', bitmapMonksMask);
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

  // lets keep track of the starting level
  globals->startLevel = prefs->config.startLevel;

  // open the database
  result &= EditorInitialize(prefs->game.strLevelSetName, false);

  return result;
}

/**
 * Is the active game a high score candidate?
 *
 * @param prefs the global preference data.
 * @return true if valid, false otherwise.
 */
Boolean 
GameHighScoreCandidate(PreferencesType *prefs) 
{
  GameGlobals *globals;
  Boolean     result;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  result = ((StrCompare(prefs->game.strLevelSetName, "LODE_ORIGINAL") == 0) &&
            (globals->startLevel == 1));

  return result; 
}

/**
 * Reset the Game preferences.
 *
 * @param prefs the global preference data.
 */
void
GameResetPreferences(PreferencesType *prefs)
{
  GameGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // now we are playing
  prefs->game.gamePlaying               = true;
  prefs->game.gamePaused                = false;
  prefs->game.gameWait                  = true;
  prefs->game.gameAnimationCount        = 0;

  // reset score and lives
  prefs->game.gameScore                 = 0;
  prefs->game.gameLives                 = 3;

  // what level are we starting at?
  if (prefs->config.startLevel > EditorLevelCount()) {
    prefs->game.loderunner.stageNumber  = EditorLevelCount();
  }
  else 
    prefs->game.loderunner.stageNumber  = prefs->config.startLevel;

  // get the game ready for playing
  GameAdjustLevel(prefs);
}

/**
 * Process key input from the user.
 *
 * @param prefs the global preference data.
 * @param keyStatus the current key state.
 */
void
GameProcessKeyInput(PreferencesType *prefs, UInt32 keyStatus)
{
  GameGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  keyStatus &= (prefs->config.ctlKeyLeftDig  |
                prefs->config.ctlKeyRightDig |
                prefs->config.ctlKeyUp       |
                prefs->config.ctlKeyDown     |
                prefs->config.ctlKeyLeft     |
                prefs->config.ctlKeyRight);

  // additonal checks here
  if (globals->hardware.gamePadPresent) {

    UInt8 gamePadKeyStatus;
    Err   err;

    // read the state of the gamepad
    err = GPDReadInstant(globals->hardware.gamePadLibRef, &gamePadKeyStatus);
    if (err == errNone) {

      // process
      if  ((gamePadKeyStatus & GAMEPAD_RIGHTFIRE) != 0) 
         keyStatus |= prefs->config.ctlKeyRightDig;
      if  ((gamePadKeyStatus & GAMEPAD_LEFTFIRE)  != 0)
        keyStatus |= prefs->config.ctlKeyLeftDig;
      if  ((gamePadKeyStatus & GAMEPAD_DOWN)      != 0)
         keyStatus |= prefs->config.ctlKeyDown;
      if  ((gamePadKeyStatus & GAMEPAD_UP)        != 0)
         keyStatus |= prefs->config.ctlKeyUp;
      if  ((gamePadKeyStatus & GAMEPAD_LEFT)      != 0)
         keyStatus |= prefs->config.ctlKeyLeft;
      if  ((gamePadKeyStatus & GAMEPAD_RIGHT)     != 0)
         keyStatus |= prefs->config.ctlKeyRight;

      // special purpose :)
      if  ((gamePadKeyStatus & GAMEPAD_SELECT)    != 0) {

        // wait until they let it go :)
        do {
          GPDReadInstant(globals->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_SELECT) != 0);

        keyStatus = 0;
        prefs->game.gamePaused = !prefs->game.gamePaused;
      }
      if  ((gamePadKeyStatus & GAMEPAD_START)     != 0) {

        // wait until they let it go :)
        do {
          GPDReadInstant(globals->hardware.gamePadLibRef, &gamePadKeyStatus);
        } while ((gamePadKeyStatus & GAMEPAD_START) != 0);

        keyStatus = 0;
        GameKillPlayer(prefs);
      }
    }
  }

  // did they press at least one of the game keys?
  if (keyStatus != 0) {

    Coord x, y;

    // if they were waiting, we should reset the game animation count
    if (prefs->game.gameWait) {
      prefs->game.gameAnimationCount = 0;
      prefs->game.gameWait           = false;
    }

    // whats the players current position?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // move up?
    if ((y > 0) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        (keyStatus & prefs->config.ctlKeyUp)) {

      prefs->game.loderunner.playerTarget.x = x;
      prefs->game.loderunner.playerTarget.y = y-1;
    }
    else

    // move down?
    if ((y < (GRID_HEIGHT-1)) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        (keyStatus & prefs->config.ctlKeyDown)) {

      prefs->game.loderunner.playerTarget.x = x;
      prefs->game.loderunner.playerTarget.y = y+1;
    }
    else

    // move left?
    if ((x > 0) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        (prefs->game.loderunner.playerDir != moveDown) &&
        (prefs->game.loderunner.playerDir != moveUp) &&
        (keyStatus & prefs->config.ctlKeyLeft)) {

      prefs->game.loderunner.playerTarget.x = x-1;
      prefs->game.loderunner.playerTarget.y = y;
    }
    else

    // move right?
    if ((x < (GRID_WIDTH-1)) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        (prefs->game.loderunner.playerDir != moveDown) &&
        (prefs->game.loderunner.playerDir != moveUp) &&
        (keyStatus & prefs->config.ctlKeyRight)) {

      prefs->game.loderunner.playerTarget.x = x+1;
      prefs->game.loderunner.playerTarget.y = y;
    }
    else

    // dig left?
    if ((x > 0) &&
        (y < (GRID_HEIGHT-1)) &&

        // its a brick?
	(prefs->game.loderunner.stage.tileState[x-1][y+1] == 255) &&
	(prefs->game.loderunner.stage.tiles[x-1][y+1].tool == toolBrick) &&

        // ensure we can actually get to the next position (and we can dig)
        (prefs->game.loderunner.stage.tileState[x-1][y] != 255) &&
        (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolRope) &&
        (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolLadder) &&
        (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolFallThru) && 

        (keyStatus & prefs->config.ctlKeyLeftDig)) {

      prefs->game.loderunner.playerTarget.x = x-1;
      prefs->game.loderunner.playerTarget.y = y+1;
    }
    else

    // dig right?
    if ((x < (GRID_WIDTH-1)) &&
        (y < (GRID_HEIGHT-1)) &&

        // its a brick?
	(prefs->game.loderunner.stage.tileState[x+1][y+1] == 255) &&
	(prefs->game.loderunner.stage.tiles[x+1][y+1].tool == toolBrick) &&

        // ensure we can actually get to the next position (and we can dig)
        (prefs->game.loderunner.stage.tileState[x+1][y] != 255) &&
        (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolRope) &&
        (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolLadder) &&
        (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolFallThru) && 

        (keyStatus & prefs->config.ctlKeyRightDig)) {

      prefs->game.loderunner.playerTarget.x = x+1;
      prefs->game.loderunner.playerTarget.y = y+1;
    }

    // great! they wanna play
    prefs->game.gamePaused = false;
  }
}

/**
 * Process stylus input from the user.
 *
 * @param prefs the global preference data.
 * @param x the x co-ordinate of the stylus event.
 * @param y the y co-ordinate of the stylus event.
 */
void
GameProcessStylusInput(PreferencesType *prefs, Coord x, Coord y)
{
  GameGlobals    *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // if they were waiting, we should reset the game animation count
  if (prefs->game.gameWait) {
    prefs->game.gameAnimationCount = 0;
    prefs->game.gameWait           = false;
  }

  prefs->game.loderunner.playerTarget.x = x / TILE_WIDTH;
  prefs->game.loderunner.playerTarget.y = y / TILE_HEIGHT;

  // great! they wanna play
  prefs->game.gamePaused = false;
}

/**
 * Kill the player, and restart level.
 *
 * @param prefs the global preference data.
 */
void
GameKillPlayer(PreferencesType *prefs)
{
  SndCommandType deathSnd = {sndCmdFreqDurationAmp,0,2048,2,sndMaxAmp};
  Int16          i;
  GameGlobals    *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // lose a life :(
  prefs->game.gameLives--;

  // play a little death sound
  for (i=0; i<8; i++) {
    DevicePlaySound(&deathSnd);
    SysTaskDelay(2); // small deley

    deathSnd.param1 = deathSnd.param1 >> 1;
  }
  DevicePlaySound(&deathSnd);

  // no move lives left: GAME OVER!
  if (prefs->game.gameLives == 0) {

    EventType event;

    // return to main screen
    MemSet(&event, sizeof(EventType), 0);
    event.eType            = menuEvent;
    event.data.menu.itemID = gameMenuItemExit;
    EvtAddEventToQueue(&event);
  }

  // reset level position and continue game
  else {
    GameAdjustLevel(prefs);

    // lets draw the level and continue!
    GameDrawLevel(prefs);
  }
}

/**
 * Process the object movement in the game.
 *
 * @param prefs the global preference data.
 */
void
GameMovement(PreferencesType *prefs)
{
  GameGlobals    *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  //
  // the game is NOT paused.
  //

  if (!prefs->game.gamePaused) {

    // we must make sure the user is ready for playing
    if (!prefs->game.gameWait) {

      // we cannot be dead yet :)
      globals->playerDied = false;

      // 
      // standard game play, draw, move and move
      // 

      // movement inside game: monk, other then player
      GameMoveMonks(prefs);
      GameMoveOthers(prefs);
      GameMovePlayer(prefs);

      // draw the current game screen
      GameDraw(prefs);

      // 
      // has the player died in this frame?
      // 

      if (globals->playerDied) 
        GameKillPlayer(prefs);
    }

    // we have to display "GET READY"
    else {

      // flash on:
      if ((prefs->game.gameAnimationCount % GAME_FPS) == 0) {

        Char   str[32];
        FontID currFont = FntGetFont();

        // draw the current game screen
        GameDraw(prefs);

        StrCopy(str, "    * GET READY *    ");
        FntSetFont(boldFont);
        WinDrawChars(str, StrLen(str),
                     80 - (FntCharsWidth(str, StrLen(str)) >> 1), 74);
        FntSetFont(currFont);
      }

      // flash off
      if ((prefs->game.gameAnimationCount % GAME_FPS) == (GAME_FPS >> 1)) {

        // draw the current game screen
        GameDraw(prefs);
      }
    }

    // update the animation counter
    prefs->game.gameAnimationCount++;
  }

  //
  // the game is paused.
  //

  else {
    Char   str[32];
    FontID currFont = FntGetFont();

    StrCopy(str, "    *  PAUSED  *    ");
    FntSetFont(boldFont);
    WinDrawChars(str, StrLen(str),
                 80 - (FntCharsWidth(str, StrLen(str)) >> 1), 74);
    FntSetFont(currFont);
  }
}

/**
 * Draw the level on the screen.
 *
 * @param prefs the global preference data.
 */
void
GameDrawLevel(PreferencesType *prefs)
{
  GameGlobals   *globals;
  Int16         i, j;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // lets double check the "gold" state
  GameGoldCheck(prefs);

  // update the "background" graphics for the level
  for (j=0; j<GRID_HEIGHT; j++) {
    for (i=0; i<GRID_WIDTH; i++) {
      GameDrawTile(prefs->game.loderunner.stage.tiles[i][j], i, j);
    }
  }
}

/**
 * Draw the game on the screen.
 *
 * @param prefs the global preference data.
 */
void
GameDraw(PreferencesType *prefs)
{
  GameGlobals   *globals;
  Char          displayStr[8];
  FontID        currFont;
  RectangleType scrRect, rect;
  Int16         i, j;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  currFont = FntGetFont();

  //
  // DRAW BITMAPS ON SCREEN
  //

  // "overlay" the monks and players - (keep backup of background)
  {
    GameTileType tile;

    switch (prefs->game.loderunner.playerDir) 
    {
      case moveNone:

           // standard upright?
           if (prefs->game.loderunner.playerRecCount == 0) {
             tile.tool       = tilePlayerUpright; 
             tile.properties = 0;
           }

           // recovery!! :)
           else {
             tile.tool       = tilePlayerGenerating; 
             tile.properties = 8 - prefs->game.loderunner.playerRecCount;
           }
           break;

      case digLeft:
      case digRight:

           // determine the bitmap to draw :)
           tile.tool       = tilePlayerUpright; 
           tile.properties = 0;                 // standard upright!
           break;

      case moveUp:
      case moveDown:

           // determine the bitmap to draw :)
           tile.tool       = tilePlayerUpright; 
           tile.properties = (prefs->game.loderunner.playerPosition.y % 4)+1;
           break;

      case moveLeft:
           {
             Coord x,y;

             // what tile we one?
             x = (prefs->game.loderunner.playerPosition.x + 3) / TILE_WIDTH;
             y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

             // determine the bitmap to draw :)
             tile.tool       = tilePlayerLeft; 
             tile.properties = 
               (prefs->game.loderunner.stage.tiles[x][y].tool != toolRope)
               ? prefs->game.loderunner.playerPosition.x % 6
               : ((prefs->game.loderunner.playerPosition.x % 4) >> 1) + 6;
           }
           break;
  
      case moveRight:
           {
             Coord x,y;

             // what tile we one?
             x = (prefs->game.loderunner.playerPosition.x + 3) / TILE_WIDTH;
             y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

             // determine the bitmap to draw :)
             tile.tool       = tilePlayerRight; 
             tile.properties = 
               (prefs->game.loderunner.stage.tiles[x][y].tool != toolRope)
               ? prefs->game.loderunner.playerPosition.x % 6
               : ((prefs->game.loderunner.playerPosition.x % 4) >> 1) + 6;
           }
           break;

      default:
           break;
    }

    // backup the "bit behind" the image we are about to blit
    GameBackupTile(prefs->game.loderunner.playerPosition.x,
                   prefs->game.loderunner.playerPosition.y,
                   globals->winPlayerBackup);

    // draw the tile to the back buffer (so we can display it)
    GameDrawTile(tile, 
                 prefs->game.loderunner.playerPosition.x,
                 prefs->game.loderunner.playerPosition.y);
  }

  // draw the monks
  for (i=0; i<prefs->game.loderunner.monkCount; i++) { 
  
    GameTileType tile;

    switch (prefs->game.loderunner.monkDir[i]) 
    {
      case moveNone:

           // standard upright?
           if (prefs->game.loderunner.monkRecCount[i] == 0) {

             // is the monk about to wiggle out of the hole?
	     if (prefs->game.loderunner.monkWait[i] < 4) {

               tile.tool       = tileMonkUpright; 
               tile.properties = prefs->game.loderunner.monkWait[i];
             }
	     else 

	     // just a standard upright standing position
	     {
               tile.tool       = tileMonkUpright; 
               tile.properties = 0;
             }
           }

           // recovery!! :)
           else {
             tile.tool       = tileMonkGenerating; 
             tile.properties = 8 - prefs->game.loderunner.monkRecCount[i];
           }

           break;

      case moveUp:
      case moveDown:

           // determine the bitmap to draw :)
           tile.tool       = tileMonkUpright; 
           tile.properties = prefs->game.loderunner.monkPosition[i].y % 4;
           break;

      case moveLeft:
           {
             Coord x,y;

             // what tile we one?
             x = (prefs->game.loderunner.monkPosition[i].x + 3) / TILE_WIDTH;
             y = prefs->game.loderunner.monkPosition[i].y / TILE_HEIGHT;

             // determine the bitmap to draw :)
             tile.tool       = tileMonkLeft; 
             tile.properties = 
               (prefs->game.loderunner.stage.tiles[x][y].tool != toolRope)
               ? prefs->game.loderunner.monkPosition[i].x % 6
               : (prefs->game.loderunner.monkPosition[i].x % 2) + 6;
           }
           break;
  
      case moveRight:
           {
             Coord x,y;

             // what tile we one?
             x = (prefs->game.loderunner.monkPosition[i].x + 3) / TILE_WIDTH;
             y = prefs->game.loderunner.monkPosition[i].y / TILE_HEIGHT;

             // determine the bitmap to draw :)
             tile.tool       = tileMonkRight; 
             tile.properties = 
               (prefs->game.loderunner.stage.tiles[x][y].tool != toolRope)
               ? prefs->game.loderunner.monkPosition[i].x % 6
               : (prefs->game.loderunner.monkPosition[i].x % 2) + 6;
           }
           break;

      default:
           break;
    }

    // backup the "bit behind" the image we are about to blit
    GameBackupTile(prefs->game.loderunner.monkPosition[i].x,
                   prefs->game.loderunner.monkPosition[i].y,
                   globals->winMonkBackup[i]);

    // draw the tile to the back buffer (so we can display it)
    GameDrawTile(tile, 
                 prefs->game.loderunner.monkPosition[i].x,
                 prefs->game.loderunner.monkPosition[i].y);
  }

  // shift background buffer to the real screen
  GraphicsRepaint();

  // "underlay" the monks and players - restore "original" :)

  for (i=0; i<prefs->game.loderunner.monkCount; i++) {
  
    j = prefs->game.loderunner.monkCount - i - 1;

    // restore the "bit behind" the image we blitted
    GameRestoreTile(prefs->game.loderunner.monkPosition[j].x,
                    prefs->game.loderunner.monkPosition[j].y,
                    globals->winMonkBackup[j]);
  }
  // restore the "bit behind" the image we blitted
  GameRestoreTile(prefs->game.loderunner.playerPosition.x,
                  prefs->game.loderunner.playerPosition.y,
                  globals->winPlayerBackup);

  //
  // DRAW OTHER INFORMATION ON SCREEN
  //

  rect.topLeft.x    = 0;  // will change
  rect.topLeft.y    = 0;
  rect.extent.x     = TILE_WIDTH;
  rect.extent.y     = TILE_HEIGHT;
  scrRect.topLeft.x = 0;  // will change
  scrRect.topLeft.y = 0;  // will change
  scrRect.extent.x  = TILE_WIDTH;
  scrRect.extent.y  = TILE_HEIGHT;

  // draw the level
  {
    Char levelStr[8];

    if (prefs->game.loderunner.stageNumber <  10) StrCopy(levelStr, "00"); else
    if (prefs->game.loderunner.stageNumber < 100) StrCopy(levelStr, "0");  else
                                                  StrCopy(levelStr, "");

    StrPrintF(displayStr, "%s%d", 
              levelStr, prefs->game.loderunner.stageNumber);

    FntSetFont(boldFont);
    WinDrawChars(displayStr, StrLen(displayStr), 32, 148);
    FntSetFont(currFont);
  }

  // draw the lives
  {
    Char livesStr[8];

    if (prefs->game.gameLives <  10) StrCopy(livesStr, "00"); else
    if (prefs->game.gameLives < 100) StrCopy(livesStr, "0");  else
                                     StrCopy(livesStr, "");

    // put the player there
    scrRect.topLeft.x = 92;
    scrRect.topLeft.y = 149;
    rect.topLeft.x    = toolPlayer * (rect.extent.x + 2);
    WinCopyRectangle(globals->winTools, WinGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);

    StrPrintF(displayStr, "%s%d", 
              livesStr, prefs->game.gameLives);

    FntSetFont(boldFont);
    WinDrawChars(displayStr, StrLen(displayStr), 100, 148);
    FntSetFont(currFont);
  }

  // draw the score
  {
    Char scoreStr[8];

    if (prefs->game.gameScore <      10) StrCopy(scoreStr, "00000"); else
    if (prefs->game.gameScore <     100) StrCopy(scoreStr, "0000");  else
    if (prefs->game.gameScore <    1000) StrCopy(scoreStr, "000");   else
    if (prefs->game.gameScore <   10000) StrCopy(scoreStr, "00");    else
    if (prefs->game.gameScore <  100000) StrCopy(scoreStr, "0");     else
                                         StrCopy(scoreStr, "");

    StrPrintF(displayStr, "%s%d", scoreStr, prefs->game.gameScore);

    FntSetFont(boldFont);
    WinDrawChars(displayStr, StrLen(displayStr), 124, 148);
    FntSetFont(currFont);
  }

  FntSetFont(currFont);
}

/**
 * Terminate the Game.
 */
void   
GameTerminate()
{
  GameGlobals *globals;
  Int16       i;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // return the state of key processing
  KeySetMask(globals->keyMask);

  // unload the gamepad driver (if available)
  if (globals->hardware.gamePadPresent) {

    Err    err;
    UInt32 gamePadUserCount;

    err = GPDClose(globals->hardware.gamePadLibRef, &gamePadUserCount);
    if (gamePadUserCount == 0)
      SysLibRemove(globals->hardware.gamePadLibRef);
  }

  // close the database
  EditorTerminate();

  // clean up windows/memory
  if (globals->winTools != NULL) 
    WinDeleteWindow(globals->winTools,             false);
  if (globals->winHoles != NULL) 
  WinDeleteWindow(globals->winHoles,             false);
  if (globals->winPlayers != NULL) 
  WinDeleteWindow(globals->winPlayers,           false);
  if (globals->winPlayersMask != NULL) 
  WinDeleteWindow(globals->winPlayersMask,       false);
  if (globals->winPlayerBackup != NULL) 
  WinDeleteWindow(globals->winPlayerBackup,      false);
  if (globals->winMonks != NULL) 
  WinDeleteWindow(globals->winMonks,             false);
  if (globals->winMonksMask != NULL) 
  WinDeleteWindow(globals->winMonksMask,         false);
  for (i=0; i<MAX_MONKS; i++) {
    if (globals->winMonkBackup[i] != NULL) 
      WinDeleteWindow(globals->winMonkBackup[i],   false);
  }
  MemPtrFree(globals);

  // unregister global data
  FtrUnregister(appCreator, ftrGameGlobals);
}

/**
 * Adjust the level (remove birds that are too close and reset positions)
 *
 * @param prefs the global preference data.
 */
static void
GameAdjustLevel(PreferencesType *prefs)
{
  GameGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  //
  // load the level
  //

  // load the level into gameplay
  EditorLoadLevel(&prefs->game.loderunner.stage, 
                  prefs->game.loderunner.stageNumber);

  // reset loderunner specific things
  prefs->game.loderunner.playerDir = moveNone;
  MemSet(&prefs->game.loderunner.playerTarget,sizeof(PositionType),0);
  MemSet(&prefs->game.loderunner.playerPosition,sizeof(PositionType),0);
  prefs->game.loderunner.playerRecCount = 0;

  prefs->game.loderunner.monkCount      = 0;
  MemSet(prefs->game.loderunner.monkDir, sizeof(UInt16)*MAX_MONKS,0);
  MemSet(prefs->game.loderunner.monkWait, sizeof(UInt16)*MAX_MONKS,0);
  MemSet(prefs->game.loderunner.monkTarget, sizeof(PositionType)*MAX_MONKS,0);
  MemSet(prefs->game.loderunner.monkPosition, sizeof(PositionType)*MAX_MONKS,0);
  MemSet(prefs->game.loderunner.monkRecCount, sizeof(UInt16)*MAX_MONKS,0);
  MemSet(prefs->game.loderunner.monkHasGold, sizeof(Boolean)*MAX_MONKS,0);

  // identify the "accessable" tiles (for walking)
  {
    Int16  i, j;
    UInt16 mCount, rCount;

    // reset our "counters"
    mCount = 0;
    rCount = 0;

    // go through and adjust everything
    for (j=0; j<GRID_HEIGHT; j++) {
      for (i=0; i<GRID_WIDTH; i++) {

        switch (prefs->game.loderunner.stage.tiles[i][j].tool) 
        {
          case toolPlayer: 
               prefs->game.loderunner.playerPosition.x = i * TILE_WIDTH;
               prefs->game.loderunner.playerPosition.y = j * TILE_HEIGHT;
               prefs->game.loderunner.playerTarget.x   = i;
               prefs->game.loderunner.playerTarget.y   = j;
               prefs->game.loderunner.playerRecCount   = 0;
               prefs->game.loderunner.playerDir        = moveNone;

               prefs->game.loderunner.stage.tileState[i][j] = 0;
               break;

          case toolMonk: 
               prefs->game.loderunner.monkPosition[mCount].x = i * TILE_WIDTH;
               prefs->game.loderunner.monkPosition[mCount].y = j * TILE_HEIGHT;
               prefs->game.loderunner.monkTarget[mCount].x   = i;
               prefs->game.loderunner.monkTarget[mCount].y   = j;
               prefs->game.loderunner.monkDir[mCount]        = moveNone;
               prefs->game.loderunner.monkRecCount[mCount]   = 0;
               if (mCount < MAX_MONKS) mCount++;

               prefs->game.loderunner.stage.tileState[i][j] = 0;
               break;

          case toolEmpty: 
          case toolGold: 
          case toolRope:
          case toolFallThru:
          case toolEndLadder:
               prefs->game.loderunner.stage.tileState[i][j] = 0;
               break;

          case toolBrick:
          case toolRock:
               prefs->game.loderunner.stage.tileState[i][j] = 255;
               break;

          default:
               break;
        }
      }
    }

    // generate a list of "recovery" positions
    {
      for (i=0; i<GRID_WIDTH; i++) {
        for (j=0; j<GRID_HEIGHT; j++) {
          if (prefs->game.loderunner.stage.tileState[i][j] != 255) {
            prefs->game.loderunner.stage.recoveryPosition[rCount].x = i;
            prefs->game.loderunner.stage.recoveryPosition[rCount].y = j;
            if (rCount < MAX_SETS) rCount++;

            break; // out of loop
          }
        }
      }
    }

    prefs->game.loderunner.monkCount           = mCount;
    prefs->game.loderunner.stage.recoveryCount = rCount;
  }

  // reset the "backup" and "onscreen" flags
  prefs->game.loderunner.goldCollected  = false;
  globals->goldCollected                = prefs->game.loderunner.goldCollected;
  globals->playerDied                   = false;

  // ok, wait for some input before we start playing
  prefs->game.gameWait                  = true;
}

/**
 * Move the player.
 *
 * @param prefs the global preference data.
 */
static void
GameMovePlayer(PreferencesType *prefs)
{
  GameGlobals    *globals;
  SndCommandType collectSnd  = {sndCmdFreqDurationAmp,0,1024,32,sndMaxAmp};
  SndCommandType fallingSnd  = {sndCmdFreqDurationAmp,0,4096,2,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  {
    Coord x, y;

    // whats the players current position?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    //
    // adjust anything we need
    //

    // reborn?
    if (prefs->game.loderunner.playerRecCount != 0) 
      prefs->game.loderunner.playerRecCount--;

    //
    // determine the players next move
    //

    // @ a tile position - and can move :)
    if (
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        (prefs->game.loderunner.playerRecCount == 0)     // reborn?
       ) {

      // nothing solid to stand on? gravity rules
      if (
          (y < (GRID_HEIGHT-1)) &&
          (prefs->game.loderunner.stage.tileState[x][y+1] != 255) &&
          (prefs->game.loderunner.stage.tiles[x][y+1].tool != toolLadder) &&
          (prefs->game.loderunner.stage.tiles[x][y].tool   != toolLadder) &&
          (prefs->game.loderunner.stage.tiles[x][y].tool   != toolRope) &&
          (!GameMonkBelowPlayer(prefs))
         ) {

        // keep falling (sucker)
        prefs->game.loderunner.playerDir = moveDown;
      }
      else

      // case #1 - new pos same level - just move
      if (y == prefs->game.loderunner.playerTarget.y) {

        // priority: horizontal
        if (!GameMovePlayerHorizontally(prefs))
          prefs->game.loderunner.playerDir = moveNone;
      }
      else

      // case #2 - new pos above current level
      if (y > prefs->game.loderunner.playerTarget.y) {

        // priority: vertical, horizontal
        if (!GameMovePlayerVerticlly(prefs)) 
          if (!GameMovePlayerHorizontally(prefs))
            prefs->game.loderunner.playerDir = moveNone;
      }
      else

      // case #3 - new pos above current level
      if (y < prefs->game.loderunner.playerTarget.y) {

        // priority: task, horizontal, task
        if (!GameMovePlayerTask(prefs)) 
          if (!GameMovePlayerHorizontally(prefs))
            if (!GameMovePlayerVerticlly(prefs))
              prefs->game.loderunner.playerDir = moveNone;
      }
    }

    //
    // update the players position (if possible)
    //

    switch (prefs->game.loderunner.playerDir)
    {
      case moveLeft:

           // move!
           prefs->game.loderunner.playerPosition.x -= 2;

           break;

      case moveRight:

           // move!
           prefs->game.loderunner.playerPosition.x += 2;

           break;

      case moveUp:

           // move!
           prefs->game.loderunner.playerPosition.y -= 2;

           break;

      case moveDown:

           // move!
           prefs->game.loderunner.playerPosition.y += 2;

           // do we need to play the falling sound?
           if ((prefs->game.loderunner.stage.tiles[x][y].tool   != toolLadder) &&
               (prefs->game.loderunner.stage.tiles[x][y+1].tool != toolLadder))
             DevicePlaySound(&fallingSnd);

           break;

      case digLeft:

           // @ a tile position
           if (GameTilePosition(&prefs->game.loderunner.playerPosition)) {

             // dig
             prefs->game.loderunner.stage.tileState[x-1][y+1] = holeFirst;

             // stay put
             prefs->game.loderunner.playerDir = moveNone;
             prefs->game.loderunner.playerTarget.x = x;
             prefs->game.loderunner.playerTarget.y = y;
           }

           break;

      case digRight:

           // @ a tile position
           if (GameTilePosition(&prefs->game.loderunner.playerPosition)) {

             // dig!
             prefs->game.loderunner.stage.tileState[x+1][y+1] = holeFirst;

             // stay put
             prefs->game.loderunner.playerDir = moveNone;
             prefs->game.loderunner.playerTarget.x = x;
             prefs->game.loderunner.playerTarget.y = y;
           }
           break;

      case moveNone:
      default:
           break;
    }

    //
    // lets process the new movement (pickups etc)
    //

    // whats the players new position (if any)?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // has the player picked up some $$$$?
    if ((prefs->game.loderunner.stage.tiles[x][y].tool == toolGold) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition) &&
        ((prefs->game.loderunner.playerDir == moveDown) ||
	 (prefs->game.loderunner.playerDir == moveUp) ||
         (y == (GRID_HEIGHT-1)) ||
         (GameMonkBelowPlayer(prefs)) ||
         (prefs->game.loderunner.stage.tileState[x][y+1] == 255) ||
         (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder))) {

      // one point!
      prefs->game.gameScore++;
      DevicePlaySound(&collectSnd);

      // remove the gold from the level (and background)
      prefs->game.loderunner.stage.tiles[x][y].tool       = toolEmpty;
      prefs->game.loderunner.stage.tiles[x][y].properties = 0;
      GameDrawTile(prefs->game.loderunner.stage.tiles[x][y], x, y);

      // has *all* the gold been collected?
      GameGoldCheck(prefs);
    }
    else

    // has the player got to the top of screen and finished?
    if (globals->goldCollected && (y == 0) &&
        GameTilePosition(&prefs->game.loderunner.playerPosition)) {

      Int16          i;
      SndCommandType snd = {sndCmdFreqDurationAmp,0,0,5,sndMaxAmp};

      // play a little zap! :)
      for (i=0; i<15; i++) {
        snd.param1 += 256 + (1 << i);  // frequency
        DevicePlaySound(&snd);

        SysTaskDelay(2); // small deley
      }

      // prepare for the next stage
      prefs->game.gameScore += 100;
      prefs->game.gameLives++;

      // have we run out of levels? :)
      if (prefs->game.loderunner.stageNumber == EditorLevelCount()) {
  
        EventType event;

        // are we playing the "ORIGINAL" set - without cheating :P
        if (GameHighScoreCandidate(prefs)) {

          // for every remaining life, add 250 points
          prefs->game.gameScore += (250 * prefs->game.gameLives);
          prefs->game.loderunner.stageNumber = 255;
        }

        // return to main screen
        MemSet(&event, sizeof(EventType), 0);
        event.eType            = menuEvent;
        event.data.menu.itemID = gameMenuItemExit;
        EvtAddEventToQueue(&event);
      }

      // continue playing
      else {

        // demo's up? :)
        prefs->game.loderunner.stageNumber++;

#ifdef PROTECTION_ON
	if (!RegisterDemoExpired(prefs, globals->startLevel)) {
#endif
          GameAdjustLevel(prefs);

          // lets draw the level and continue!
          GameDrawLevel(prefs);
#ifdef PROTECTION_ON
        }
#endif
      }
    }
  }
}

/**
 * Move the player - perform a task (dig etc)?
 *
 * @param prefs the global preference data.
 * @return true if we have determined what to do, false otherwise.
 */
static Boolean 
GameMovePlayerTask(PreferencesType *prefs)
{
  Boolean result = false;

  {
    Coord x, y;

    // whats the players current position?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // lets see if we at least qualify for a command
    if ((y < (GRID_WIDTH-1)) &&
        (y == (prefs->game.loderunner.playerTarget.y-1))) {

      // dig a hole on the left?
      if (x == (prefs->game.loderunner.playerTarget.x+1)) {

        // ok, lets do all the checks
        if (
            // its a brick?
            (prefs->game.loderunner.stage.tiles[x-1][y+1].tool == toolBrick) &&
            (prefs->game.loderunner.stage.tileState[x-1][y+1] == 255) &&

            // ensure we can actually get to the next position (and we can dig)
            (prefs->game.loderunner.stage.tileState[x-1][y] != 255) &&
            (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolRope) &&
            (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolLadder) &&
            (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolFallThru) 
           ) {

          // ok, rules are ok to dig on left
          prefs->game.loderunner.playerDir = digLeft;
          result = true; 
        }
        else 

        // we dont want to start falling into our newly dug hole?
        if ((prefs->game.loderunner.stage.tiles[x-1][y+1].tool == toolBrick) &&
            (prefs->game.loderunner.stage.tileState[x-1][y+1] != 255)) {

          // stay put!
          prefs->game.loderunner.playerDir = moveNone;
          prefs->game.loderunner.playerTarget.x = x;
          prefs->game.loderunner.playerTarget.y = y;
          result = true; 
        }
      }
      else

      // dig a hole on the right?
      if (x == (prefs->game.loderunner.playerTarget.x-1)) {

        // ok, lets do all the checks
        if (
            // its a brick?
            (prefs->game.loderunner.stage.tiles[x+1][y+1].tool == toolBrick) &&
            (prefs->game.loderunner.stage.tileState[x+1][y+1] == 255) &&

            // ensure we can actually get to the next position (and we can dig)
            (prefs->game.loderunner.stage.tileState[x+1][y] != 255) &&
            (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolLadder) &&
            (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolRope) &&
            (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolFallThru) 
           ) {

          // ok, rules are ok to dig on right
          prefs->game.loderunner.playerDir = digRight;
          result = true;
        }
        else 

        // we dont want to start falling into our newly dug hole?
        if ((prefs->game.loderunner.stage.tiles[x+1][y+1].tool == toolBrick) &&
            (prefs->game.loderunner.stage.tileState[x+1][y+1] != 255)) {

          // stay put!
          prefs->game.loderunner.playerDir = moveNone;
          prefs->game.loderunner.playerTarget.x = x;
          prefs->game.loderunner.playerTarget.y = y;
          result = true; 
        }
      }
    }
  }

  return result;
}

/**
 * Move the player - move horizontally
 *
 * @param prefs the global preference data.
 * @return true if we have determined what to do, false otherwise.
 */
static Boolean 
GameMovePlayerHorizontally(PreferencesType *prefs)
{
  Boolean result = false;

  {
    Coord x, y;

    // whats the players current position?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // destination is somewhere on the left?
    if (x > prefs->game.loderunner.playerTarget.x) {

      // ok, can we do it?
      if (
          (((prefs->game.loderunner.stage.tileState[x-1][y] != 255) &&
            (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolFallThru)) 
	  ) &&
          (
           (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder) || 
           (prefs->game.loderunner.stage.tiles[x][y].tool == toolRope) ||
           (y == (GRID_HEIGHT-1)) ||
           (
            (y < (GRID_HEIGHT-1)) &&
            (
             (prefs->game.loderunner.stage.tileState[x][y+1] == 255) ||
             (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder) ||
             (GameMonkBelowPlayer(prefs)) 
            )
           )
          )) {

        // ok, rules are ok to move left
        prefs->game.loderunner.playerDir = moveLeft;

        result = true;
      }
    }

    // destination is somewhere on the right?
    else
    if (x < prefs->game.loderunner.playerTarget.x) {

      // ok, can we do it?
      if (
          (((prefs->game.loderunner.stage.tileState[x+1][y] != 255) &&
            (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolFallThru)) 
	  ) &&
          (
           (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder) || 
           (prefs->game.loderunner.stage.tiles[x][y].tool == toolRope) ||
           (y == (GRID_HEIGHT-1)) ||
           (
            (y < (GRID_HEIGHT-1)) &&
            (
             (prefs->game.loderunner.stage.tileState[x][y+1] == 255) ||
             (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder) ||
             (GameMonkBelowPlayer(prefs)) 
            )
           )
          )) {

        // ok, rules are ok to move right
        prefs->game.loderunner.playerDir = moveRight;

        result = true;
      }
    }
  }

  return result;
}

/**
 * Move the player - move vertically
 *
 * @param prefs the global preference data.
 * @return true if we have determined what to do, false otherwise.
 */
static Boolean 
GameMovePlayerVerticlly(PreferencesType *prefs)
{
  Boolean result = false;

  {
    Coord x, y;

    // whats the players current position?
    x = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    y = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // destination is somewhere below the player?
    if (y < prefs->game.loderunner.playerTarget.y) {

      // ok, can we do it?
      if ((y < (GRID_HEIGHT-1)) &&
          (
           (((prefs->game.loderunner.stage.tiles[x][y].tool == toolRope) ||
             (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder)) &&
            ((prefs->game.loderunner.stage.tileState[x][y+1] != 255))
           ) ||
           (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder))) {

        // ok, rules are ok to move down
        prefs->game.loderunner.playerDir = moveDown;

        result = true;
      }
    }

    // destination is somewhere above the player?
    else
    if (y > prefs->game.loderunner.playerTarget.y) {

      // ok, can we do it?
      if ((y > 0) &&
          (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder) &&
          (
	   ((prefs->game.loderunner.stage.tileState[x][y-1] != 255) &&
            (prefs->game.loderunner.stage.tiles[x][y-1].tool != toolFallThru)
	   ))) {

        // ok, rules are ok to move up
        prefs->game.loderunner.playerDir = moveUp;

        result = true;
      }
    }
  }

  return result;
}

/**
 * Move the monks.
 *
 * @param prefs the global preference data.
 */
static void
GameMoveMonks(PreferencesType *prefs)
{
  GameGlobals *globals;
  SndCommandType trappedSnd = {sndCmdFreqDurationAmp,0,256,32,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // only do this if the player is still alive
  if (!globals->playerDied) {

    Int16   i;
    Coord   x,  y;
    Coord   px, py;
    Boolean justReleased;

    // whats the players current position?
    px = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    py = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    for (i=0; i<prefs->game.loderunner.monkCount; i++) {

      // identify the current position
      x = prefs->game.loderunner.monkPosition[i].x / TILE_WIDTH;
      y = prefs->game.loderunner.monkPosition[i].y / TILE_HEIGHT;

      //
      // adjust anything we need
      //

      // reborn?
      if (prefs->game.loderunner.monkRecCount[i] != 0)
        prefs->game.loderunner.monkRecCount[i]--;

      // trapped?
      justReleased = false;
      if (prefs->game.loderunner.monkWait[i] != 0) {

        // decrement counter
        prefs->game.loderunner.monkWait[i]--;

        // release time? :)
        if (prefs->game.loderunner.monkWait[i] == 0) {

          // we can release ONLY if there is no monk above
          if ((!GameMonkStationaryAt(prefs, x, y-1)) &&
              (prefs->game.loderunner.stage.tileState[x][y-1] != 255)) {

            // spring them out of the hole
            prefs->game.loderunner.monkPosition[i].y -= TILE_HEIGHT;
            prefs->game.loderunner.monkDir[i]         = moveNone;
            prefs->game.loderunner.monkTarget[i].x    = x;
            prefs->game.loderunner.monkTarget[i].y    = --y;

            // we dont want to just fall back in now do we?
            justReleased = true;
          }

          // cannot get out? stuck there
          else 
            prefs->game.loderunner.monkWait[i] = holeDelay >> 1;
        }
      }

      //
      // chase the player
      //

      // @ a tile position - and can move :)
      if (
          GameTilePosition(&prefs->game.loderunner.monkPosition[i]) &&
          (prefs->game.loderunner.monkWait[i]     == 0) &&  // trapped?
          (prefs->game.loderunner.monkRecCount[i] == 0)     // reborn?
         ) {

        // nothing solid to stand on? gravity rules
        if (
            (!justReleased) &&
            (y < (GRID_HEIGHT-1)) &&
            (prefs->game.loderunner.stage.tileState[x][y+1] != 255) &&
            (prefs->game.loderunner.stage.tiles[x][y+1].tool != toolLadder) &&
            (prefs->game.loderunner.stage.tiles[x][y].tool   != toolLadder) &&
            (prefs->game.loderunner.stage.tiles[x][y].tool   != toolRope) &&
            (!GameMonkStationaryAt(prefs, x, y+1))
           ) {

          // keep falling (sucker)
          prefs->game.loderunner.monkDir[i] = moveDown;
          prefs->game.loderunner.monkTarget[i].x = x;
          prefs->game.loderunner.monkTarget[i].y = y+1;
        }
        else

        // player is on the *same* level
        if (y == py) {

          // player is on the right?
          if (x < px) {

            // can we go there?
            if ((prefs->game.loderunner.stage.tileState[x+1][y] != 255) &&
	        (prefs->game.loderunner.stage.tiles[x+1][y].tool != toolFallThru)) {
              prefs->game.loderunner.monkDir[i] = moveRight;
              prefs->game.loderunner.monkTarget[i].x = x+1;
              prefs->game.loderunner.monkTarget[i].y = y;
            }

            // no good, stay put
            else 
              prefs->game.loderunner.monkDir[i] = moveNone;
          }
          else 

          // player is on the left?
          if (x > px) {

            // can we go there?
            if ((prefs->game.loderunner.stage.tileState[x-1][y] != 255) &&
	        (prefs->game.loderunner.stage.tiles[x-1][y].tool != toolFallThru)) {
              prefs->game.loderunner.monkDir[i] = moveLeft;
              prefs->game.loderunner.monkTarget[i].x = x-1;
              prefs->game.loderunner.monkTarget[i].y = y;
            }

            // no good, stay put
            else 
              prefs->game.loderunner.monkDir[i] = moveNone;
          }

          // um.. we are on player? :) [debugging only]
          else 
            prefs->game.loderunner.monkDir[i] = moveNone;
        }
        else 

        // player is below monk
        if (y < py) {

          // can we move down somehow?
          if (
              (
               (prefs->game.loderunner.stage.tiles[x][y].tool == toolRope) ||
               (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder) 
              ) &&
              (prefs->game.loderunner.stage.tileState[x][y+1] != 255) &&
              (!GameMonkStationaryAt(prefs, x, y+1))
             ) {

            // fall down :)
            prefs->game.loderunner.monkDir[i] = moveDown;
            prefs->game.loderunner.monkTarget[i].x = x;
            prefs->game.loderunner.monkTarget[i].y = y+1;
          }
          else

          // can we get on a ladder?
          if (
              (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder) 
             ) {

            // climb down :)
            prefs->game.loderunner.monkDir[i] = moveDown;
            prefs->game.loderunner.monkTarget[i].x = x;
            prefs->game.loderunner.monkTarget[i].y = y+1;
          }

          // look for something to fall down - catch him!
          else {

            Boolean processed = false;
            Boolean notRight  = false;
            Boolean notLeft   = false;
            Int16   dx        = 0;

            // look for a space to climb/fall down
            while (!processed ) {

              dx++;

              // something blocking the right?
              if ((!notRight) &&
                  (
                   ((int)(x+dx) >= GRID_WIDTH) ||
                   (prefs->game.loderunner.stage.tileState[x+dx][y] == 255) ||
                   ((prefs->game.loderunner.stage.tileState[x+dx][y] != 255) &&
		    (prefs->game.loderunner.stage.tiles[x+dx][y].tool == toolFallThru))
                  )) {
                notRight = true;
              }
                
              // something blocking the left?
              if ((!notLeft) &&
                  (
                   ((int)(x-dx) < 0) ||
                   (prefs->game.loderunner.stage.tileState[x-dx][y] == 255) ||
                   ((prefs->game.loderunner.stage.tileState[x-dx][y] != 255) &&
		    (prefs->game.loderunner.stage.tiles[x-dx][y].tool == toolFallThru))
                  )) {
                notLeft = true;
              }

              // is there a free space on right to fall into? (including ladder)
              if ((!notRight) &&
                  (
                   (prefs->game.loderunner.stage.tileState[x+dx][y+1] != 255) &&
                   (!GameMonkStationaryAt(prefs, x-dx, y+1))
                  )
                 ) {

                // go for it
                prefs->game.loderunner.monkDir[i] = moveRight;
                prefs->game.loderunner.monkTarget[i].x = x+1;
                prefs->game.loderunner.monkTarget[i].y = y;

                processed = true;
              }
              else 

              // is there a free space on left to fall into? (including ladder)
              if ((!notLeft) &&
                  (
                   (prefs->game.loderunner.stage.tileState[x-dx][y+1] != 255) &&
                   (!GameMonkStationaryAt(prefs, x-dx, y+1))
                  )
                 ) {

                // go for it
                prefs->game.loderunner.monkDir[i] = moveLeft;
                prefs->game.loderunner.monkTarget[i].x = x-1;
                prefs->game.loderunner.monkTarget[i].y = y;

                processed = true;
              }
              else

              // impossible mission?
              if (notLeft && notRight) { 

                prefs->game.loderunner.monkDir[i] = moveNone;
                processed = true;
              }
            }
          }
        }
        else

        // player is above monk
        if (y > py) {

          // are we on a ladder?
          if (
              (y > 0) &&
              (prefs->game.loderunner.stage.tiles[x][y].tool == toolLadder) &&
              (prefs->game.loderunner.stage.tileState[x][y-1] != 255) &&
              (prefs->game.loderunner.stage.tiles[x][y-1].tool != toolFallThru)
             ) {

            // climb :)
            prefs->game.loderunner.monkDir[i] = moveUp;
            prefs->game.loderunner.monkTarget[i].x = x;
            prefs->game.loderunner.monkTarget[i].y = y-1;
          }

          // look for the ladder damnit - catch him!
          else {

            Int16   dx;
            Boolean processed = false;
            Boolean notRight  = false;
            Boolean notLeft   = false;

            // look for a ladder to climb up
            dx = 0;
            while (!processed ) {

              dx++;

              // something blocking the right?
              if ((!notRight) &&
                  (
                   ((int)(x+dx) >= GRID_WIDTH) ||
                   (prefs->game.loderunner.stage.tileState[x+dx][y] == 255) ||
                   (prefs->game.loderunner.stage.tiles[x+dx][y].tool == toolFallThru) ||
                   (
                    (y < (GRID_HEIGHT-1)) &&
                    (prefs->game.loderunner.stage.tileState[x+dx][y+1] != 255) &&
                    (prefs->game.loderunner.stage.tiles[x+dx][y+1].tool != toolLadder) &&
                    (prefs->game.loderunner.stage.tiles[x+dx][y+1].tool != toolBrick) &&
                    (prefs->game.loderunner.stage.tiles[x+dx][y].tool != toolLadder) &&
                    (prefs->game.loderunner.stage.tiles[x+dx][y].tool != toolRope) &&
                    (!GameMonkStationaryAt(prefs, x+dx, y+1))
                   ))) {

                notRight = true;
              }
                
              // something blocking the left?
              if ((!notLeft) &&
                  (
                   ((int)(x-dx) < 0) ||
                   (prefs->game.loderunner.stage.tileState[x-dx][y] == 255) ||
                   (prefs->game.loderunner.stage.tiles[x-dx][y].tool == toolFallThru) ||
                   (
                    (y < (GRID_HEIGHT-1)) &&
                    (prefs->game.loderunner.stage.tileState[x-dx][y+1] != 255) &&
                    (prefs->game.loderunner.stage.tiles[x-dx][y+1].tool != toolLadder) &&
                    (prefs->game.loderunner.stage.tiles[x-dx][y+1].tool != toolBrick) &&
                    (prefs->game.loderunner.stage.tiles[x-dx][y].tool != toolLadder) &&
                    (prefs->game.loderunner.stage.tiles[x-dx][y].tool != toolRope) &&
                    (!GameMonkStationaryAt(prefs, x-dx, y+1))
                   ))) {

                notLeft = true;
              }

              // is there a ladder to the right?
              if ((!notRight) &&
                  (prefs->game.loderunner.stage.tiles[x+dx][y].tool == toolLadder) &&
                  (
                   (y > 0) &&
                   (prefs->game.loderunner.stage.tileState[x+dx][y-1] != 255)
                  )) {

                // go for it
                prefs->game.loderunner.monkDir[i] = moveRight;
                prefs->game.loderunner.monkTarget[i].x = x+1;
                prefs->game.loderunner.monkTarget[i].y = y;

                processed = true;
              }
              else 

              // is there a ladder to the left?
              if ((!notLeft) &&
                  (prefs->game.loderunner.stage.tiles[x-dx][y].tool == toolLadder) &&
                  (
                   (y > 0) &&
                   (prefs->game.loderunner.stage.tileState[x-dx][y-1] != 255)
                  )) {

                // go for it
                prefs->game.loderunner.monkDir[i] = moveLeft;
                prefs->game.loderunner.monkTarget[i].x = x-1;
                prefs->game.loderunner.monkTarget[i].y = y;

                processed = true;
              }
              else

              // impossible mission?
              if (notLeft && notRight) { 

                prefs->game.loderunner.monkDir[i] = moveNone;
                processed = true;
              }
            }
          }
        }

        // lets make sure the move is valid :)
        if (GameMonkInPath(prefs, i)) {
          prefs->game.loderunner.monkDir[i] = moveNone;
          prefs->game.loderunner.monkTarget[i].x = x;
          prefs->game.loderunner.monkTarget[i].y = y;
        }
      }

      //
      // update the monks position (if possible)
      //

      switch (prefs->game.loderunner.monkDir[i])
      {
        case moveLeft:

             // move!
             prefs->game.loderunner.monkPosition[i].x--;

             break;

        case moveRight:

             // move!
             prefs->game.loderunner.monkPosition[i].x++;

             break;

        case moveUp:

             // move!
             prefs->game.loderunner.monkPosition[i].y--;

             break;
  
        case moveDown:

             // lets see if we have just fallen into a hole :)
             if (GameTilePosition(&prefs->game.loderunner.monkPosition[i]) &&
                 (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolBrick) &&
                 (prefs->game.loderunner.stage.tileState[x][y+1] != 255)) {

               // if the hole is not complete, close it 
               if (prefs->game.loderunner.stage.tileState[x][y+1] < (holeFirst+5)) {
                 // close it, and draw the original tile back
                 prefs->game.loderunner.stage.tileState[x][y+1] = 255;
                 GameDrawTile(prefs->game.loderunner.stage.tiles[x][y+1], x, y+1);

                 // leave the monk, he will chase next move :)
                 prefs->game.loderunner.monkDir[i]         = moveNone;
                 prefs->game.loderunner.monkTarget[i].x    = x;
                 prefs->game.loderunner.monkTarget[i].y    = y;
               }
               else {

                 // force them into the hole
                 prefs->game.loderunner.monkPosition[i].y += TILE_HEIGHT;
                 prefs->game.loderunner.monkDir[i]         = moveNone;
                 prefs->game.loderunner.monkTarget[i].x    = x;
                 prefs->game.loderunner.monkTarget[i].y    = y+1;
                 DevicePlaySound(&trappedSnd);

                 // make them wait 1/2 the time it takes for a hole to close
                 prefs->game.loderunner.monkWait[i] = holeDelay >> 1;

                 // were they carrying gold?
                 if (prefs->game.loderunner.monkHasGold[i]) {

                   // take it off the monk
                   prefs->game.loderunner.monkHasGold[i] = false;

                   // draw the gold back in the level
                   prefs->game.loderunner.stage.tiles[x][y].tool = toolGold;
                   prefs->game.loderunner.stage.tiles[x][y].properties = 0;
                   prefs->game.loderunner.stage.tileState[x][y]        = 0;
                   GameDrawTile(prefs->game.loderunner.stage.tiles[x][y], x, y);
                 }
               }
             }

             // nothing special, just move!
             else 
               prefs->game.loderunner.monkPosition[i].y++;

             break;

        case moveNone:
        default:
             break;
      }

      //
      // has the monk caught up with our "player"?
      //

      x = prefs->game.loderunner.monkPosition[i].x;
      y = prefs->game.loderunner.monkPosition[i].y;

      globals->playerDied |= 
       ((ABS(prefs->game.loderunner.playerPosition.x-x) < (TILE_WIDTH >> 1)) &&
        (ABS(prefs->game.loderunner.playerPosition.y-y) < (TILE_HEIGHT >> 1)));

      //
      // process the new position
      //

      x = prefs->game.loderunner.monkPosition[i].x / TILE_WIDTH;
      y = prefs->game.loderunner.monkPosition[i].y / TILE_HEIGHT;

      // has the monk picked up some $$$$?
      if ((prefs->game.loderunner.stage.tiles[x][y].tool == toolGold) &&
          (!prefs->game.loderunner.monkHasGold[i]) &&
          GameTilePosition(&prefs->game.loderunner.monkPosition[i])) {

        UInt16 chance = SysRandom(0) % 10;

        // 40% chance of picking it up
        if (chance < 4) {

          // now the monk has the gold!
          prefs->game.loderunner.monkHasGold[i] = true;

          // remove the gold from the level (and background)
          prefs->game.loderunner.stage.tiles[x][y].tool       = toolEmpty;
          prefs->game.loderunner.stage.tiles[x][y].properties = 0;
          GameDrawTile(prefs->game.loderunner.stage.tiles[x][y], x, y);
        }
      }
      else

      // if the monk has gold, does he feel like dropping it? :)
      if ((prefs->game.loderunner.stage.tiles[x][y].tool == toolEmpty) &&
          (prefs->game.loderunner.monkHasGold[i]) &&
	  ((y == (GRID_WIDTH-1)) || 
           (prefs->game.loderunner.stage.tileState[x][y+1] == 255) ||
           (prefs->game.loderunner.stage.tiles[x][y+1].tool == toolLadder)) &&
          GameTilePosition(&prefs->game.loderunner.monkPosition[i])) {

        UInt16 chance = SysRandom(0) % 10;

        // 10% chance of dropping it
        if (chance == 0) {

          // now the monk no longer has the gold!
          prefs->game.loderunner.monkHasGold[i] = false;

          // remove the gold from the level (and background)
          prefs->game.loderunner.stage.tiles[x][y].tool       = toolGold;
          prefs->game.loderunner.stage.tiles[x][y].properties = 0;
          GameDrawTile(prefs->game.loderunner.stage.tiles[x][y], x, y);
        }
      }
    }
  }
}

/**
 * Is there another monk in the position we intend to move into?
 *
 * @param prefs the global preference data.
 * @param index the index of the monk we are comparing with.
 * @return true if monk is in path, false otherwise.
 */
static Boolean
GameMonkInPath(PreferencesType *prefs,
               UInt16          index)
{
  Boolean result = false;
  Int16   i;
  UInt16  dir;
  Coord   x, y;

  // since we may be moving, lets use the "center" point :)
  x   = (prefs->game.loderunner.monkPosition[index].x + 3) / TILE_WIDTH;
  y   = (prefs->game.loderunner.monkPosition[index].y + 4) / TILE_HEIGHT;
  dir = prefs->game.loderunner.monkDir[index];

  // what position we look at?
  switch (prefs->game.loderunner.monkDir[index])
  {
    case moveLeft:
         x--;
         break;

    case moveRight:
         x++;
         break;

    case moveUp:
         y--;
         break;

    case moveDown:
         y++;
         break;

    case moveNone:
    default:
         break;
  }

  // scan through all
  for (i=0; i<prefs->game.loderunner.monkCount; i++) {

    result |= (
               (i != index) &&
               (prefs->game.loderunner.monkTarget[i].x == x) &&
               (prefs->game.loderunner.monkTarget[i].y == y) 
              ); 
  }
  
  return result;
}

/**
 * Is there a monk stationary at the specified position?
 *
 * @param prefs the global preference data.
 * @param x the x-coordinate in the level array (position)
 * @param y the y-coordinate in the level array (position)
 * @return true if there is a stationary monk below, false otherwise.
 */
static Boolean
GameMonkStationaryAt(PreferencesType *prefs,
                     Coord           x, 
                     Coord           y)
{
  Boolean result = false;

  // lets check if we need
  if (y < GRID_HEIGHT) {

    Int16 i;

    // scan through all
    for (i=0; i<prefs->game.loderunner.monkCount; i++) {

      result |= (
                 (prefs->game.loderunner.monkTarget[i].x == x) &&
                 (prefs->game.loderunner.monkTarget[i].y == y) &&
                 (prefs->game.loderunner.monkDir[i]      == moveNone) &&
                 (prefs->game.loderunner.monkRecCount[i] == 0)
                );
    }
  }
  
  return result;
}

/**
 * Is there a monk stationary below the player?
 *
 * @param prefs the global preference data.
 * @return true if there is a monk below, false otherwise.
 */
Boolean
GameMonkBelowPlayer(PreferencesType *prefs)
{
  Boolean result;
  Int16   i;
  Coord   x, y, mx, my;

  x = prefs->game.loderunner.playerPosition.x;
  y = prefs->game.loderunner.playerPosition.y;

  // scan through all
  result = false;
  for (i=0; i<prefs->game.loderunner.monkCount; i++) {

    mx = prefs->game.loderunner.monkPosition[i].x;
    my = prefs->game.loderunner.monkPosition[i].y;

    result |= ((ABS(mx-x) < (TILE_WIDTH >> 1)) &&
               ((my-y)    <= TILE_HEIGHT) &&
               ((my-y)    >  0));
  }

  return result;
}

/**
 * Move everything else in the game (animations etc)
 *
 * @param prefs the global preference data.
 */
static void
GameMoveOthers(PreferencesType *prefs)
{
  GameGlobals *globals;
  SndCommandType diggingSnd[] = {
                                 {sndCmdFreqDurationAmp,0,512,5,sndMaxAmp},
                                 {sndCmdFreqDurationAmp,0,544,5,sndMaxAmp},
                                 {sndCmdFreqDurationAmp,0,608,5,sndMaxAmp},
                                 {sndCmdFreqDurationAmp,0,736,5,sndMaxAmp},
                                 {sndCmdFreqDurationAmp,0,994,5,sndMaxAmp}
                                };

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // only do this if the player is still alive
  if (!globals->playerDied) {

    Int16 i, j, k;
    Coord px, py;
    Coord x, y;

    // whats the players position in this?
    px = prefs->game.loderunner.playerPosition.x / TILE_WIDTH;
    py = prefs->game.loderunner.playerPosition.y / TILE_HEIGHT;

    // the dynamic things
    for (j=0; j<GRID_HEIGHT; j++) {
      for (i=0; i<GRID_WIDTH; i++) {

        // update is required (!= 0 or 255)
        if (
            (prefs->game.loderunner.stage.tileState[i][j] != 0) &&
            (prefs->game.loderunner.stage.tileState[i][j] != 255)
           ) {

          //
          // dealing with a hole animation?
          //

          if (((prefs->game.loderunner.stage.tileState[i][j] >= holeFirst) &&
              (prefs->game.loderunner.stage.tileState[i][j] <= holeLast))) {

            // is the block about to close?
            if (prefs->game.loderunner.stage.tileState[i][j] == holeLast) {
          
              // close the hole, draw the original tile in its place :)
              prefs->game.loderunner.stage.tileState[i][j] = 254; // inc later
              GameDrawTile(prefs->game.loderunner.stage.tiles[i][j], i, j);

              // player?
              if (// standing still?
                  ((px == i) && (py == j)) ||

		  // walking into it
		  ((prefs->game.loderunner.playerTarget.x == i) &&
		   (prefs->game.loderunner.playerTarget.y == j))) {

                globals->playerDied = true;
              }

              // guards?
              for (k=0; k<prefs->game.loderunner.monkCount; k++) {

                // whats the monks position in this?
                x = prefs->game.loderunner.monkPosition[k].x / TILE_WIDTH;
                y = prefs->game.loderunner.monkPosition[k].y / TILE_HEIGHT;

                if (// standing still?
                    ((x == i) && (y == j)) ||

		    // walking into it
		    ((prefs->game.loderunner.monkTarget[k].x == i) &&
		     (prefs->game.loderunner.monkTarget[k].y == j))) {

                  UInt16 idx;
                 
                  // find a random position
                  idx = SysRandom(0) %
                        prefs->game.loderunner.stage.recoveryCount;

                  x = prefs->game.loderunner.stage.recoveryPosition[idx].x;
                  y = prefs->game.loderunner.stage.recoveryPosition[idx].y;

                  // does the monk have some gold? if so, its gone
                  if (prefs->game.loderunner.monkHasGold[k]) {
                    prefs->game.loderunner.monkHasGold[k] = false;
                    GameGoldCheck(prefs);
                  }

                  // relocate the monk!
                  prefs->game.loderunner.monkPosition[k].x = x * TILE_WIDTH;
                  prefs->game.loderunner.monkPosition[k].y = y * TILE_HEIGHT;
                  prefs->game.loderunner.monkTarget[k].x   = x;
                  prefs->game.loderunner.monkTarget[k].y   = y;
                  prefs->game.loderunner.monkDir[k]        = moveNone;
                  prefs->game.loderunner.monkRecCount[k]   = 8;
                  prefs->game.loderunner.monkWait[k]       = 0;
                }
              }
            }
            else

            // is it about to close?
            if ((prefs->game.loderunner.stage.tileState[i][j] >= holeLast-5) &&
                (prefs->game.loderunner.stage.tileState[i][j] < holeLast)) {

              GameTileType tile;

              tile.tool = tileHoleClosing;
              tile.properties = 
                holeLast - (prefs->game.loderunner.stage.tileState[i][j] + 1);
              DevicePlaySound(&diggingSnd[tile.properties]);

              // draw the tile as needed
              GameDrawTile(tile, i, j);
            }
            else

            // is it just opening?
            if ((prefs->game.loderunner.stage.tileState[i][j] < holeFirst+5) &&
                (prefs->game.loderunner.stage.tileState[i][j] >= holeFirst)) {

              GameTileType tile;

              tile.tool = tileHoleOpening;
              tile.properties = 
                prefs->game.loderunner.stage.tileState[i][j] - holeFirst;
              DevicePlaySound(&diggingSnd[tile.properties]);

              // draw the tile as needed
              GameDrawTile(tile, i, j);
            }
            else

            // it must be something else? - lets assume empty
            {
              GameTileType tile;

              tile.tool       = toolEmpty;
              tile.properties = 0;

              // draw the tile as needed
              GameDrawTile(tile, i, j);
            }

            // increment the counter...
            prefs->game.loderunner.stage.tileState[i][j]++;
          }
        }
      }
    }
  }
}

/**
 * Determine if the position supplied lies on a tile boundary.
 *
 * @param pos the position to check.
 * @return true if it is an aligned position, false otherwise.
 */
static Boolean
GameTilePosition(PositionType *pos)
{
  return ( 
          ((pos->x % TILE_WIDTH)  == 0) &&
          ((pos->y % TILE_HEIGHT) == 0)
         );
}

/**
 * Determine if there is any gold available in the level still
 *
 * @param prefs the globals preferencs structuer.
 * @return true if it is an aligned position, false otherwise.
 */
static void 
GameGoldCheck(PreferencesType *prefs)
{
  GameGlobals    *globals;
  Int16          i, j;
  Boolean        goldExists;
  SndCommandType gotAllSnd = {sndCmdFreqDurationAmp,0,2048,50,sndMaxAmp};

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  // lets look!
  goldExists = false;
  for (j=0; j<GRID_HEIGHT; j++) {
    for (i=0; i<GRID_WIDTH; i++) {
      goldExists |= 
        (prefs->game.loderunner.stage.tiles[i][j].tool == toolGold);
    }
  }
  for (i=0; i<prefs->game.loderunner.monkCount; i++) {
    goldExists |= prefs->game.loderunner.monkHasGold[i];
  }

  // no more gold? great!! exit time!
  if (!goldExists) {
         
    globals->goldCollected = true;

    // start showing everything we need to show (on exit)
    for (j=0; j<GRID_HEIGHT; j++) {
      for (i=0; i<GRID_WIDTH; i++) {
        if (prefs->game.loderunner.stage.tiles[i][j].tool == toolEndLadder) {
          prefs->game.loderunner.stage.tiles[i][j].tool = toolLadder;
          GameDrawTile(prefs->game.loderunner.stage.tiles[i][j], i, j);
        }
      }
    }

    // give some audial notion that completed
    for (i=0; i<2; i++) {

      DevicePlaySound(&gotAllSnd);
      SysTaskDelay(5); 

      GraphicsClearLCD();

      DevicePlaySound(&gotAllSnd);
      SysTaskDelay(5); 

      GraphicsRepaint();
    }
  }
  else
    globals->goldCollected = false;

  prefs->game.loderunner.goldCollected = globals->goldCollected;
}

/**
 * Draw a tile onto the backgroud buffer.
 *
 * @param tile the tile to draw
 * @param x the x-coordinate in the level array (position)
 * @param y the y-coordinate in the level array (position)
 */
static void 
GameDrawTile(GameTileType tile, 
             Coord        x,
             Coord        y)
{
  GameGlobals   *globals;
  RectangleType scrRect, rect;

  // get a globals reference
  FtrGet(appCreator, ftrGameGlobals, (UInt32 *)&globals);

  rect.topLeft.x    = 0;  // will change
  rect.topLeft.y    = 0;
  rect.extent.x     = TILE_WIDTH;
  rect.extent.y     = TILE_HEIGHT;
  scrRect.topLeft.x = x * rect.extent.x + 2;
  scrRect.topLeft.y = y * rect.extent.y;
  scrRect.extent.x  = TILE_WIDTH;
  scrRect.extent.y  = TILE_HEIGHT;

  // are we displaying a standard tool here?
  if (tile.tool <= toolLast) {

    UInt8 tool = toolEmpty;

    // which tile do we actually need to use?
    switch (tile.tool)
    {
      // leave them as they are
      case toolEmpty:
      case toolGold:
      case toolLadder:
      case toolRope:
      case toolBrick:
      case toolRock:
           tool = tile.tool;
           break;

      // always show up as a brick
      case toolFallThru:
           tool = toolBrick;
           break;

      // only show if level is complete
      case toolEndLadder:
           if (globals->goldCollected) tool = toolLadder;
           break;

      // these are NEVER shown
      case toolPlayer:
      case toolMonk:
           tool = toolEmpty;
           break;

      default:
           break;
    }    

    // update the screen
    rect.topLeft.x = tool * (rect.extent.x + 2);
    WinCopyRectangle(globals->winTools, GraphicsGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
  }

  // a overlayed animated tile
  else
  if (tile.tool <= tileLastOverlay) {

    WinHandle window     = NULL;
    WinHandle windowMask = NULL;

    // NOTE:
    //
    // since these are "moving" tiles, the x,y are not i,j in the grid :)
    // 
    // -- Aaron Ardiri, 2000

    scrRect.topLeft.x = x + 2;
    scrRect.topLeft.y = y;

    // lets check our rules
    switch (tile.tool)
    {
      case tilePlayerUpright:
      case tilePlayerRight:
      case tilePlayerLeft:
      case tilePlayerGenerating:
           rect.topLeft.x = tile.properties * rect.extent.x;
           rect.topLeft.y = (tile.tool - tilePlayerUpright) * rect.extent.y;
           window         = globals->winPlayers;
           windowMask     = globals->winPlayersMask;
           break;

      case tileMonkUpright:
      case tileMonkRight:
      case tileMonkLeft:
      case tileMonkGenerating:
           rect.topLeft.x = tile.properties * rect.extent.x;
           rect.topLeft.y = (tile.tool - tileMonkUpright) * rect.extent.y;
           window         = globals->winMonks;
           windowMask     = globals->winMonksMask;
           break;

      default:
           break;
    }

    // update the screen
    WinCopyRectangle(windowMask, GraphicsGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winMask);
    WinCopyRectangle(window, GraphicsGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winOverlay);
  }

  // animated tile?
  else {

    UInt8     imageOffset = 0;
    WinHandle window      = NULL;
   
    // lets check our rules
    switch (tile.tool)
    {
      case tileHoleOpening:
           imageOffset = tile.properties;
           window      = globals->winHoles;
           break;

      case tileHoleClosing:
           imageOffset = tile.properties;
           window      = globals->winHoles;
           break;

      default:
           break;
    }

    // update the screen
    rect.topLeft.x = imageOffset * rect.extent.x;
    WinCopyRectangle(window, GraphicsGetDrawWindow(),
                     &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
  }
}

/**
 * Backup a tile from the backgroud buffer.
 *
 * @param x the absolute x-coordinate in the background buffer
 * @param y the absolute y-coordinate in the background buffer
 * @param winBackup where to place the background buffer contents at x,y
 */
static void 
GameBackupTile(Coord     x, 
               Coord     y, 
               WinHandle winBackup)
{
  RectangleType scrRect, rect;

  rect.topLeft.x    = 0;
  rect.topLeft.y    = 0;
  rect.extent.x     = TILE_WIDTH;
  rect.extent.y     = TILE_HEIGHT;
  scrRect.topLeft.x = x + 2;
  scrRect.topLeft.y = y;
  scrRect.extent.x  = TILE_WIDTH;
  scrRect.extent.y  = TILE_HEIGHT;

  // take a backup copy!
  WinCopyRectangle(GraphicsGetDrawWindow(), winBackup,
                   &scrRect, rect.topLeft.x, rect.topLeft.y, winPaint);
}

/**
 * Restore a tile into the backgroud buffer.
 *
 * @param x the absolute x-coordinate in the background buffer
 * @param y the absolute y-coordinate in the background buffer
 * @param winBackup the contents of the old background buffer at x,y
 */
static void
GameRestoreTile(Coord     x, 
                Coord     y, 
                WinHandle winBackup)
{
  RectangleType scrRect, rect;

  rect.topLeft.x    = 0;
  rect.topLeft.y    = 0;
  rect.extent.x     = TILE_WIDTH;
  rect.extent.y     = TILE_HEIGHT;
  scrRect.topLeft.x = x + 2;
  scrRect.topLeft.y = y;
  scrRect.extent.x  = TILE_WIDTH;
  scrRect.extent.y  = TILE_HEIGHT;

  // restore the backup copy!
  WinCopyRectangle(winBackup, GraphicsGetDrawWindow(),
                   &rect, scrRect.topLeft.x, scrRect.topLeft.y, winPaint);
}
