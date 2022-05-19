/*
 * @(#)editor.h
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

#ifndef _EDITOR_H
#define _EDITOR_H

#include "palm.h"

extern Boolean EditorInitialize(Char *, Boolean)                      __GAME__;
extern Boolean EditorCreateDatabase(Char *)                           __GAME__; 
extern void    EditorOpenDatabase(Char *)                             __GAME__; 
extern UInt8   EditorLevelCount()                                     __GAME__; 
extern void    EditorCreateLevel(LevelType *)                         __GAME__; 
extern void    EditorLoadLevel(LevelType *, UInt8)                    __GAME__; 
extern void    EditorSaveLevel(LevelType *, UInt8)                    __GAME__; 
extern void    EditorMoveLevel(UInt8, UInt8)                          __GAME__; 
extern void    EditorProcessPenTap(LevelType *, GameTileType, 
                                   Coord, Coord, Boolean)             __GAME__; 
extern void    EditorDrawLevel(LevelType *)                           __GAME__; 
extern void    EditorDeleteLevel(UInt8)                               __GAME__; 
extern void    EditorCloseDatabase()                                  __GAME__; 
extern void    EditorTerminate()                                      __GAME__;

#endif 
