//Header for reading and writing image files

#ifndef FILE12_H
#define FILE12_H

#define FILEISOK        1
#define FILEISMACFORMAT 2

int read_b16(char *filename,void *bufadr);
int store_b16(char *filename,int width,int height,int colormode,void *bufadr);
int store_tiff(char *filename,int width,int height,int colormode,void *bufadr);
int store_bmp(char *filename,int width,int height,int colormode,void *bufadr);
int store_tif8bw(char *filename,int width,int height,int colormode,void *bufadr);
int store_fits(char *filename,int width,int height,int colormode,void *bufadr);
int store_fits_exp(char *filename,int W, int H, void *_img_data,int _exp_time_ms);

int GET_B16_FILEPARAMS(char *filename,int *width,int *height,int *colormode);

#endif

