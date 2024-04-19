#include "stdafx.h"
#ifndef FILE12_H
#include "file12.h"

#define FILEVERSION200 200
#define FILEVERSION300 300
#define FILEVERSION301 301
#define FILEVERSION302 302
#define FILEVERSION303 303             // starting 04.2008 with CamWare 2.21


#endif

void Shift(WORD* data, unsigned long k, bool bup, unsigned long bits)
{
  if(bup)
  {
    for(unsigned int i = 0; i < k; i++)
      data[i] <<= bits;
  }
  else
  {
    for(unsigned int i = 0; i < k; i++)
      data[i] >>= bits;
  }
}



/********************************************************************************************************/
/* added TIFF - Reader... Franz Reitner/24.06.1999                                                        */
/********************************************************************************************************/
const short ImageWidth      = 256;     // IFD - entries (Image File Descriptor)
const short ImageHeight     = 257;
const short BitsPerPixel    = 258;
const short Compression     = 259;
const short PhotoInterp     = 262;
const short StripOffsets    = 273;
const short SamplesPerPixel = 277;
const short RowsPerStrip    = 278;
const short StripByteCnt    = 279;
const short XResolution     = 282;
const short YResolution     = 283;
const short PlanarConfig    = 284;
const short ResolutionUnits = 296;
const short ColorMap        = 320;

/* General TIF - File: Header 0x49 0x49 0x2A 0x2A (Identification with version)
XXXXYYYY(Offset for IFD)
.... DATA (usually before or after IFD)
at adress XXXXYYYY: ZZZZ Number of entries in IFD
.... IFDs
.... DATA (usually before or after IFD) */

typedef struct                         // IFD - Structure
{
  unsigned short TagField;             // IFD ID
  unsigned short ftype;                // Type (Byte,String,Short,Long,Float)
  unsigned long length;                // Number of entries
  unsigned long Offset;                // Date, if entry # =1; otherwise offset in file
}TE;

TE TIFFEntry;

int IsTiffFile(char *filename, HANDLE file)
{
  char bfr[4];
  DWORD read, currPos;
  int ok;
  bool bl = FALSE;


  if ((file == NULL) &&(filename == NULL))
    return false;

  if ((file == NULL) &&(filename != NULL))
  {
    file = CreateFile(filename,
      GENERIC_READ,
      0,
      0,
      OPEN_EXISTING,
      0,
      0);
    bl = TRUE;
  }
  if (file == INVALID_HANDLE_VALUE)
    return false;

  currPos = SetFilePointer(file, 0, 0, FILE_CURRENT);
  SetFilePointer(file, 0, 0, FILE_BEGIN);
  if (!ReadFile(file, bfr, 4, &read, 0))
  {
    ok = FALSE; 
  }
  else
  {
    if ((bfr[0] == 'I') &&(bfr[1] == 'I') &&(bfr[2] == 0x2A) &&(bfr[3] == 0))
      ok = FILEISOK;                   // read INTEL-format only
    else                               // Intel saves Lowbyte then Highbyte
    if ((bfr[0] == 'M') &&(bfr[1] == 'M') &&(bfr[3] == 0x2A) &&(bfr[2] == 0))
      ok = FILEISOK | FILEISMACFORMAT;
    else
      ok = 0;
  }
  SetFilePointer(file, currPos, 0, FILE_BEGIN);
  if (bl)
    CloseHandle(file);
  return ok;
}                                      // IsTiffFile

void mswab(byte* src, byte* dest, int isize)
{
  dest+= isize;
  dest--;
  for(int i = 0; i < isize; i++)
  {
    *dest = *src;
    src++;
    dest--;
  }
}

int getsize_tif (char *filename, int *iXRes, int *iYRes, bool *bDouble)
{
  HANDLE hfread;
  unsigned long DataOffset = 0;

  int width = 0, height = 0, ifdcnt = 0;
  long offset = 0, lh;
  unsigned short ush, ush2;
  DWORD read = 0;
  Bild *strBild = {0};
  bool bmacformat = FALSE;

  *bDouble = FALSE;
  hfread = CreateFile(filename,
    GENERIC_READ,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
    0);

  if (hfread== INVALID_HANDLE_VALUE)
  {
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }

  ifdcnt = IsTiffFile(NULL, hfread);
  if (ifdcnt == 0)
  {
    CloseHandle(hfread);
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }
  if((ifdcnt & FILEISMACFORMAT) == FILEISMACFORMAT)
    bmacformat = TRUE;

  SetFilePointer(hfread, 4, 0, FILE_BEGIN);
  ReadFile(hfread, &offset, sizeof(offset), &read, 0);
  if(bmacformat)
  {
    mswab((byte*)&offset, (byte*)&lh, sizeof(lh));
    offset = lh;
  }
  SetFilePointer(hfread, offset, 0, FILE_BEGIN);
  ReadFile(hfread, &ush, 2, &read, 0);
  if(bmacformat)
  {
    mswab((byte*)&ush, (byte*)&ush2, sizeof(ush2));
    ifdcnt = ush2;
  }
  else
    ifdcnt = ush;

  offset += 2;

  for (int i = 0; i < ifdcnt; i++)
  {
    SetFilePointer(hfread, offset + i * sizeof(TIFFEntry), 0, FILE_BEGIN);
    ReadFile(hfread, &TIFFEntry, sizeof(TIFFEntry), &read, 0);
    if(bmacformat)
    {
      mswab((byte*)&TIFFEntry.ftype, (byte*)&ush, sizeof(ush));
      TIFFEntry.ftype = ush;
      mswab((byte*)&TIFFEntry.TagField, (byte*)&ush, sizeof(ush));
      TIFFEntry.TagField = ush;
      mswab((byte*)&TIFFEntry.length, (byte*)&lh, sizeof(lh));
      TIFFEntry.length = lh;
      if(TIFFEntry.length == 1)
      {
        if(TIFFEntry.ftype == 3)
        {
          mswab((byte*)&TIFFEntry.Offset, (byte*)&ush, sizeof(ush));
          TIFFEntry.Offset = ush;
        }
        if(TIFFEntry.ftype == 4)
        {
          mswab((byte*)&TIFFEntry.Offset, (byte*)&lh, sizeof(lh));
          TIFFEntry.Offset = lh;
        }
      }
    }
    if (TIFFEntry.TagField == ImageWidth)
      width = TIFFEntry.Offset;
    if (TIFFEntry.TagField == ImageHeight)
      height = TIFFEntry.Offset;
    if (TIFFEntry.TagField == 0xC53F)
    {
      unsigned char* pucdat;
      int ilen = -1;

      DataOffset = TIFFEntry.Offset;
      if(TIFFEntry.ftype == 1)
      {
        ilen = TIFFEntry.length;
      }
      if(TIFFEntry.ftype == 3)
      {
        ilen = TIFFEntry.length * sizeof(short);
      }
      if(TIFFEntry.ftype == 4)
      {
        ilen = TIFFEntry.length * sizeof(long);
      }
      if (ilen == -1)
      {
        CloseHandle(hfread);
        return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
      }
      if(ilen > sizeof(Bild))
        ilen = sizeof(Bild);
      pucdat = (unsigned char*) malloc(ilen + 0x10);
      strBild = (Bild*)&pucdat[0];

      if((TIFFEntry.ftype == 4)||(TIFFEntry.ftype == 1))// Type 4: Multitif (old); Type 1: Multitif and normal Tif (new, 303) 
      {
        strBild = (Bild*)&pucdat[2];
      }

      SetFilePointer(hfread, DataOffset, 0, FILE_BEGIN);
      ReadFile(hfread, &pucdat[2], ilen, &read, 0);

      //ReadFile(hfread, &SCnt, sizeof(word), &read, 0);
      //ReadFile(hfread, &strBild.sTime, SCnt, &read, 0);

      if((strBild->iVersion == FILEVERSION200) ||
        (strBild->iVersion == FILEVERSION300) ||
        (strBild->iVersion == FILEVERSION301) ||
        (strBild->iVersion == FILEVERSION302) ||
        (strBild->iVersion == FILEVERSION303))
      {
        *bDouble = strBild->bDouble;
      }
      else
        *bDouble = FALSE;
      free(pucdat);
    }
  }

  CloseHandle(hfread);

  if (!((width != 0) &&(height != 0)))
  {
    *iXRes = 0;
    *iYRes = 0;
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }
  *iXRes = width;
  *iYRes = height;

  return (PCO_NOERROR);
}

int read_tif (char *filename, Bild* strBild, int iNoShifting)
{
  HANDLE hfread = NULL;
  void *im;

  int width = 0, height = 0, ifdcnt = 0, iImageType = 0, iBitsR = 0, 
    iBitsG = 0, iBitsB = 0, iRowsPerStrip = 0;
  unsigned long offset = 0, DataOffset = 0, lFileLengthL, lFileLengthH;
  long lh;
  unsigned short ush, ush2;
  int ishift;
  bool bmulti = FALSE;
  bool bmacformat = FALSE;
  int    SDataBytes = 0, SDataCnt = 0, SBytes = 0, SCnt = 0;

  DWORD *SDataOffset = NULL, *SByteCnt = NULL;
  DWORD read = 0;
  WORD  *p = NULL;
  int err = PCO_NOERROR;

  im = strBild->pic12;

  if(strBild->bAlignUpper)
  {
    ishift = (16 - strBild->iBitRes);
  }
  else
    ishift = 0;

  hfread = CreateFile(filename,
    GENERIC_READ,
    0,
    NULL,
    OPEN_EXISTING,
    FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS,
    0);

  lFileLengthL = GetFileSize(hfread, &lFileLengthH);

  if (hfread== INVALID_HANDLE_VALUE)
  {
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }

  SCnt = IsTiffFile(NULL, hfread);
  if (SCnt == 0)
  {
    CloseHandle(hfread);
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }
  if((SCnt & FILEISMACFORMAT) == FILEISMACFORMAT)
    bmacformat = TRUE;

  SetFilePointer(hfread, 4, 0, FILE_BEGIN);
  ReadFile(hfread, &offset, sizeof(offset), &read, 0);
  if(bmacformat)
  {
    mswab((byte*)&offset, (byte*)&lh, sizeof(lh));
    offset = lh;
  }
  SetFilePointer(hfread, offset, 0, FILE_BEGIN);
  ReadFile(hfread, &ush, 2, &read, 0);
  if(bmacformat)
  {
    mswab((byte*)&ush, (byte*)&ush2, sizeof(ush2));
    ifdcnt = ush2;
  }
  else
    ifdcnt = ush;
  offset += 2;

  for (int i = 0; i < ifdcnt; i++)      // Entries like TIFF - description (tiff.pdf of 03.06.1992)
  {                                     //                       (is current Version! 24.06.1999/Franz)
    //                       (http:// www.adobe.com)
    SetFilePointer(hfread, offset + i * sizeof(TIFFEntry), 0, FILE_BEGIN);
    ReadFile(hfread, &TIFFEntry, sizeof(TIFFEntry), &read, 0);
    if(bmacformat)
    {
      mswab((byte*)&TIFFEntry.ftype, (byte*)&ush, sizeof(ush));
      TIFFEntry.ftype = ush;
      mswab((byte*)&TIFFEntry.TagField, (byte*)&ush, sizeof(ush));
      TIFFEntry.TagField = ush;
      mswab((byte*)&TIFFEntry.length, (byte*)&lh, sizeof(lh));
      TIFFEntry.length = lh;
      if(TIFFEntry.length == 1)
      {
        if(TIFFEntry.ftype == 3)
        {
          mswab((byte*)&TIFFEntry.Offset, (byte*)&ush, sizeof(ush));
          TIFFEntry.Offset = ush;
        }
        if(TIFFEntry.ftype == 4)
        {
          mswab((byte*)&TIFFEntry.Offset, (byte*)&lh, sizeof(lh));
          TIFFEntry.Offset = lh;
        }
      }
      else
      {
        mswab((byte*)&TIFFEntry.Offset, (byte*)&lh, sizeof(lh));
        TIFFEntry.Offset = lh;
      }
    }
    switch (TIFFEntry.TagField)
    {
      case ImageWidth:
      {
        if (TIFFEntry.ftype == 3)
          width = TIFFEntry.Offset;// word
        if (TIFFEntry.ftype == 4)
          width = TIFFEntry.Offset;// dword
        break;
      }
      case ImageHeight:
      {
        if (TIFFEntry.ftype == 3)
          height = TIFFEntry.Offset;// word
        if (TIFFEntry.ftype == 4)
          height = TIFFEntry.Offset;// dword
        break;
      }
      case BitsPerPixel:               // Bit resolution in bit per Pixel (8bit BW, 16bit BW or 24bit RGB)
      {
        short sValue;

        sValue =(short)TIFFEntry.Offset;// word
        if (TIFFEntry.length == 1)
        {
          if ((sValue > 16) || (sValue < 8))
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE;
          if (sValue <= 8)
            iImageType = 1;
          else
            iImageType = 2;            // 1 -> 8 bit BW; 2 -> 9...16 bit BW
        }
        if ((TIFFEntry.length == 3) ||(TIFFEntry.length == 4))
        {
          short buf[5];

          if (TIFFEntry.ftype == 3)
          {
            SetFilePointer(hfread, TIFFEntry.Offset, 0, FILE_BEGIN);
            ReadFile(hfread, buf, 6, &read, 0);
            if(bmacformat)
            {
              mswab((byte*)&buf[0], (byte*)&ush, sizeof(ush));
              buf[0] = ush;
              mswab((byte*)&buf[1], (byte*)&ush, sizeof(ush));
              buf[1] = ush;
              mswab((byte*)&buf[2], (byte*)&ush, sizeof(ush));
              buf[2] = ush;
            }
            iBitsR = buf[0];
            iBitsG = buf[1];
            iBitsB = buf[2];
            if ((iBitsR > 8) ||(iBitsG > 8) ||(iBitsB > 8))
              err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
            else
              iImageType = TIFFEntry.length;          // 3 -> RGB Bild
          }
          else
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        }
        break;
      }
      case Compression:                // 1: unpacked, 2: Huffman, 3: Fax G3,
      {                               // 4: Fax G4, 5: LZW, 32773: PackBits
        if (TIFFEntry.Offset != 1)// word
          err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        break;
      }
      case PhotoInterp:                // 0: bilevel and gray levels, 0 is white, 1: bilevel a. Gr., 0 is black
      {                               // 2: RGB 3: RGB by palette (nicht implemented)
        if ((TIFFEntry.Offset != 1) &&(TIFFEntry.Offset != 2))// word
          err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        break;
      }
      case 0xC53F:
      {
        unsigned char* pucdat;
        int ilen = -1;
        Bild *strbildl;

        DataOffset = TIFFEntry.Offset;
        if(TIFFEntry.ftype == 1)
        {
          ilen = TIFFEntry.length;
        }
        if(TIFFEntry.ftype == 3)
        {
          ilen = TIFFEntry.length * sizeof(short);
        }
        if(TIFFEntry.ftype == 4)
        {
          ilen = TIFFEntry.length * sizeof(long);
          bmulti = TRUE;
        }
        if (ilen == -1)
        {
          CloseHandle(hfread);
          return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
        }
        if(ilen > (sizeof(Bild) - sizeof(void*)))
          ilen = (sizeof(Bild) - sizeof(void*));
        pucdat = (unsigned char*) malloc(sizeof(Bild) + 0x10);
        memset(pucdat, 0, sizeof(Bild) + 0x10);
#if defined _WIN64
        strbildl = (Bild*)&pucdat[0];
#else
        strbildl = (Bild*)&pucdat[4];
#endif
        if((TIFFEntry.ftype == 4)||(TIFFEntry.ftype == 1))// 4: Multi(old) 1: tif and multi (new, >= 303)
        {
#if defined _WIN64
          strbildl = (Bild*)&pucdat[2];
#else
          strbildl = (Bild*)&pucdat[6];
#endif
        }

        SetFilePointer(hfread, DataOffset, 0, FILE_BEGIN);
        ReadFile(hfread, &pucdat[6], ilen, &read, 0);
        if((strbildl->iVersion == FILEVERSION200) ||
          (strbildl->iVersion == FILEVERSION300) ||
          (strbildl->iVersion == FILEVERSION301) ||
          (strbildl->iVersion == FILEVERSION302) ||
          (strbildl->iVersion > FILEVERSION303))
        {
          memcpy(&strBild->sTime, &strbildl->sTime, ilen);
          if(strBild->iVersion < FILEVERSION200)
          {
            strBild->iBWMin2 = strBild->iBWMin;                   // Lut bw min
            strBild->iBWMax2 = strBild->iBWMax;                   // Lut bw max
            strBild->iBWLut2 = strBild->iBWLut;                   // Lut lin log
            strBild->iRMin2 = strBild->iRMin;                    // red min
            strBild->iRMax2 = strBild->iRMax;                    // red max
            strBild->iGMin2 = strBild->iGMin;                    // green min
            strBild->iGMax2 = strBild->iGMax;                    // green max
            strBild->iBMin2 = strBild->iBMin;                    // blue min
            strBild->iBMax2 = strBild->iBMax;                    // blue max
            strBild->iColLut2 = strBild->iColLut;                  // Lut lin log color
            strBild->bAlignUpper = FALSE;
          }

          if(strBild->iVersion < FILEVERSION300)
          {
            strBild->dGammaLut = 1.0;                 // Gamma value b/w
            strBild->dGammaLutC = 1.0;                // Gamma value color
            strBild->dGammaLut2 = 1.0;                // Gamma value b/w 2
            strBild->dGammaLutC2 = 1.0;               // Gamma value color 2
          }

          if(strBild->iVersion < FILEVERSION301)
          {
            strBild->iBitRes = 12;                    // Assume 14bit resolution
          }

          if(strBild->iVersion < FILEVERSION302)
            strBild->dSaturation = 100;
          // close filehandle
        }
        free(pucdat);

        break;
      }
      case StripOffsets:               // Adress of single strips in file
      {
        DataOffset = TIFFEntry.Offset;
        if (TIFFEntry.length > 1)
        {
          char cSize[6] = { 0, 1, 0, 2, 4, 8 };
          int iBytes;

          if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
          {
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
            break;
          }
          iBytes = cSize[TIFFEntry.ftype];

          SDataOffset =(DWORD*)malloc(TIFFEntry.length * iBytes);

          SetFilePointer(hfread, DataOffset, 0, FILE_BEGIN);
          ReadFile(hfread, SDataOffset, TIFFEntry.length * iBytes, &read, 0);
          if(bmacformat)
          {
            for(unsigned int i = 0; i < TIFFEntry.length; i++)
            {
              if(iBytes == 2)
              {
                mswab((byte*)&SDataOffset[i], (byte*)&ush, sizeof(ush));
                SDataOffset[i] = ush;
              }
              else
              {
                mswab((byte*)&SDataOffset, (byte*)&lh, sizeof(lh));
                SDataOffset[i] = lh;
              }
            }
          }
          SDataBytes = iBytes;
          SDataCnt   = TIFFEntry.length;
          if (read !=(DWORD)(SDataCnt * SDataBytes))
          {
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
            break;
          }
        }
        else
        {
          if (TIFFEntry.length == 1)
          {
            char cSize[6] = { 0, 1, 0, 2, 4, 8 };
            int iBytes;

            if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
            {
              err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
              break;
            }
            iBytes = cSize[TIFFEntry.ftype];

            SDataOffset =(DWORD*)malloc(TIFFEntry.length * iBytes);
            *SDataOffset = DataOffset;
            SDataCnt   = TIFFEntry.length;
          }
          else
          {
            SDataOffset = NULL;
          }
        }
        break;
      }
      case SamplesPerPixel:            // 1: BW, 3: RGB (24bit), 4: RGB (32bit)
      {
        if ((TIFFEntry.Offset != 1) &&(TIFFEntry.Offset != 3) &&(TIFFEntry.Offset != 4))// word
          err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        break;
      }
      case RowsPerStrip:               // Number of pixel per strip
      {
        if (TIFFEntry.ftype == 3)
          iRowsPerStrip = TIFFEntry.Offset;
        else
          if (TIFFEntry.ftype == 4)
            iRowsPerStrip = TIFFEntry.Offset;
          else
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        break;
      }
      case StripByteCnt:               // Number of bytes per strip
      {
        DataOffset = TIFFEntry.Offset;
        char cSize[6] = { 0, 1, 0, 2, 4, 8 };
        int iBytes;

        if (TIFFEntry.length > 1)
        {
          if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
          {
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
            break;
          }
          iBytes = cSize[TIFFEntry.ftype];

          SByteCnt =(DWORD*)malloc(TIFFEntry.length * iBytes);

          SetFilePointer(hfread, DataOffset, 0, FILE_BEGIN);
          ReadFile(hfread, SByteCnt, TIFFEntry.length * iBytes, &read, 0);
          if(bmacformat)
          {
            for(unsigned int i = 0; i < TIFFEntry.length; i++)
            {
              if(iBytes == 2)
              {
                mswab((byte*)&SByteCnt[i], (byte*)&ush, sizeof(ush));
                SByteCnt[i] = ush;
              }
              else
              {
                mswab((byte*)&SByteCnt, (byte*)&lh, sizeof(lh));
                SByteCnt[i] = lh;
              }
            }
          }
          SBytes = iBytes;
          SCnt   = TIFFEntry.length;

          if (read !=(DWORD)(SCnt * SBytes))
          {
            err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
          }
        }
        else
        {
          if (TIFFEntry.length == 1)
          {
            if ((TIFFEntry.ftype != 3) &&(TIFFEntry.ftype != 4))
            {
              err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
              break;
            }
            iBytes = cSize[TIFFEntry.ftype];
            SBytes = iBytes;
            SByteCnt =(DWORD*)malloc(TIFFEntry.length * iBytes);
            *SByteCnt = TIFFEntry.Offset;
            SCnt   = TIFFEntry.length;
          }
          else
          {
            SByteCnt = NULL;
          }
        }
        break;
      }
      case PlanarConfig:               // 1: BW, normal RGB (RGBRGBRGB...) 2: Planes (not implemented)
      {
        if (TIFFEntry.Offset != 1)// word
          err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
        break;
      }
    }
    if (err != 0)
      break;
  }

  if (SDataCnt != SCnt)
    err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;

  if(bmulti == TRUE)
  {
    if(*SByteCnt != (DWORD)(width * height * iImageType))
      *SByteCnt = (DWORD)(width * height * iImageType);// correct error in mmfilewriter...
  }

  if (err)
  {
    CloseHandle(hfread);
    if (SDataOffset != NULL)
      free(SDataOffset);
    if (SByteCnt != NULL)
      free(SByteCnt);
    return (err);
  }

  WORD *os, *bc;
  DWORD *dwp;
  WORD  *cp;
  char  *cccp;
  unsigned long lOs, lBc;

  p =(WORD*)malloc(width * height * iImageType);

  os =(WORD*)SDataOffset;
  bc =(WORD*)SByteCnt;
  cp = p;
  for (short j = 0; j < SCnt; j++)      // read stripdata from file
  {
    if (SBytes == 4)                    // Pointer are DWORDs
    {
      dwp =(DWORD*)os;
      lOs = *dwp;
      dwp =(DWORD*)bc;
      lBc = *dwp;
    }
    else                               // Pointer are WORDs
    {
      lOs = *os;
      lBc = *bc;
    }
    if (SetFilePointer(hfread, lOs, 0, FILE_BEGIN) != lOs)
    {                                  // set file offset
      err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
      break;
    }

    ReadFile(hfread, cp, lBc, &read, 0);// read data
    // DWORD x = GetLastError();// winerror.h winbase.h

    if (read != lBc)                    // bytes read = bytes requested ?
    {
      err = PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_WRONGVALUE;
      break;
    }
    os++;                               // Increment pointer by 2 Byte
    bc++;
    if (SBytes == 4)                    // If DWORDs add another 2 Byte
    {
      os++;
      bc++;
    }
    // cp += (read>>1);
    cccp =(char*) cp;                   // Increment target by bytes read
    cccp += read;
    cp =(WORD*) cccp;
  }

  CloseHandle(hfread);                  // close file

  if (err != PCO_NOERROR)
  {                                    // release mem in case of an error
    if (p != NULL)
    {
      free(p);
      p = NULL;
    }
  }

  if (SDataOffset != NULL)              // release data
    free(SDataOffset);
  if (SByteCnt != NULL)
    free(SByteCnt);
  os = NULL;
  bc = NULL;
  cp = NULL;

  if ((!((width != 0) &&(height != 0))) ||(err))
  {
    strBild->iXRes = 0;
    strBild->iYRes = 0;
    return (PCO_ERROR_CAMWARE + PCO_ERROR_APPLICATION + PCO_ERROR_NOFILE);
  }

  strBild->iXRes = width;               // forward image size
  strBild->iYRes = height;

  if (iImageType == 1)                  // 8bit BW in 16bit raw data
  {
    short *spl;
    char  *cpl;
    long k, l;

    spl =(short*)im;                  // target data
    cpl =(char*) p;                   // file data

    for (k = 0; k < height; k++)
    {
      for (l = 0; l < width; l++)
      {
        *spl = (unsigned char)*cpl;   // forward 8bit directly
        *spl <<= ishift;

        spl++;                        // next Pixel
        cpl++;
      }
    }
  }

  if (iImageType == 2)                // copy 16bit directly
  {
    long m;
    WORD *o;

    o = p;
    memset(im, 0, width * height *2); // set target data to 0
    for (m = 0; m < height; m++)      // copy line by line as bytes are dropped
    {                                 // to meet 4 byte limits
      if(bmacformat)
      {
        for (int ix = 0; ix < width; ix++)
        {
          ush = o[ix];
          mswab((byte*)&ush, (byte*)&ush2, sizeof(ush2));
          ((WORD*)im)[ix + m * width] = ush2;
        }
      }
      else
        memcpy((WORD*)im + m * width, (const void*)o, width * 2);

      o += width;                      // next line
    }
  }

  if ((iImageType == 3) ||(iImageType == 4))// filter RGB to raw data
  {
    long k, l;
    bool bRed = TRUE;
    bool bTog = TRUE;
    unsigned char *cpl;
    unsigned short *spl;

    cpl =(unsigned char*) p;                   // Imagedata from file
    spl =(unsigned short*) im;                 // Imagedata target
    for (k = 0; k < height; k++)
    {
      bTog = TRUE;
      if (bRed)                         // red line
      {
        for (l = 0; l < width; l++)
        {
          *spl = (unsigned char)*cpl;   // get 8bit data
          *spl <<= ishift;
          if (bTog)                     // red pixel
          {
            bTog = FALSE;
            cpl += 4;                  // next is green
          }
          else                         // green pixel
          {
            bTog = TRUE;
            cpl += 2;                  // next is red
          }
          if (iImageType == 4)          // skip 4. byte
            cpl++;
          spl++;
        } 
        if(bTog)
          cpl++;                        // prepare for blue line, next is green
        bRed = FALSE;
      }
      else                             // blue line
      {
        for (l = 0; l < width; l++)
        {
          *spl = (unsigned char)*cpl;
          *spl <<= ishift;
          if (bTog)                     // green pixel
          {
            bTog = FALSE;
            cpl += 4;                  // next is blue
          }
          else                         // blue pixel
          {
            bTog = TRUE;
            cpl += 2;                  // next is green
          }
          if (iImageType == 4)          // skip 4. byte
            cpl++;
          spl++;
        } 
        if(!bTog)
          cpl -= 2;                     // prepare for red line, next is red
        else
          cpl--;
        bRed = TRUE;
      }
    }
  }

  if(strBild->iVersion < FILEVERSION200)
  {
    strBild->iBWMin2 = strBild->iBWMin;                   // Lut bw min
    strBild->iBWMax2 = strBild->iBWMax;                   // Lut bw max
    strBild->iBWLut2 = strBild->iBWLut;                   // Lut lin log
    strBild->iRMin2 = strBild->iRMin;                    // red min
    strBild->iRMax2 = strBild->iRMax;                    // red max
    strBild->iGMin2 = strBild->iGMin;                    // green min
    strBild->iGMax2 = strBild->iGMax;                    // green max
    strBild->iBMin2 = strBild->iBMin;                    // blue min
    strBild->iBMax2 = strBild->iBMax;                    // blue max
    strBild->iColLut2 = strBild->iColLut;                  // Lut lin log color
    strBild->bAlignUpper = FALSE;
  }

  if(strBild->iVersion < FILEVERSION300)
  {
    strBild->dGammaLut = 1.0;                 // Gamma value b/w
    strBild->dGammaLutC = 1.0;                // Gamma value color
    strBild->dGammaLut2 = 1.0;                // Gamma value b/w 2
    strBild->dGammaLutC2 = 1.0;               // Gamma value color 2
  }

  if(strBild->iVersion < FILEVERSION301)
  {
    strBild->iBitRes = 14;                    // Assume 14bit resolution
  }

  if(strBild->iVersion < FILEVERSION302)
    strBild->dSaturation = 100;

  if((iImageType == 2) && (strBild->bAlignUpper) && (iNoShifting == 0))
  {
    Shift((WORD*)im, strBild->iXRes * strBild->iYRes, FALSE, 16 - strBild->iBitRes);
  }

  if (p != NULL)
    free(p);
  return (err);
}

