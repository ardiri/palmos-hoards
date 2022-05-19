/*
 * @(#)palm.h
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

#ifndef _PALM_H
#define _PALM_H

// system includes
#include <PalmOS.h>
#include <System/DLServer.h>

// resource "include" :P
#include "resource.h"

// special includes (additional hardware etc)
#include "hardware/GPDLib.h"

// application constants and structures
#define appCreator 'LODE'
#define levelType  '_lev'
#define __REGISTER__ __attribute__ ((section ("register"))) // code0002.bin
#define __DEVICE__   __attribute__ ((section ("device")))   // code0003.bin
#define __GAME__     __attribute__ ((section ("game")))     // code0004.bin
#define __HELP__     __attribute__ ((section ("help")))     // code0005.bin
#define __SAFE0001__ __attribute__ ((section ("safe0001"))) // code0006.bin
#define __SAFE0002__ __attribute__ ((section ("safe0002"))) // code0007.bin

#define ftrGlobals               1000
#define ftrGlobalsCfgActiveVol   1001
#define ftrDeviceGlobals         2000
#define ftrGraphicsGlobals       3000
#define ftrGameGlobals           4000
#define ftrEditorGlobals         5000
#define ftrHelpGlobals           6000
#define ftrHelpGlobalsActiveForm 6001
#define ftrRegisterGlobals       1006 // <-- must be, as original code0007.bin
//#define ftrRegisterGlobals     7000

#define MAX_IDLENGTH  8 
#define MAX_LISTITEMS 32   // 32 items MAX in the lists
#define GAME_FPS      8    // 8 frames per second
#define VERSION       0

#define TILE_WIDTH    6
#define TILE_HEIGHT   8
#define GRID_WIDTH    26
#define GRID_HEIGHT   16   // 26x16 grid
#define MAX_MONKS     8    // max of 8 monks
#define MAX_TYPE      8    // number of "posibilities" (2*2*2)
#define MAX_SETS      32   // max number of anything

#define SCREEN_WIDTH  160
#define SCREEN_HEIGHT 128  // 160x128 visible display area (game)

//
// perfs->game.loderunner.tileState[x][y]:
//
//  case 0:   empty area - walk ok! no animations (ladder, gold, key etc)
//  case ...  animataion - walk ok!
//  case 255: solid area - walk no :(
//
// -- Aaron Ardiri, 2000
//

#define holeDelaySec  12
#define holeDelay     (holeDelaySec * GAME_FPS)
#define holeFirst     1
#define holeLast      (holeFirst + holeDelay)

enum appEvents
{
  appRedrawEvent = firstUserEvent,
  appGenerateLevelSetList
};

enum tools
{
  toolEmpty = 0,
  toolPlayer,
  toolMonk,
  toolGold,
  toolLadder,
  toolRope,
  toolBrick,
  toolFallThru,
  toolRock,
  toolEndLadder,

  tilePlayerUpright,
  tilePlayerRight,
  tilePlayerLeft,
  tilePlayerGenerating, 

  tileMonkUpright,
  tileMonkRight,
  tileMonkLeft,
  tileMonkGenerating,  // these tiles must be "overlayed" (mask + or)

  tileHoleOpening,
  tileHoleClosing
};

#define toolLast        toolEndLadder
#define tileLastOverlay tileMonkGenerating
#define tileLast        tileHoleClosing

enum
{
  moveNone = 0,
  moveLeft,
  moveRight,
  moveUp,
  moveDown,
  digLeft,
  digRight
};

typedef struct
{
  Coord x;
  Coord y;
} PositionType;

typedef struct
{
  UInt8 tool:5;
  UInt8 properties:3;
} GameTileType;

typedef struct
{
  GameTileType tiles[GRID_WIDTH][GRID_HEIGHT];

  // only during game play <-- these are NOT saved in the level database
  UInt8        tileState[GRID_WIDTH][GRID_HEIGHT];

  UInt8        recoveryCount;
  PositionType recoveryPosition[MAX_SETS];
} LevelType;

#define levelSize       ((GRID_WIDTH*GRID_HEIGHT) * sizeof(GameTileType))

typedef struct
{
  struct 
  {
    UInt8          signatureVersion;        // a preference version number
    Char           signature[16];           // a "signature" for decryption
    Char           *hotSyncUsername;        // the HotSync user name on device
  } system;

  struct 
  {
    UInt16         ctlKeyLeftDig;           // key definition for left dig
    UInt16         ctlKeyRightDig;          // key definition for right dig
    UInt16         ctlKeyUp;                // key definition for move up
    UInt16         ctlKeyDown;              // key definition for move down
    UInt16         ctlKeyLeft;              // key definition for move left
    UInt16         ctlKeyRight;             // key definition for move right

    Boolean        cheatsEnabled;           // can we cheat?
    UInt8          startLevel;              // what is the starting level?

    Boolean        sndMute;                 // sound is muted?
    UInt16         sndVolume;               // the volume setting for sound

    UInt8          lgray;                   // the lgray configuration setting
    UInt8          dgray;                   // the dgray configuration setting
  } config;

  struct 
  {
    Char           strLevelSetName[32];     // the currently active level set!

    Boolean        gamePlaying;             // is there a game in play?
    Boolean        gamePaused;              // is the game currently paused?
    Boolean        gameWait;                // are we waiting for the user?
    UInt16         gameAnimationCount;      // a ticking counter
    UInt16         gameScore;               // the score of the player
    UInt16         gameLives;               // the number of lives remaining

    UInt16         gameHighScore;           // the high score
    UInt8          gameHighLevel;           // the high score level

    struct
    {
      UInt8        stageNumber;             // the level 
      Boolean      goldCollected;           // is the gold collected?
      LevelType    stage;                   // the actual level contents  

      UInt16       playerDir;               // the direction of the player 
      PositionType playerTarget;            // the target [i][j] of the player
      PositionType playerPosition;          // the position of the player
      UInt16       playerRecCount;          // the recovery state

      UInt16       monkCount;               // number of monks on the screen
      UInt16       monkWait[MAX_MONKS];     // the delay between monk moves
      UInt16       monkDir[MAX_MONKS];      // the direction of the player 
      PositionType monkTarget[MAX_MONKS];   // the target position of the monk
      PositionType monkPosition[MAX_MONKS]; // the position of the monk
      UInt16       monkRecCount[MAX_MONKS]; // the recovery state
      Boolean      monkHasGold[MAX_MONKS];  // does the monk have a gold?

    } loderunner;
  } game;

  struct 
  {
    Char           strLevelSetName[32];     // the currently active level set!

    Boolean        gameEditing;             // are we currently editing a game?
    LevelType      level;                   // the actual level contents
    GameTileType   editTool;                // the current editing tool
    UInt8          currLevel;               // the current level
  } editor;
  
} PreferencesType;

// this is our 'double check' for decryption - make sure it worked :P
#define CHECK_SIGNATURE(x) (StrCompare(x->system.signature, "|HaCkMe|") == 0)

// helper macro
#define ABS(a) (((a) < 0) ? -(a) : (a))

// local includes
#include "device.h"
#include "graphics.h"
#include "help.h"
#include "game.h"
#include "editor.h"
#include "register.h"
#include "gccfix.h"

// functions
extern UInt32  PilotMain(UInt16, MemPtr, UInt16);
extern void    InitApplication(void);
extern Boolean ApplicationHandleEvent(EventType *);
extern void    ApplicationDisplayDialog(UInt16);
extern void    EventLoop(void);
extern void    EndApplication(void);

#endif 
