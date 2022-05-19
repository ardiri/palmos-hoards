/*
 * @(#)resource.h
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

#ifdef PROTECTION_ON

#ifndef LANG_en
#error "unsupported translation for protected build."
#endif

#endif

// bitmaps

#define bitmapPalm              1000
#define bitmapPaw               1001
#define bitmapLogo              1003
#define bitmapIcon              1004
#define bitmapPGHQ              1005
#define bitmapAbout             1006
#define bitmapHelp              1007
#define bitmapSound             1008
#define bitmapPause             1009
#define bitmapGrayscaleTest     1010
#define bitmapKeyConfig         1020
#define bitmapNavigation        1030

#define bitmapTools             2000
#define bitmapHoles             2001
#define bitmapPlayers           2002
#define bitmapPlayersMask       2003
#define bitmapMonks             2004
#define bitmapMonksMask         2005

#define bitmapHelpSelectList    3000
#define bitmapHelpEditTiles     3001

// alerts

#define removeLevelSetAlert     1000
#define removeLevelAlert        1001
#define resetHighScoresAlert    1002

// dialogs

#define infoForm                2000
#define infoFormOkButton        2001
#define deviForm                2100
#define cfigForm                2300
#define cfigFormHardKey1Trigger 2301
#define cfigFormHardKey1List    2302
#define cfigFormHardKey2Trigger 2303
#define cfigFormHardKey2List    2304
#define cfigFormHardKey3Trigger 2305
#define cfigFormHardKey3List    2306
#define cfigFormHardKey4Trigger 2307
#define cfigFormHardKey4List    2308
#define cfigFormPageUpTrigger   2309
#define cfigFormPageUpList      2310
#define cfigFormPageDownTrigger 2311
#define cfigFormPageDownList    2312
#define cfigFormSound0Button    2313
#define cfigFormSound1Button    2314
#define cfigFormSound2Button    2315
#define cfigFormSound3Button    2316
#define cfigFormSound4Button    2317
#define cfigFormSound5Button    2318
#define cfigFormSound6Button    2319
#define cfigFormSound7Button    2320
#define cfigFormMuteCheckbox    2321
#define cfigFormStage0Trigger   2322
#define cfigFormStage0List      2323
#define cfigFormStage1Trigger   2324
#define cfigFormStage1List      2325
#define cfigFormStage2Trigger   2326
#define cfigFormStage2List      2327
#define cfigFormCheatCheckbox   2328
#define cfigFormOkButton        2329
#define cfigFormCancelButton    2330
#define grayForm                2400
#define grayWhite1Button        2401
#define grayWhite2Button        2402
#define grayWhite3Button        2403
#define grayWhite4Button        2404
#define grayWhite5Button        2405
#define grayWhite6Button        2406
#define grayWhite7Button        2407
#define grayLightGray1Button    2408
#define grayLightGray2Button    2409
#define grayLightGray3Button    2410
#define grayLightGray4Button    2411
#define grayLightGray5Button    2412
#define grayLightGray6Button    2413
#define grayLightGray7Button    2414
#define grayDarkGray1Button     2415
#define grayDarkGray2Button     2416
#define grayDarkGray3Button     2417
#define grayDarkGray4Button     2418
#define grayDarkGray5Button     2419
#define grayDarkGray6Button     2420
#define grayDarkGray7Button     2421
#define grayBlack1Button        2422
#define grayBlack2Button        2423
#define grayBlack3Button        2424
#define grayBlack4Button        2425
#define grayBlack5Button        2426
#define grayBlack6Button        2427
#define grayBlack7Button        2428
#define grayFormOkButton        2429
#define regiForm                2500
#define regiFormOkButton        2501
#define rbugForm                2600
#define rbugFormOkButton        2601
#define highForm                2700
#define highFormScoreField      2701
#define highFormLevelField      2702
#define highFormCodeField       2703
#define highFormOkButton        2704
#define helpForm                2800
#define helpFormScrollBar       2801
#define helpFormOkButton        2802
#define xmemForm                2900
#define xmemFormOkButton        2901
#define cr8rForm                3000
#define cr8rFormDBNameField     3001
#define cr8rFormOkButton        3002
#define cr8rFormCancelButton    3003
#define moveForm                3100
#define moveFormSStage0Trigger  3101
#define moveFormSStage0List     3102
#define moveFormSStage1Trigger  3103
#define moveFormSStage1List     3104
#define moveFormSStage2Trigger  3105
#define moveFormSStage2List     3106
#define moveFormDStage0Trigger  3107
#define moveFormDStage0List     3108
#define moveFormDStage1Trigger  3109
#define moveFormDStage1List     3110
#define moveFormDStage2Trigger  3111
#define moveFormDStage2List     3112
#define moveFormOkButton        3113
#define moveFormCancelButton    3114
#define gotoForm                3200
#define gotoFormStage0Trigger   3201
#define gotoFormStage0List      3202
#define gotoFormStage1Trigger   3203
#define gotoFormStage1List      3204
#define gotoFormStage2Trigger   3205
#define gotoFormStage2List      3206
#define gotoFormOkButton        3207
#define gotoFormCancelButton    3208
#define xlevForm                3300
#define xlevFormOkButton        3301

// forms

#define globalFormAboutButton   4001
#define globalFormHelpButton    4002
#define mainForm                4100
#define mainFormGameGadget      4101
#define mainFormScrollBar       4102
#define mainFormPlayButton      4103
#define editForm                4200
#define editFormPrevButton      4201
#define editFormGotoButton      4202
#define editFormNextButton      4203
#define editFormEmptyButton     4204
#define editFormPlayerButton    4205
#define editFormMonkButton      4206
#define editFormGoldButton      4207
#define editFormLadderButton    4208
#define editFormRopeButton      4209
#define editFormBrickButton     4210
#define editFormFallThruButton  4211
#define editFormRockButton      4212
#define editFormEndLadderButton 4213  // *must* be sequential, and in order
#define gameForm                4300
#define gameFormSoundButton     4301
#define gameFormPauseButton     4302

// menus

#define menuItemGrayscale       5001
#define menuItemGrayPalette     5002
#define menuItemConfig          5003
#define menuItemRegistration    5004
#define menuItemHelp            5006
#define menuItemAbout           5007
#define mainMenu_nogray         5100
#define mainMenu_gray           5101
#define mainMenu_nogray_nobeam  5102
#define mainMenu_gray_nobeam    5103
#define mainMenuItemPlay        5110
#define mainMenuItemBeam        5111
#define mainMenuItemCreate      5112
#define mainMenuItemEdit        5113
#define mainMenuItemDelete      5114
#define mainMenuItemHigh        5115
#define mainMenuItemResetHigh   5116
#define gameMenu_nogray         5200
#define gameMenu_gray           5201
#define gameMenuItemPause       5211
#define gameMenuItemSuicide     5212
#define gameMenuItemExit        5213
#define editMenu                5300
#define editMenuItemSave        5310
#define editMenuItemRevert      5311
#define editMenuItemCreate      5312
#define editMenuItemNext        5313
#define editMenuItemPrev        5314
#define editMenuItemGoto        5315
#define editMenuItemMove        5316
#define editMenuItemDelete      5317
#define editMenuItemExit        5318
#define editMenuItemPanLeft     5319
#define editMenuItemPanRight    5320
#define editMenuItemPanUp       5321
#define editMenuItemPanDown     5322
#define editMenuItemFill        5323
