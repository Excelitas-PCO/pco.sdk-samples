//-----------------------------------------------------------------//
// Name        | file12.cpp                  | Type: (*) source    //
//-------------------------------------------|       ( ) header    //
// Project     | pco.camera simple examples  |       ( ) others    //
//-----------------------------------------------------------------//
// Platform    | WINDOWS                                           //
//-----------------------------------------------------------------//
// Environment | Microsoft VisualStudio                            //
//             |                                                   //
//-----------------------------------------------------------------//
// Purpose     | Source for image file read/write                  //
//-----------------------------------------------------------------//
// Author      | MBL, PCO AG                                       //
//-----------------------------------------------------------------//
// Revision    | rev. 1.30 rel. 1.30                               //
//-----------------------------------------------------------------//
// Notes       | supported formats:                                //
//             |  simple b16         read/write                    //
//             |  tif 16Bit          write                         //
//             |  tif 8Bit           write                         //
//             |  bmp                write                         //
//             |  fits               write                         //
//             |                                                   //
//-----------------------------------------------------------------//
// (c) 2016 PCO AG * Donaupark 11 *                                //
// D-93309  Kelheim / Germany * Phone: +49 (0)9441 / 2005-0 *      //
// Fax: +49 (0)9441 / 2005-20 * Email: pco@excelitas.com           //
//-----------------------------------------------------------------//

#ifdef LINUX
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>

#include <unistd.h>
#include "../common/defs.h"
#include "../SrcCommon/Include/pco/v00.05/PCO_err.h"

#endif

#ifdef WIN32

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

#include <io.h>
#include <fcntl.h>

#include "pco_err.h"

#define open _open
#define read _read
#define write _write
#endif

#include "file12.h"

#define FPVERS "1.30"
#define FPVER  130

char pcotiff_text[70]="";

int GET_B16_FILEPARAMS(char *filename,int *width,int *height,int *colormode)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfread;
  int e;

  cptr=(unsigned char *)malloc(200);
  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;


  hfread = open(filename,O_RDONLY|O_BINARY);
  if(hfread == -1)
  {
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  e=read(hfread,cptr,200);

  if(e<200)
  {
   _close(hfread);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  if((*c1 != 'P')||(*(c1+1) != 'C')||(*(c1+2) != 'O')||(*(c1+3) != '-'))
  if(e<200)
  {
   _close(hfread);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

/* read FILEHEADER width and height
*/
  c1=cptr+12;                          /*Width*/
  b1=(unsigned int*)c1;
  *width   = *b1;

  c1=cptr+16;                          /*Height*/
  b1=(unsigned int*)c1;
  *height  = *b1;

  c1=cptr+20;

  if(*c1!='C')
  {
   switch(*b1)
   {
      case 0:
      case 1:
       *colormode  = *b1;
       break;

      case -1:
       b1++;
       *colormode  = *b1;
       break;

      default:
       *colormode=0;
       break;
   }
  }
  else
  {
    c1=cptr+35;
    if(*c1=='5')
     *colormode=1;
    else
     *colormode=0;
  }

  _close(hfread);
  free(cptr);

  return PCO_NOERROR;
}

int read_b16(char *filename, void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfread;
  int e,z;
  int fsize;
  int headerl;
  int err;

  cptr=(unsigned char *)malloc(200);
  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;

  hfread = open(filename,O_RDONLY|O_BINARY);
  if(hfread == -1)
  {
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  e=read(hfread,cptr,200);

  if(e<200)
  {
   _close(hfread);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  if((*c1 != 'P')||(*(c1+1) != 'C')||(*(c1+2) != 'O')||(*(c1+3) != '-'))
  {
   _close(hfread);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

/*read FILEHEADER*/
  c1=cptr+4;                  /* filesize */
  b1=(unsigned int*)c1;
  fsize=*b1;

  c1=cptr+8;                  /* headerlength */
  b1=(unsigned int*)c1;
  headerl =*b1;

  z=fsize-headerl;

  err=PCO_NOERROR;
  
/* read data */
  _lseek(hfread,headerl,SEEK_SET);

  e=read(hfread,buf,z);

  if(e<z)
   err= PCO_ERROR_NOFILE;
  
  _close(hfread);
  free(cptr);
  return err;
}

int store_b16(char *filename,int width,int height,
              int colormode,void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfstore;
  int e,z;
  int headerl;

//  printf("store %s w%d h%d size %d buf%p\n",filename,width,height,width*height*2,buf);

  cptr=(unsigned char *)malloc(2000);

  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC|O_BINARY,666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   printf("\ncreat failed\n");
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  headerl = 128;

  c1=cptr;
  *c1++ = 'P';           /* Begin PCO-Header PCO- */
  *c1++ = 'C';
  *c1++ = 'O';
  *c1++ = '-';

  b1 = (unsigned int*)c1;

/* Data for Header */
  *b1++ = (width*height*2)+headerl;
  *b1++ = headerl;
  *b1++ = width;
  *b1++ = height;
  *b1++ = 0;

  *b1++ = colormode;

/* Fill Header */
  c1=(unsigned char *)b1;
  for(;c1<cptr+128;)
   *c1++=0;

  z=headerl;
  e=write(hfstore,(void *)cptr,z);
  if(e==-1)
  {
   printf("\nwrite header failed\n");
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height*2;

  e=write(hfstore,(void *)buf,z);
  if(e == -1)
  {
   printf("\nwrite buffer failed\n");
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }


  _close(hfstore);
  free(cptr);
  return PCO_NOERROR;
}


int store_tiff(char *filename,int width,int height,
               int colormode,void *bufadr,char *apptext)
{
  unsigned short *cptr;
  unsigned short *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;

//  printf("store %s w%d h%d size %d buf%p\n",filename,width,height,width*height*2,bufadr);

  cptr=(unsigned short *)malloc(65536);

  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC|O_BINARY,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  int slen,txtlen;
  char *ch;
  slen=(int)strlen(apptext);
  txtlen=slen+1;
  txtlen=(txtlen/16)*16+16;

  c1=cptr;
  *c1++ = 0x4949;           /* Begin TIFF-Header II */
  *c1++ = 0x002A;

  *c1++ = 0x0010;           /* Pointer to IFD  */
  *c1++ = 0;


  *c1++ = 0;
  *c1++ = 0;
  *c1 = 0;

/* create  IFD */
  c1=cptr+8;

  *c1++ = 0x000F;             /* Entry Count */

  *c1++ = 0x00FE;             /* NewSubfileType */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000000;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0100;             /* ImageWidth */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = width;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0101;             /* ImageHeight */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = height;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0102;             /* BitsPerPixel */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0010;             /* 16 */
  *c1++ = 0x0000;

  *c1++ = 0x0103;             /* Compression */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0106;             /* PhotometricInterpretation */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;


  *c1++ = 0x0111;             /* StripOffset */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
//@a  *b1++ = height/4;           /* 4 Zeilen pro */
  *b1++ = height;             /* 1 Zeilen pro */
  *b1++ = 0x00E0;              /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x0115;             /* SamplePerPixel */
  *c1++ = 0x0003;             /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0116;             /* RowsPerStrip */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000001;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0117;              /* StripByteCounts */
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
//@a  *b1++ = height/4;
  *b1++ = height;
//@a  *b1++ = 0x0E0+height;        /* pointer */
  *b1++ = 0x0E0+height*4;        /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011A;              /* X-Resolution */
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
//@a  *b1++ = 0x0E0+height*2;      /* pointer */
  *b1++ = 0x0E0+height*8;      /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011B;              /* Y-Resolution */
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
//@a  *b1++ = 0x0E0+height*2+8;    /* pointer */
  *b1++ = 0x0E0+height*8+8;    /* pointer */
  c1 = (unsigned short *)b1;

  *c1++ = 0x011C;              /* PlanarConfiguration */
  *c1++ = 0x0003;              /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0128;              /* ResolutionUnit */
  *c1++ = 0x0003;              /* SHORT */
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              /* 1 */
  *c1++ = 0x0000;

  *c1++ = 0x0131;              /* Software */
  *c1++ = 0x0002;
  b1 = (unsigned int *)c1;
//  *b1++ = 0x00000046;
  *b1++=slen+1;
  *b1++ = 0x0E0+height*8+16;   /* pointer */


//fill with 0 until strips at 0xE0
  c1 = (unsigned short *)b1;
  for(;c1<cptr+0xE0/2;)
   *c1++ = 0;                   /* ende */

/* beginn der stripadressen 224byte vom Fileanfang */
//@a  z=0x0E0+height*2+16+70;     /*  70 is textlength */
  z=0x0E0+height*8+16+txtlen; // 70;     /*  70 is textlength */

  c1=cptr+0x70;  //0x70=0xE0/2
  b1 = (unsigned int *)c1;
//@a  for(x=0;x<height/4;x++)
//@a	*b1++=z+x*width*8;
  for(x=0;x<height;x++)
   *b1++=z+x*width*2;

//@a  for(x=0;x<height/4;x++)
//@a   *b1++=width*8;
  for(x=0;x<height;x++)
   *b1++=width*2;

  *b1++=0x00000004;
  *b1++=width;
  *b1++=0x00000004;
  *b1++=height;

  strcpy((char*)b1,apptext);
  ch=(char*)b1;
  ch+=slen;
  for(;ch<(char*)cptr+z;)
   *ch++=0;

  headerl = (int)((ch-(char*)cptr));

  e=write(hfstore,(void *)cptr,headerl);
  if(e== -1)
  {
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height*2;
  e=write(hfstore,(void *)bufadr,z);
  if(e==-1)
  {
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  _close(hfstore);
  free(cptr);
  return PCO_NOERROR;
}

int store_tiff(char *filename,int width,int height,
               int colormode,void *bufadr)
{
  if(strlen(pcotiff_text)==0)
  {
   int x;
   sprintf(pcotiff_text,"PCO 16bit File-Program %s Copyright (C)2016 PCO ",FPVERS);
   if(colormode==0)
    strcat(pcotiff_text,"Version: B/W    ");
   else
    strcat(pcotiff_text,"Version: COLOR  ");
   for(x=(int)strlen(pcotiff_text);x<70-1;x++)
    pcotiff_text[x]=0x20;
   pcotiff_text[x]=0;
  }
  return store_tiff(filename,width,height,colormode,bufadr,pcotiff_text);
}


static void chbytes(int W,int H,unsigned short* array)
{
  int x=0,y=0,pos=0;
  unsigned char buf1=0,buf2=0;
  for (y=0;y<H;y++)
  {
    for (x=0;x<W;x++)
    {
      pos=y*W+x;
      buf1=array[pos]%256;
      buf2=array[pos]/256;
      array[pos]=buf1*256;
      array[pos]+=buf2;
    }
  }
}


/********************************************************************************************************************
*                                            FITS-Datei schreiben                                                   *
*********************************************************************************************************************/

#define FITSBLOCK 2880

int store_fits_exp(char *filename,int W, int H, void *_img_data,int _exp_time_ms)
{
  int z=0,e=0,i=0;
  FILE *fitsfile;
  char *headerline,*buffer;
  fitsfile=fopen(filename,"w+");
  if (fitsfile==NULL)
  {
    printf("Cannot open %s !\n",filename);
    remove(filename);
    return PCO_ERROR_NOFILE;
  }
  chbytes(W,H,(unsigned short*)_img_data);

  headerline =(char *)malloc(2881);

  if(headerline==NULL)
   return PCO_ERROR_NOMEMORY;

  buffer=(char *)malloc(81);
  if(buffer==NULL)
  {
   free(headerline);
   return PCO_ERROR_NOMEMORY;
  }

  memset(headerline,0,2881);//to avoid confusion
  //now we can insert the keywords
  //don't forget to provide a line length of 80 characters
  sprintf(buffer,"SIMPLE  =                    T                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"BITPIX  =                   16                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS   =                    2                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS1  =                 %4d                                                  ",W);
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS2  =                 %4d                                                  ",H);
  strcat(headerline,buffer);
 //end of header:
  sprintf(buffer,"END                                                                             ");
  strcat(headerline,buffer);
  for (i=(int)strlen(headerline);i<FITSBLOCK;i++)
  {
    sprintf(buffer," ");
    strcat(headerline,buffer);
  }

  free(buffer);

  //write the header to fitsfile
  e=0;
  e=fputs(headerline,fitsfile);
  free(headerline);
  if (e==EOF)
  {
    fclose(fitsfile);
    printf("\nCannot write to file %s!\n",filename);
    return PCO_ERROR_NOFILE;
  }

  e=0;
  //write the data to 'fitsfile'
  if(fwrite(_img_data,sizeof(unsigned short),W*H,fitsfile)==0)
  {
   fclose(fitsfile);
   printf("\nCannot write to file %s!\n",filename);
   return PCO_ERROR_NOFILE;
  }

  //we have to fill up the last 2880 byte block with blanks
  z=W*H;
  z=(H+4)*W*2;
  z%=FITSBLOCK;
  z=FITSBLOCK-z;
  for (i=0;i<z;i++)
  {
   fwrite(" ",sizeof(char),1,fitsfile);
  }

  fclose(fitsfile);
  chbytes(W,H,(unsigned short *)_img_data);
  return PCO_NOERROR;
}

int store_fits(char *filename,int W, int H,int colormode, void *_img_data)
{
  int z=0,e=0,i=0;
  FILE *fitsfile;
  char *headerline,*buffer;
  fitsfile=fopen(filename,"w+");
  if (fitsfile==NULL)
  {
    printf("Cannot open %s !\n",filename);
    remove(filename);
    return PCO_ERROR_NOFILE;
  }
  chbytes(W,H,(unsigned short *)_img_data);
  headerline =(char *)malloc(2881);
  if(headerline==NULL)
   return PCO_ERROR_NOMEMORY;

  buffer=(char *)malloc(81);
  if(buffer==NULL)
  {
   free(headerline);
   return PCO_ERROR_NOMEMORY;
  }

  memset(headerline,0,2881);//to avoid confusion
  //now we can insert the keywords
  //don't forget to provide a line length of 80 characters
  sprintf(buffer,"SIMPLE  =                    T                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"BITPIX  =                   16                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS   =                    2                                                  ");
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS1  =                 %4d                                                  ",W);
  strcat(headerline,buffer);
  sprintf(buffer,"NAXIS2  =                 %4d                                                  ",H);
  strcat(headerline,buffer);
 //end of header:
  sprintf(buffer,"END                                                                             ");
  strcat(headerline,buffer);
  for (i=(int)strlen(headerline);i<FITSBLOCK;i++)
  {
    sprintf(buffer," ");
    strcat(headerline,buffer);
  }

  free(buffer);

  //write the header to fitsfile
  e=0;
  e=fputs(headerline,fitsfile);
  free(headerline);
  if (e==EOF)
  {
    fclose(fitsfile);
    printf("\nCannot write to file %s!\n",filename);
    return PCO_ERROR_NOFILE;
  }

  e=0;
  //write the data to 'fitsfile'
  if(fwrite(_img_data,sizeof(unsigned short),W*H,fitsfile)==0)
  {
    fclose(fitsfile);
    printf("\nCannot write to file %s!\n",filename);
    return PCO_ERROR_NOFILE;
  }

  //we have to fill up the last 2880 byte block with blanks
  z=W*H;
  z=(H+4)*W*2;
  z%=FITSBLOCK;
  z=FITSBLOCK-z;
  for (i=0;i<z;i++)
  {
    fwrite(" ",sizeof(char),1,fitsfile);
  }

  fclose(fitsfile);
  chbytes(W,H,(unsigned short *)_img_data);

  return PCO_NOERROR;
}

int store_bmp(char *filename,int width,int height,
              int colormode,void *buf)
{
  unsigned char *cptr;
  unsigned char *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;


  if(colormode>2)
   return PCO_ERROR_WRONGVALUE;

  cptr=(unsigned char *)malloc(2000L);

  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;

//  printf("store %s w%d h%d size %d buf%p\n",filename,width,height,width*height,buf);

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH); |O_SYNC
  if(hfstore == -1)
  {
   printf("\ncreat failed\n");
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

//calculate size of LUT-Table
  if(colormode==0)
   x=256;
  else
   x=0;

  headerl=x*4 + 0x28 + 0x0A;

  c1=cptr;
  *c1++ = 'B';           //Begin BMP-Header BM
  *c1++ = 'M';

  b1 = (unsigned int *)c1;

// Data for header
  if(colormode==0)
   *b1++ = width*height+headerl;
  if(colormode==1)
   *b1++ = width*height*3+headerl;
  if(colormode==2)
   *b1++ = width*height*4+headerl;
  *b1++ = 0L;
  *b1++ = headerl;
  *b1++ = 0x028;               //sizeof(BITMAPAPINFOHEADER);
  *b1++ = width;
  *b1++ = height;
  if(colormode==0)
   *b1++ = 1+(8<<16);
  if(colormode==1)
   *b1++ = 1+(24<<16);
  if(colormode==2)
   *b1++ = 1+(32<<16);
  *b1++ = 0;                  // BI_RGB;
  *b1++ = width*height;
  *b1++ = 0;
  *b1++ = 0;
  *b1++ = 0;
  *b1++ = 0;

  c1 = (unsigned char *)b1;

//write  LUT-Table
  if(colormode==0)
	for(z=0;z<x;z++)
	 {
	  *c1++ = (unsigned char)z;
	  *c1++ = (unsigned char)z;
	  *c1++ = (unsigned char)z;
	  *c1++ = 0;
	 }


  z=headerl;
  e=write(hfstore,(void *)cptr,z);
  if(e==-1)
  {
   printf("\nwrite header failed\n");
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

//write data to file
  switch(colormode)
   {
    case 0:
     z=width*height;
     break;

    case 1:
     z=width*height*3;
     break;

    case 2:
     z=width*height*4;
     break;

    default:
     break;
   }

  e=write(hfstore,(void *)buf,z);
  if(e==-1)
  {
   printf("\nwrite data failed\n");
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  _close(hfstore);
  free(cptr);
   return PCO_NOERROR;
}

int store_tif8bw(char *filename,int width,int height,
                int colormode,void *bufadr)
{
  unsigned short *cptr;
  unsigned short *c1;
  unsigned int *b1;
  int hfstore;
  int e,z,x;
  int headerl;

//  printf("store %s w%d h%d size %d buf%p\n",filename,width,height,width*height,bufadr);

  cptr=(unsigned short *)malloc(16000);

  if(cptr==NULL)
   return PCO_ERROR_NOMEMORY;

  hfstore = open(filename,O_CREAT|O_WRONLY|O_TRUNC,0666);//S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH);
  if(hfstore == -1)
  {
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  c1=cptr;
  *c1++ = 0x4949;           //Begin TIFF-Header II
  *c1++ = 0x002A;

  *c1++ = 0x0010;            //Pointer to IFD
  *c1++ = 0;


  *c1++ = 0;
  *c1++ = 0;
  *c1 = 0;

// create  IFD
  c1=cptr+8;                 //8 W�ter

  *c1++ = 0x000F;             //Entry Count

  *c1++ = 0x00FE;             //NewSubfileType
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000000;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0100;             //ImageWidth
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = width;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0101;             //ImageHeight
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = height;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0102;             //BitsPerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0008;             //8
  *c1++ = 0x0000;

  *c1++ = 0x0103;             //Compression
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1
  *c1++ = 0x0000;

  *c1++ = 0x0106;             //PhotometricInterpretation
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1 min is black
  *c1++ = 0x0000;

  *c1++ = 0x0111;             //StripOffset
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height/4;            //4 Zeilen pro
  *b1++ = 0x0E0;               //pointer
  c1 = (unsigned short *)b1;

  *c1++ = 0x0115;             //SamplePerPixel
  *c1++ = 0x0003;             //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;             //1
  *c1++ = 0x0000;

  *c1++ = 0x0116;             //RowsPerStrip
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x00000004;
  c1 = (unsigned short *)b1;

  *c1++ = 0x0117;              //StripByteCounts
  *c1++ = 0x0004;
  b1 = (unsigned int *)c1;
  *b1++ = height/4;
  *b1++ = 0x0E0+height;     //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011A;              //X-Resolution
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*2;   //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011B;              //Y-Resolution
  *c1++ = 0x0005;
  b1 = (unsigned int *)c1;
  *b1++ = 0x00000001;
  *b1++ = 0x0E0+height*2+8; //pointer;
  c1 = (unsigned short *)b1;

  *c1++ = 0x011C;              //PlanarConfiguration
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;

  *c1++ = 0x0128;              //ResolutionUnit
  *c1++ = 0x0003;              //SHORT
  *c1++ = 0x0001;
  *c1++ = 0x0000;
  *c1++ = 0x0001;              //1
  *c1++ = 0x0000;
  *c1++ = 0x0131;             //Software
  *c1++ = 0x0002;

  b1 = (unsigned int *)c1;
  *b1++ = 0x00000032;
  *b1++ = 0x0E0+height*2+16; //pointer;

  c1 = (unsigned short *)b1;
  *c1++ = 0;                  //ende
  *c1 = 0;

  //beginn der stripadressen 224byte vom Fileanfang
//@ver102a
  z=0x0E0+height*2+16+70;     //  70 is textlength
  c1=cptr+0x70;
  b1 = (unsigned int *)c1;
  for(x=0;x<height/4;x++)
	*b1++=z+x*width*4;

  for(x=0;x<height/4;x++)
	*b1++=width*4;

  *b1++=0x00000004;
  *b1++=width;
  *b1++=0x00000004;
  *b1++=height;

  c1 = (unsigned short *)b1;
                     //12345678901234567890123456789012345678901234567890
  sprintf((char *)c1, "SensiCam File-Program %s Copyright (C) 1997 PCO ",FPVERS);
//  sprintf((char *)c1,"SensiCam   Utility-Program Copyright (C) 1996 PCO ");
  c1+=25;
  if(colormode==0)
   sprintf((char *)c1,"Version: B/W    ");
  else
   sprintf((char *)c1,"Version: COLOR  ");
  c1+=8;

//@ver3.04  fehler beseitigt
  *c1++=0x2020;    // 4 x blank
  *c1++=0x2020;

  headerl = (int)((c1-cptr)*2);

  e=write(hfstore,(void *)cptr,headerl);
  if(e== -1)
  {
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  z=width*height;
  e=write(hfstore,(void *)bufadr,z);
  if(e==-1)
  {
   _close(hfstore);
   remove(filename);
   free(cptr);
   return PCO_ERROR_NOFILE;
  }

  _close(hfstore);
  free(cptr);
  return PCO_NOERROR;
}

