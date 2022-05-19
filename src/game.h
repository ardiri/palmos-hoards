/*
 * @(#)game.h
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

#ifndef _GAME_H
#define _GAME_H

#include "palm.h"

extern Boolean GameInitialize(PreferencesType *)                       __GAME__;
extern Boolean GameHighScoreCandidate(PreferencesType *)               __GAME__;
extern void    GameResetPreferences(PreferencesType *)                 __GAME__;
extern void    GameProcessKeyInput(PreferencesType *, UInt32)          __GAME__;
extern void    GameProcessStylusInput(PreferencesType *, Coord, Coord) __GAME__;
extern void    GameKillPlayer(PreferencesType *)                       __GAME__;
extern void    GameMovement(PreferencesType *)                         __GAME__;
extern void    GameDrawLevel(PreferencesType *)                        __GAME__;
extern void    GameDraw(PreferencesType *)                             __GAME__;
extern void    GameTerminate()                                         __GAME__;

#endif 
