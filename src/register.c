/*
 * @(#)register.c
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
  MemHandle adjustGame;
  UInt16    adjustGameSize;
} RegisterGlobals;

/**
 * Initialize the registration routines.
 *
 * @param the global preferences structure.
 */
void
RegisterInitialize(PreferencesType *prefs)
{
  RegisterGlobals *gbls;

  // create the globals object, and register it
  gbls = (RegisterGlobals *)MemPtrNew(sizeof(RegisterGlobals));
  MemSet(gbls, sizeof(RegisterGlobals), 0);
  FtrSet(appCreator, ftrRegisterGlobals, (UInt32)gbls);

#ifdef PROTECTION_ON
  // 
  // perform the registration check :))
  //

  StrCopy(prefs->system.signature, "-HaCkMe-");
  {
    MemHandle dataHandle;
    UInt8     *dataChunk;
    Int16     dataChunkSize;
    UInt8     *ptrCodeChunk;
    void      (*regiLoader)(PreferencesType *);

    // extract the "encrypted" code
    dataHandle    = DmGet1Resource('code', 0x0006);  // code0006.bin
    dataChunk     = (UInt8 *)MemHandleLock(dataHandle);
    dataChunkSize = MemHandleSize(dataHandle);
    ptrCodeChunk  = (UInt8 *)MemPtrNew(dataChunkSize);
    MemMove(ptrCodeChunk, dataChunk, dataChunkSize);
    MemHandleUnlock(dataHandle);
    DmReleaseResource(dataHandle);

    // decrypt it
    RegisterDecryptChunk(ptrCodeChunk, dataChunkSize, 0x0002, 0x00);

    // execute the code chunk
    regiLoader = (void *)ptrCodeChunk;
    regiLoader(prefs);

    // we dont need the memory anymore, dispose of it
    MemPtrFree(ptrCodeChunk);
  }
#else
  StrCopy(prefs->system.signature, "|HaCkMe|");
  _regiLoader(prefs);
#endif
}

/**
 * Display the "*UNREGISTERED*" message on the screen if needed.
 * 
 * @param the global preferences structure.
 */
void
RegisterShowMessage(PreferencesType *prefs)
{
  const RectangleType rect      = {{0,148},{160,14}};
  const CustomPatternType erase = {0,0,0,0,0,0,0,0};

  // we MUST be on the main form to do this
  if (FrmGetActiveFormID() == mainForm) {

    Coord x;
    Char  str[32];

    // erase the are behind
    WinSetPattern(&erase);
    WinFillRectangle(&rect, 0);

    // draw our status
#ifdef LANG_de
    StrCopy(str, (CHECK_SIGNATURE(prefs)) 
            ? "- REGISTRIERTE VERSION -" : "* NICHT REGISTRIERT *");
#else
    StrCopy(str, (CHECK_SIGNATURE(prefs)) 
            ? "- REGISTERED VERSION -" : "*UNREGISTERED*");
#endif
    x = (rect.extent.x - FntCharsWidth(str, StrLen(str))) >> 1; 
  
    WinDrawChars(str, StrLen(str), x, rect.topLeft.y);
  }
}

#ifdef PROTECTION_ON
/**
 * Has the demonstration version expired?
 * 
 * @param prefs the global preferences type.
 * @param startLevel the level they started playing on.
 * @return true if the demo has expired, false otherwise.
 */
Boolean
RegisterDemoExpired(PreferencesType *prefs, UInt8 startLevel)
{
  Boolean result;

  // only "5" playable levels in free version
  result = (
	    (!CHECK_SIGNATURE(prefs)) &&
            ((prefs->game.loderunner.stageNumber - startLevel) >= 5)
           );

  // demo time is up? :P
  if (result) {

    EventType event;

    // "please register" dialog :)
    ApplicationDisplayDialog(rbugForm);

    // GAME OVER - return to main screen
    MemSet(&event, sizeof(EventType), 0);
    event.eType            = menuEvent;
    event.data.menu.itemID = gameMenuItemExit;
    EvtAddEventToQueue(&event);
  }

  return result;
}
#endif

/**
 * Adjust the game status that is "sensitive" to cracking
 * 
 * @param prefs the global preferences type.
 * @param adjustType the adjustment definition.
 * @return true if the adjustment was performed, false otherwise.
 */
Boolean
RegisterAdjustGame(PreferencesType    *prefs,
                   GameAdjustmentType *adjustType)
{
  Boolean result = false;

  // this stuff can only be done if the user is registered
  if (CHECK_SIGNATURE(prefs)) {

    RegisterGlobals *gbls;
    Boolean         (*adjustGame)(PreferencesType *, GameAdjustmentType *);

    // get a globals reference
    FtrGet(appCreator, ftrRegisterGlobals, (UInt32 *)&gbls);

    // perform the 'adjustment' :)
    adjustGame = (void *)MemHandleLock(gbls->adjustGame);
    result     = adjustGame(prefs, adjustType);
    MemHandleUnlock(gbls->adjustGame);
  }

  return result;
}

/**
 * Generate a checksum of a data chunk.
 *
 * @param dataChunk a pointer to the chunk of data to decrypt.
 * @param dataChunkSize the size of the chunk of data to decrypt.
 */
UInt8
RegisterChecksum(UInt8 *dataChunk, 
                 Int16 dataChunkSize) 
{
  UInt8 key;
  Int16 i;

  // calculate a checksum of the data chunk
  key = 0;
  for (i=0; i<dataChunkSize; i++) {
    key ^= dataChunk[i];
  }

  return key;
}

/**
 * Decrypt a data chunk using an RC4 style algorithm.
 *
 * @param dataChunk a pointer to the chunk of data to decrypt.
 * @param dataChunkSize the size of the chunk of data to decrypt.
 * @param keyResource the resource ID of the key to be used in decryption. 
 * @param key the starting key value, zero if autogenerated.
 */
void
RegisterDecryptChunk(UInt8 *dataChunk, 
                     Int16 dataChunkSize,
                     Int16 keyResource,
                     UInt8 key) 
{
  MemHandle       keyChunkHandle;
  UInt8           *keyChunk;
  Int16           i, index, keyChunkSize;

  // 
  // NOTE: the decryption/encryption is similar to the RC4 (USA military)
  //       method for securing data. RC4 can be applied twice to get the
  //       original data (as it is an XOR based algorithm). The 'key' used
  //       by this algorithm is the "register" code segment. After each 
  //       byte is adjusted, the key is dynamically updated using a simple
  //       "windowing" effect.
  //
  //       *all* code used for registration purposes is placed in the 
  //       "register" code segment. patching this segment will create an
  //       invalid 'key' data set and decryption will result in the Palm
  //       device giving FATAL EXCEPTIONS upon execution of the data chunks.
  //
  //                                                      Aaron Ardiri, 2000

  keyChunkHandle = DmGet1Resource('code', keyResource);  // codeXXXX.bin
  keyChunkSize   = MemHandleSize(keyChunkHandle);
  keyChunk       = (UInt8 *)MemPtrNew(keyChunkSize);
  MemMove(keyChunk, (UInt8 *)MemHandleLock(keyChunkHandle), keyChunkSize);
  MemHandleUnlock(keyChunkHandle);
  DmReleaseResource(keyChunkHandle);

  // what is our starting value?
  key = (key == 0) ? RegisterChecksum(keyChunk, keyChunkSize) : key;

  // decrypt our data chunk, using an RC4 level algorithm
  index = key;
  for (i=0; i<dataChunkSize; i++) {

    // adjust the byte
    dataChunk[i] ^= key;

    // dynamically update the key
    do {
      index = (index + key + 1) % keyChunkSize;
      key   = keyChunk[index];
    } while (key == 0);
  }

  // clean up
  MemPtrFree(keyChunk);
}

/**
 * Terminate the registration routines.
 */
void
RegisterTerminate()
{
  RegisterGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrRegisterGlobals, (UInt32 *)&gbls);

  // clean up memory
  if (gbls->adjustGame) 
    MemHandleFree(gbls->adjustGame);
  MemPtrFree(gbls);

  // unregister global data
  FtrUnregister(appCreator, ftrRegisterGlobals);
}

/**
 * Perform the necessary data adjustments and verify the existance
 * of a registered "keygen.pdb" file and set the registration flag
 * appropriately.
 *
 * @param prefs the global preferences type.
 */
void
_regiLoader(PreferencesType *prefs)
{
  RegisterGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrRegisterGlobals, (UInt32 *)&gbls);

  // lets assume there is no keygen.pdb file
  gbls->adjustGame = NULL;

#ifdef PROTECTION_ON
  {
    DmOpenRef keygenDB;

    // try and open the database
    keygenDB = DmOpenDatabaseByTypeCreator('_key',appCreator,dmModeReadOnly);
    if (keygenDB != NULL) {

      MemHandle recordH;
      UInt8     *recordChunk, key;
      UInt16    i, recordChunkSize;

      // get a reference to the first record
      recordH = DmGetRecord(keygenDB, 0);

      // process the record
      recordChunkSize = MemHandleSize(recordH);
      recordChunk     = (UInt8 *)MemPtrNew(recordChunkSize);
      MemMove(recordChunk, MemHandleLock(recordH), recordChunkSize);

      // decrypt it (based on the @message file)
      key = 0;
      for (i=0; i<MAX_IDLENGTH; i++) {
        key += prefs->system.hotSyncUsername[i];
      }
      key = key & 0xff;
      if (key == 0) key = 0x20; // key *cannot* be zero

      RegisterDecryptChunk(recordChunk, recordChunkSize, 0x0007, key);

      // extract the 'decryption' signature
      StrNCopy(prefs->system.signature, (Char *)recordChunk, 16);
      prefs->system.signature[15] = '\0';

      // now get the 'adjustGame' function if the signature is good
      if (CHECK_SIGNATURE(prefs)) {
        gbls->adjustGameSize = recordChunkSize - 16;
        gbls->adjustGame     = MemHandleNew(gbls->adjustGameSize);
        MemMove(MemHandleLock(gbls->adjustGame),
                recordChunk+16, gbls->adjustGameSize);
        MemHandleUnlock(gbls->adjustGame);
      }

      // release the resource
      MemPtrFree(recordChunk);
      MemHandleUnlock(recordH);
      DmReleaseRecord(keygenDB, 0, false);

      DmCloseDatabase(keygenDB);
    }
  }
#else
  {
    MemHandle codeH = DmGet1Resource('code', 0x0007);
    gbls->adjustGameSize = MemHandleSize(codeH);
    gbls->adjustGame     = MemHandleNew(gbls->adjustGameSize);
    MemMove(MemHandleLock(gbls->adjustGame), 
            MemHandleLock(codeH), gbls->adjustGameSize);
    MemHandleUnlock(gbls->adjustGame);
    MemHandleUnlock(codeH);
  }
#endif
}

/**
 * Adjust the game status that is "sensitive" to cracking
 *
 * @param prefs the global preferences type.
 * @param adjustType the adjustment definition.
 * @return true if the adjustment was performed, false otherwise.
 */
Boolean
_adjustGame(PreferencesType    *prefs,
            GameAdjustmentType *adjustType) 
{
  Boolean result = true;

  // what task do we need to do?
  switch (adjustType->adjustMode) 
  {
    case gameScoreCode:
         {
           RegisterGlobals *gbls;
           UInt16          result, i, index;
           Char            *key;
	   UInt8           *keyChunk;

           // get a globals reference
           FtrGet(appCreator, ftrRegisterGlobals, (UInt32 *)&gbls);

           key = adjustType->data.scoreCode.key;
	   keyChunk = (UInt8 *)MemHandleLock(gbls->adjustGame);
       
           result = 0xcafe;
           index  = key[0] % gbls->adjustGameSize;
           for (i=1; (i < MAX_IDLENGTH) && (key[i] != '\0'); i++) {
             result -= (
                        ((keyChunk[index] & 0x4b) << 8) | 
                         (keyChunk[index] & 0xb4)
                       );
             result  = (((result & 0x5a5a) << 1) ^ ((result & 0xa5a5) >> 15));
             index   = (index + key[i] + 1) % gbls->adjustGameSize;
           }

	   MemHandleUnlock(gbls->adjustGame);
           adjustType->data.scoreCode.code = result % 0xffff;
         }
         break;

    case gameHighScore:
         
	 // did we beat the high score?
	 if (prefs->game.gameScore >= prefs->game.gameHighScore) {
	   prefs->game.gameHighScore = prefs->game.gameScore;
	   prefs->game.gameHighLevel = prefs->game.loderunner.stageNumber;
	 }
	 else
	   result = false;

         break;

    default:
         result = false;
         break;
  }

  return result;
}
