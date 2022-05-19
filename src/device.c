/*
 * @(#)device.c
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

// globals variable structure
typedef struct
{
  UInt32  romVersion;                      // the rom version of the device
  UInt32  depthState;                      // the screen depth state (old)
  UInt16  deviceVolume;                    // the volume of the game on device
  Boolean deviceMuted;                     // the volume muting status
} DeviceGlobals;

static Err DeviceBeamWriteProc(const void *, UInt32*, void *)       __DEVICE__;

/**
 * Initialize the device.
 */
void
DeviceInitialize()
{
  DeviceGlobals *globals;
 
  // create the globals objects, and register it
  globals = (DeviceGlobals *)MemPtrNew(sizeof(DeviceGlobals));
  MemSet(globals, sizeof(DeviceGlobals), 0);
  FtrSet(appCreator, ftrDeviceGlobals, (UInt32)globals);

  // get the rom version for this device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &globals->romVersion);

  // only OS 3.0 and above have > 1bpp display via API's
  if (DeviceSupportsVersion(romVersion3)) {

    // save the current display state
    WinScreenMode(winScreenModeGet,NULL,NULL,&globals->depthState,NULL);

    // change into the "highest" possible mode
    {
      UInt32 depthsToTry[] = { 8, 2, 1 };
      UInt32 *depthPtr = &depthsToTry[0];

      // loop until a valid mode is found
      while (WinScreenMode(winScreenModeSet,NULL,NULL,depthPtr,NULL)) {

        // try the next depth
        depthPtr++;
      }

      // did we make it into 8bpp mode? = color
      if (*depthPtr == 8) {

        RGBColorType palette[] = {
                                   {  0, 0xff, 0xff, 0xff },
                                   {  1, 0x80, 0x80, 0x80 },
                                   {  2, 0x80, 0x00, 0x00 },
                                   {  3, 0x80, 0x80, 0x00 },
                                   {  4, 0x00, 0x80, 0x00 },
                                   {  5, 0x00, 0x80, 0x80 },
                                   {  6, 0x00, 0x00, 0x80 },
                                   {  7, 0x80, 0x00, 0x80 },
                                   {  8, 0xff, 0x00, 0xff },
                                   {  9, 0xc0, 0xc0, 0xc0 },
                                   { 10, 0xff, 0x00, 0x00 },
                                   { 11, 0xff, 0xff, 0x00 },
                                   { 12, 0x00, 0xff, 0x00 },
                                   { 13, 0x00, 0xff, 0xff },
                                   { 14, 0x00, 0x00, 0xff },
                                   { 15, 0x00, 0x00, 0x00 }
                                 };

        // lets change back to 4bpp grayscale
        *depthPtr = 4;
	WinScreenMode(winScreenModeSet,NULL,NULL,depthPtr,NULL);

	// and tweak a VGA 16 color palette
	WinPalette(winPaletteSet, 0, 16, palette);
      }
    }
  }
}

/**
 * Check the compatability status of the device we are working with.
 *
 * @return true if the device is supported, false otherwise.
 */
Boolean 
DeviceCheckCompatability()
{
  Boolean result = true;
  UInt16  libRef;

  // the device is only compatable if it is Palm Profressional or above
  result = (
            (
             DeviceSupportsVersion(romVersion3) ||
             (
              (!DeviceSupportsVersion(romVersion3)) &&
              (SysLibFind("Net.lib", &libRef) != sysErrLibNotFound)
             )
            ) &&
            ((DeviceGetSupportedDepths() & 0x01) != 0)
           );

  // not compatable :(
  if (!result) { 

    // display a incompatability dialog
    FormPtr form = FrmInitForm(deviForm);
    FrmDoDialog(form);
    FrmDeleteForm(form);

    // lets exit the application "gracefully" :>
    if (!DeviceSupportsVersion(romVersion2)) {
      AppLaunchWithCommand(sysFileCDefaultApp,sysAppLaunchCmdNormalLaunch,NULL);
    }
  }

  return result;
}

/**
 * Determine if the "DeviceGrayscale" routine is supported on the device.
 *
 * @return true if supported, false otherwise.
 */
Boolean
DeviceSupportsGrayscale() 
{
  Boolean result = false;

  // only OS 3.0 and above have > 1bpp display via API's
  if (DeviceSupportsVersion(romVersion3)) {

    UInt32 cpuID;

    // get the processor ID
    FtrGet(sysFtrCreator, sysFtrNumProcessorID, &cpuID);
    cpuID = cpuID & sysFtrNumProcessorMask;

    // the "rules" for grayscale support
    result = (
              (cpuID == sysFtrNumProcessor328) ||
              (cpuID == sysFtrNumProcessorVZ)  ||
              (cpuID == sysFtrNumProcessorEZ)
             ) &&
             (DeviceGetSupportedDepths() < 0x80); // if 8bpp, color device :)
  }

  return result;
}

/**
 * Grayscale routine/settings for the device.
 *
 * @param mode the desired mode of operation.
 * @param lgray the lGray index (0..6) in intensity
 * @param dgray the dGray index (0..6) in intensity
 */
void
DeviceGrayscale(UInt16 mode, 
                UInt8  *lgray, 
                UInt8  *dgray)
{
  UInt32 cpuID;

#define LGPMR1 ((unsigned char *)0xFFFFFA32)
#define LGPMR2 ((unsigned char *)0xFFFFFA33)

  // get the processor ID
  FtrGet(sysFtrCreator, sysFtrNumProcessorID, &cpuID);
  switch (cpuID & sysFtrNumProcessorMask)
  {
    case sysFtrNumProcessor328:
         {
           UInt8 gray[] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x07 };

           switch (mode)
           {
             case grayGet:
                  {
                    UInt8 data;

                    // light gray
                    data = (UInt8)(((*LGPMR1) & 0xF0) >> 4);
                    *lgray = 0;
                    while (gray[*lgray] < data) {
                      (*lgray)++;
                    }

                    // dark gray
                    data = (UInt8)((*LGPMR2) & 0x0F);
                    *dgray = 0;
                    while (gray[*dgray] < data) {
                      (*dgray)++;
                    }
                  }
                  break;

             case graySet:
                  *LGPMR1 = (gray[*lgray] << 4);
                  *LGPMR2 = (0x70 | gray[*dgray]);
                  break;

             default:
                  break;
           }
         }
         break;

    case sysFtrNumProcessorEZ:
    case sysFtrNumProcessorVZ:
         {
           UInt8 gray[] = { 0x00, 0x03, 0x04, 0x07, 0x0A, 0x0C, 0x0F };

           switch (mode)
           {
             case grayGet:
                  {
                    UInt8 data;

                    // light gray
                    data = (UInt8)((*LGPMR2) & 0x0F);
                    *lgray = 0;
                    while (gray[*lgray] < data) {
                      (*lgray)++;
                    }

                    // dark gray
                    data = (UInt8)(((*LGPMR2) & 0xF0) >> 4);
                    *dgray = 0;
                    while (gray[*dgray] < data) {
                      (*dgray)++;
                    }
                  }
                  break;

             case graySet:
                  *LGPMR2 = ((gray[*dgray] << 4) | gray[*lgray]);
                  break;

             default:
                  break;
           }
         }
         break;

    default:
         break;
  }
}

/**
 * Reset the device to its original state.
 */
void
DeviceTerminate()
{
  DeviceGlobals *globals;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&globals);

  // restore the current display state
  if (DeviceSupportsVersion(romVersion3)) {
    WinScreenMode(winScreenModeSet,NULL,NULL,&globals->depthState,NULL);
  }

  // clean up
  MemPtrFree(globals);                          // mem leak - fixed 24-Dec-2000

  // unregister global data
  FtrUnregister(appCreator, ftrDeviceGlobals);
}

/**
 * Play a sound on the device.
 *
 * @param sndCmd the sound information.
 */
void
DevicePlaySound(SndCommandType *sndCmd)
{
  DeviceGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&gbls);

  // are we muted?
  if (!gbls->deviceMuted) {

    UInt16 volumes[] = { 0x00,0x04,0x08,0x10,0x18,0x20,0x30,0x40 };

    // adjust to the right volume
    sndCmd->param3 = volumes[gbls->deviceVolume];

    // do it!
    SndDoCmd(0, sndCmd, 0);
  }
}

/**
 * Set the volume on the device.
 *
 * @param volume the volume level for the device.
 */
void
DeviceSetVolume(UInt16 volume)
{
  DeviceGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&gbls);

  // save the volume state
  gbls->deviceVolume = volume;
}

/**
 * Get the volume on the device.
 *
 * @return the volume level for the device.
 */
UInt16
DeviceGetVolume()
{
  DeviceGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&gbls);

  // return the volume state
  return gbls->deviceVolume;
}

/**
 * Set the mute status of the sound on the device.
 *
 * @param mute true if no sound wanted, false otherwise.
 */
void
DeviceSetMute(Boolean mute)
{
  DeviceGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&gbls);

  // save the mute state
  gbls->deviceMuted = mute;
}

/**
 * Get the mute status of the sound on the device.
 *
 * @return true if no sound wanted, false otherwise.
 */
Boolean
DeviceGetMute()
{
  DeviceGlobals *gbls;

  // get a globals reference
  FtrGet(appCreator, ftrDeviceGlobals, (UInt32 *)&gbls);

  // return the mute state
  return gbls->deviceMuted;
}

/**
 * Get the supported depths the device can handle. 
 *
 * @return the depths supported (1011b = 2^3 | 2^1 | 2^0 = 4,2,1 bpp).
 */
UInt32  
DeviceGetSupportedDepths()
{
  UInt32 result = 0x00000001;

  // only OS 3.0 and above have > 1bpp display via API's
  if (DeviceSupportsVersion(romVersion3)) {
    WinScreenMode(winScreenModeGetSupportedDepths,NULL,NULL,&result,NULL);
  }

  return result;
}

/**
 * Check if the device is compatable with a particular ROM version.
 *
 * @param version the ROM version to compare against.
 * @return true if it is compatable, false otherwise.
 */
Boolean 
DeviceSupportsVersion(UInt32 version)
{
  UInt32 romVersion;

  // get the rom version for this device
  FtrGet(sysFtrCreator, sysFtrNumROMVersion, &romVersion);

  return (romVersion >= version);
}

/**
 * Determine the pointer to the bitmap data chunk for a specific window.
 *
 * @param win the window.
 * @return a pointer to the bitmap data chunk.
 */
void *
DeviceWindowGetPointer(WinHandle win)
{
  void *result = NULL;

  // palmos 3.5        - use BmpGetBits()
  if (DeviceSupportsVersion(romVersion3_5)) {
    result = BmpGetBits(WinGetBitmap(win));
  }

  // palmos pre 3.5    - use standard technique
  else
    result = (void *)win->displayAddrV20;
    
  return result;
}

/**
 * Determine if the "DeviceBeamDatabase" routine is supported on the device.
 *
 * @return true if supported, false otherwise.
 */
Boolean
DeviceSupportsBeaming() 
{
  return (DeviceSupportsVersion(romVersion3));
}

/**
 * Send a database using the Exchange API (Beam).
 *
 * @param levelName the level set to beam.
 * @return true if successful, false otherwise.
 */
Boolean
DeviceBeamDatabase(Char *levelName)
{
  Char    dbName[32];
  UInt16  card;
  LocalID dbID;
  Boolean result = true;

  // initialize
  StrPrintF(dbName, "LODE_%s", levelName);
  card = 0;
  dbID = DmFindDatabase(card, dbName);

  // what? not there.. must be on card 1
  if (dbID == NULL) {
    card = 1;
    dbID = DmFindDatabase(card, dbName);
  }

  // send over the database
  if (dbID != NULL) {
  
    ExgSocketType socket;
    Err           err, e;
    Char          strName[32];

    // we need to add the ".pdb" extension (so receiver knows what to do)
    StrPrintF(strName, "%s.pdb", dbName);
    
    // configure
    MemSet(&socket, sizeof(ExgSocketType), 0);
    socket.target      = sysFileCLauncher;
    socket.description = levelName;
    socket.name        = strName;
    
    err = errNone;
    e = ExgPut(&socket); err |= e;
    if (err == errNone) {
      e = ExgDBWrite(DeviceBeamWriteProc, &socket, NULL, dbID, card); err |= e;
      e = ExgDisconnect(&socket, err); err |= e;
    }

    // did everything go ok?
    result = (err == errNone);
  }
  else
    result = false;

  return result;
}

/**
 * ExgDBWrite callback function.
 *
 * @param data the data to send.
 * @param size the size of the packet.
 * @param userData the protocol.
 * @return errNone if successful, other otherwise.
 */
static Err 
DeviceBeamWriteProc(const void *data, 
                    UInt32     *size, 
		    void       *userData)
{
  Err err = errNone; 

  // send it over!
  *size = ExgSend((ExgSocketType *)userData, (MemPtr)data, *size, &err);

  return err;
}
