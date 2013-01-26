//  Created by Nolan Miller on 10/25/12.
//  Copyright (c) 2012 Nolan Miller. All rights reserved.

//IMPORTATION
#define BMPREADER

#ifndef STDIO
#include <stdio.h>
#define STDIO
#endif

#ifndef STDLIB
#include <stdlib.h>
#define STDLIB
#endif

#define READ4BYTESFROM(X) fgetc((X)) | fgetc((X))<<8 | fgetc((X))<<16 | fgetc((X))<<24
#define READ2BYTESFROM(X) fgetc((X)) | fgetc((X))<<8

#define RED 0
#define BLUE 1
#define YELLOW 2
#define VAL 3
#define FOURTH 4

//STRUCTURE DEFINITIONS
typedef struct pix {
    int red;
    int blue;
    int yellow;
    int fourth;
    unsigned int arrayval;
}pixle;

typedef struct bmp {
    // bmp file header
    char signature[2];      //0x0   (2 bytes)
    unsigned int size;      //0x2   (4 bytes)
    unsigned int imgaddress;//0xA   (4 bytes)
    // second header
    unsigned int headersize;//0xE   (4 bytes)
    int width;              //0x12  (4 bytes)
    int height;             //0x16  (4 bytes)
    short int bitsperpixle; //0x1C  (2 bytes)
    //data
    //some sort of palate data...
    pixle** pixlearray;
    // pixlearray[width][height]
}bmpfile;

// FUNCTION DEFINITIONS AND DECLARATIONS
int readinbmp(FILE*,bmpfile*);
void printbmp(bmpfile*,int);
void reducerange(bmpfile*);
void setcolors(bmpfile*);
int writeBMP(bmpfile*,FILE*);
int initBMP_w_h(bmpfile*,int,int,int);
int downsample(bmpfile*,bmpfile*);
int print_report(bmpfile*);

//needs debugging
int setvalue(bmpfile*);
int invert(bmpfile*);
//need to support
int treshholdfilter(bmpfile*,pixle);
int dalloc(bmpfile*);
//internal function...
int uintbinprint(unsigned int, FILE*);
unsigned int color16to32(unsigned int);

int invert(bmpfile* bmp){
    int i,ii;
    for(i=0;i<bmp->width;i++){
        for(ii=0;ii<bmp->height;ii++){
            bmp->pixlearray[i][ii].arrayval = 0xffffffff - bmp->pixlearray[i][ii].arrayval;
        }
    }
    setcolors(bmp);
    return 0;

}

int print_report(bmpfile* file){

    printf("length: %u\n",file->size);
    printf("offset: %u\n",file->imgaddress);
    printf("size:   %i x %i\n",file->height,file->width);
    printf("bpp:    %hi\n",file->bitsperpixle);

    //    reducerange(&filein);
    setcolors(file);
    printf("blue:\n");
    printbmp(file,BLUE);
    printf("yellow:\n");
    printbmp(file,YELLOW);
    printf("red:\n");
    printbmp(file,RED);
    printf("fourth\n");
    printbmp(file,FOURTH);
    printbmp(file,VAL);

    return 0;
}

int uintbinprint(unsigned int num, FILE* file){
    fputc((num)&0xff,file);
    fputc((num>>8 ),file);
    fputc((num>>16),file);
    fputc((num>>24),file);
    return 0;
}
unsigned int color16to32(unsigned int in){
    unsigned int ret= (((in&0x0000000f))<<4)|(((in&0x000000f0))<<8)|(((in&0x00000f00))<<12)|(((in&0x0000f000))<<16);
    ret |= ret>>4;
    return ret;
}

int initBMP_w_h(bmpfile* bmp, int width,int height,int bpp){
    int i,ii;
    if(bpp != 32 && bpp!= 16 && bpp != 24){
        bpp = 32;
        printf("bpp given not supported... defaults to 32");
    }

    bmp->signature[0] = 'B';
    bmp->signature[1] = 'M';
    bmp->width = width;
    bmp->height= height;
    bmp->size = height*width*((bpp+7)/8)+40+14;
    bmp->headersize = 40;
    bmp->bitsperpixle =bpp;
    bmp->imgaddress = 40+14;
    bmp->pixlearray = malloc(sizeof(pixle*)*width);
    for(i=0;i<width;i++){
        bmp->pixlearray[i] = malloc(sizeof(pixle)*height);
        for(ii=0;ii<height;ii++){
            bmp->pixlearray[i][ii].arrayval = 0;
            bmp->pixlearray[i][ii].red = 0;
            bmp->pixlearray[i][ii].blue = 0;
            bmp->pixlearray[i][ii].yellow = 0;
            bmp->pixlearray[i][ii].fourth = 0;
        }
    }
    return 0;
}

int dalloc(bmpfile* bmp){
    int i,ii;
    for(i=0;i<abs(bmp->width);i++){
        free(bmp->pixlearray[i]);
    }
    free(bmp->pixlearray);

}

int downsample(bmpfile* in, bmpfile* out){
    int i,ii;
    int j,jj;
    setcolors(in);
    if(abs(out->height)>abs(in->height)|abs(out->width)>abs(in->width)){
        printf("FAILURE IN RESHAPING");
        return -10;
    }
    pixle sum;
    unsigned long int sred,sblue,syellow,sfourth;

    int dw,dh;
    dw = abs(in->width/out->width);
    dh = abs(in->height/out->height);
    for(i=0;i<(abs(in->width));i+=dw){
        for(ii=0;ii<(abs(in->height));ii+=dh){
            sred = sblue = syellow = sfourth = 0;
            for(j=0;j<dw;j++){
                for(jj=0;jj<dh;jj++){
                    sred += in->pixlearray[i+j][ii+jj].red;
                    sblue += in->pixlearray[i+j][ii+jj].blue;
                    syellow += in->pixlearray[i+j][ii+jj].yellow;
                    sfourth += in->pixlearray[i+j][ii+jj].fourth;
                }
            }
            printf("%lu,%lu,%lu,%lu,%i\n",sred,sblue,syellow,sfourth,dw*dh);
            out->pixlearray[i/dw][ii/dh].red = sred/(dh*dw);
            out->pixlearray[i/dw][ii/dh].blue = sblue/(dh*dw);
            out->pixlearray[i/dw][ii/dh].yellow = syellow/(dh*dw);
            out->pixlearray[i/dw][ii/dh].fourth = sfourth/(dh*dw);
        }
    }
    setvalue(out);
    return 0;
}

int writeBMP(bmpfile* out, FILE*file){
int i,ii;

//first header
    fputc(out->signature[0],file);
    fputc(out->signature[1],file);
    printf("\n\n--------------------debugger---------------------\n");
    printf("size:%u\n",out->size);
    printf("-----------------------end-----------------------\n\n");
    uintbinprint(out->size,file);
//should instead rewind and write ending byte index here...
    uintbinprint((unsigned int)0,file);
    uintbinprint(out->imgaddress,file);
//second header
    uintbinprint((unsigned int)40,file);
    uintbinprint((int)(out->width),file);
    uintbinprint((int)(out->height),file);
    uintbinprint((unsigned int)(0x00000001|out->bitsperpixle<<16),file);
    uintbinprint((unsigned int) 0,file);
// this one is wrong... It should be the size of the bitmap but we don't maintain that information so it will have to be calculated (more carefully for unsopported dtyps)
// ... looks like most renderers don't even use this information so letting it be 0 works too...
    uintbinprint(abs(out->width*out->height*out->bitsperpixle),file);
//horizontal res.
    uintbinprint(out->width*100,file);
//vertical res.
    uintbinprint(out->height*100,file);
    uintbinprint((unsigned int)0,file);
    uintbinprint((unsigned int)0,file);
// end of secondary header

    while(ftell(file)<out->imgaddress){
        fputc((char)0,file);
    }
    if(out->bitsperpixle == 32){
        for(i=0;i<abs(out->width);i++){
            for(ii=0;ii<abs(out->height);ii++){
                uintbinprint(out->pixlearray[i][ii].arrayval,file);
            }
        }
    }
    else if(out->bitsperpixle == 16){
        for(i=0;i<abs(out->width);i++){
            for(ii=0;ii<abs(out->height);ii++){
                //need to watch the bitshift here... it could be unneccissary or in the wrong direction...
// this is probably so wrong its painful. Should seriously consider converting everything to 32 bit color. It would be so much more robust
                fputc((char)((out->pixlearray[i][ii].arrayval)&0xff),file);
                fputc((char)((out->pixlearray[i][ii].arrayval>>8 )&0xff),file);
            }
        }
    }

}

int readinbmp(FILE* fin,bmpfile* filein){
    int i,ii;
    filein->signature[0] = fgetc(fin);
    filein->signature[1] = fgetc(fin);
    filein->size =READ4BYTESFROM(fin);
    
    fseek(fin, 0XA, SEEK_SET);
    filein->imgaddress = READ4BYTESFROM(fin);
    
    filein->headersize = READ2BYTESFROM(fin);
    
    
    //      BITMAPINFOHEADER (nearly ubiquitous)
    if (filein->headersize == 40) {
        fseek(fin, 0x12, SEEK_SET);
        filein->width =      READ4BYTESFROM(fin);
        filein->height =     READ4BYTESFROM(fin);
        
        fseek(fin, 0x1C, SEEK_SET);
        filein->bitsperpixle = READ2BYTESFROM(fin);
    }
    
    //      OS/2 BITMAPCOREHEADER (sometimes used)
    else if (filein->headersize == 12) {
        fseek(fin, 0x12, SEEK_SET);
        filein->width =      (unsigned int)READ2BYTESFROM(fin);
        filein->height =     (unsigned int)READ2BYTESFROM(fin);
        
        fseek(fin, 0x18, SEEK_SET);
        filein->bitsperpixle = READ2BYTESFROM(fin);
    }
    
    else if (filein->headersize == 64){
        printf("BITMAPCOREHEADER2 not supported");
        printf("\nData aquisition failed.");
        return 1;
    }
    else {
        printf("Header size:%i not supported.",filein->headersize);
        return 2;
    }
    
    fseek(fin, filein->imgaddress, SEEK_SET);
    if (filein->bitsperpixle == 32) {
        //4 bytes per pixel
        filein->pixlearray = (pixle**)malloc(sizeof(pixle*)*abs(filein->width));
        for (i =0; i<abs(filein->width); i++) {
            filein->pixlearray[i] = (pixle*)malloc(sizeof(pixle)*abs(filein->height));
            for (ii=0; ii<abs(filein->height); ii++) {
                filein->pixlearray[i][ii].arrayval = READ4BYTESFROM(fin);
            }
        }
    }
    
    else if (filein->bitsperpixle == 16) {
        //2 bytes per pixel
        filein->pixlearray = (pixle**)malloc(sizeof(pixle*)*abs(filein->width));
        for (i =0; i<abs(filein->width); i++) {
            filein->pixlearray[i] = (pixle*)malloc(sizeof(pixle)*abs(filein->height));
            for (ii=0; ii<abs(filein->height); ii++) {
                filein->pixlearray[i][ii].arrayval = color16to32((READ2BYTESFROM(fin)));

            }
        }
        filein->bitsperpixle = 32;
    }
    else if (filein->bitsperpixle ==8) {
        //1 byte per pixel
        filein->pixlearray = (pixle**)malloc(sizeof(pixle*)*abs(filein->width));
        for (i =0; i<abs(filein->width); i++) {
            filein->pixlearray[i] = (pixle*)malloc(sizeof(pixle)*abs(filein->height));
            for (ii=0; ii<abs(filein->height); ii++) {
                filein->pixlearray[i][ii].arrayval = fgetc(fin);
            }
        }
    }
    else {
        printf("given BPP not supported");
        return 3;
    }
    
    return 0;
}

int setvalue(bmpfile* bmp){
    int i,ii;

//this doesn't work right I THINK AHSDLFKJASDFALSDKFJASDFLAKJSDFALSDKFJASDLKFJAFJALSDKFJALKFJFAUCKYOULAKSJDFLKAJSDF
    if(bmp->bitsperpixle == 32 || bmp->bitsperpixle == 24|| bmp->bitsperpixle == 16){

        for(i=0;i<bmp->width;i++){
            for(ii=0;ii<bmp->height;ii++){
                bmp->pixlearray[i][ii].arrayval = (((bmp->pixlearray[i][ii].red&0xff)))|(((bmp->pixlearray[i][ii].blue&0xff)<<(8)))|(((bmp->pixlearray[i][ii].yellow&0xff)<<(16)))|(((bmp->pixlearray[i][ii].fourth&0xff)<<(24)));
            }
        }
    }   
    else
        printf("failure to set value: unsupported bpp");
    return 0;
}
void setcolors(bmpfile* bmp){
    int i,ii;
    if(bmp->bitsperpixle ==32||bmp->bitsperpixle == 24){
        for(i=0;i<abs(bmp->width);i++){
            for(ii=0;ii<abs(bmp->height);ii++){
                bmp->pixlearray[i][ii].red = ((bmp->pixlearray[i][ii].arrayval) & (0xFF<< 0));
                bmp->pixlearray[i][ii].blue = ((bmp->pixlearray[i][ii].arrayval) & (0xFF<< 8))>>8;
                bmp->pixlearray[i][ii].yellow = ((bmp->pixlearray[i][ii].arrayval) & (0xFF<< 16))>>16;
                bmp->pixlearray[i][ii].fourth = ((bmp->pixlearray[i][ii].arrayval) & (0xFF<< 24))>>24;
            }
        }
    }
    else if(bmp->bitsperpixle == 16){
        for(i=0;i<abs(bmp->width);i++){
            for(ii=0;ii<abs(bmp->height);ii++){
                bmp->pixlearray[i][ii].red = (bmp->pixlearray[i][ii].arrayval) & (0xF<< 0);
                bmp->pixlearray[i][ii].blue = ((bmp->pixlearray[i][ii].arrayval) & (0xF<< 4))>>4;
                bmp->pixlearray[i][ii].yellow = ((bmp->pixlearray[i][ii].arrayval) & (0xF<< 8))>>8;
                bmp->pixlearray[i][ii].fourth = ((bmp->pixlearray[i][ii].arrayval) & (0xF<< 12))>>12;
            }
        }
    }
    else
        printf("color mapping not yet supported.");
}

void printbmp(bmpfile* bmp,int type){
    int i,ii;
    for(i = 0; i<abs(bmp->height);i++){
        for (ii = 0; ii<abs(bmp->width); ii++) {
            switch(type){
                case RED:
                    printf("%x ",bmp->pixlearray[i][ii].red);
                    break;
                case BLUE:
                    printf("%x ",bmp->pixlearray[i][ii].blue);
                    break;
                case YELLOW:
                    printf("%x ",bmp->pixlearray[i][ii].yellow);
                    break;
                case VAL:
                    printf("%x ",bmp->pixlearray[i][ii].arrayval);
                    break;
                case FOURTH:
                    printf("%x ",bmp->pixlearray[i][ii].fourth);
                    break;
                default:
                    printf("failure");
                    break;
            }
        }
        printf("\n");
    }
}

void reducerange(bmpfile* bmp){
    unsigned int old;
    int k =0;
    int i,ii;
    int l,ll;
    for(i = 0; i<abs(bmp->height);i++){
        for (ii = 0; ii<abs(bmp->width); ii++) {
            if ((old = bmp->pixlearray[i][ii].arrayval)>abs(bmp->height*bmp->width)) {
                k++;
                for(l = i; l<abs(bmp->height);l++){
                    for (ll = ii; ll<abs(bmp->width); ll++) {
                        if (bmp->pixlearray[l][ll].arrayval == old) {
                            bmp->pixlearray[l][ll].arrayval = k;
                        }
                    }
                }
            }
        }
    }
}
