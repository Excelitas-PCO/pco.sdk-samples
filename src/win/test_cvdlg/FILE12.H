#ifndef FILE12_H
#define FILE12_H

#include <windows.h>
#include <stdio.h>
#include <pco_filestruct.h>

#define CEXP __declspec(dllexport)

#define dword DWORD
#define word WORD
#define byte BYTE
//#define NOFILE -1 
#define NOMEMR -2 
#define WRONGVAL -3
#define NOERR 0 

#define FILEISOK        1
#define FILEISMACFORMAT 2



int read_tif(char *filename, Bild* strBild, int iNoShifting);//, int*);
int getsize_tif(char *filename, int*, int*, bool*);
#endif