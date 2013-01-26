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
#define GREEN 2
#define VAL 3
#define FOURTH 4

//STRUCTURE DEFINITIONS
class pixle{
    private:
    unsigned int arrayval;
    public:
    pixle();
    pixle(unsigned int);
    //accessors
    void setBlue(unsigned char blue);
    unsigned char getBlue()const;
    void setRed(unsigned char red);
    unsigned char getRed()const;
    void setGreen(unsigned char green);
    unsigned char getGreen()const;
    void setFourth(unsigned char fourth);
    unsigned char getFourth()const;
    void setArrayval(unsigned int inarrayval);
    void setArrayval(char inarrayval);
    unsigned int getArrayval()const;
    //overloads
    pixle operator+(const pixle&)const;

};
pixle pixle::operator+(const pixle& right)const{
    pixle ret = pixle();
    ret.setArrayval(unsigned(((this->arrayval)+(right.getArrayval()))/2));
    return ret;
}
pixle::pixle(){
    arrayval = 0u;
}
pixle::pixle(unsigned int inarrayval){
    //code to read in array val and set other colors toooooooooo
    arrayval = inarrayval;
}

//getters and setters
void pixle::setBlue(unsigned char blue){
//    arrayval |= (!(0xFF<<0))|(blue<<0);
    arrayval = (arrayval&(0xFFFFFF00))|((blue<<0)&(0x000000FF));
}
unsigned char pixle::getBlue()const{
    return (arrayval&(0xFF<<0));
}
void pixle::setGreen(unsigned char green){
    arrayval = (arrayval&(0xFFFF00FF))|((green<<8)&(0x0000FF00));
}
unsigned char pixle::getGreen()const{
    return ((arrayval&(0xFF<<8))>>8);
}
void pixle::setRed(unsigned char red){
    arrayval = (arrayval&(0xFF00FFFF))|((red<<16)&(0x00FF0000));
}
unsigned char pixle::getRed()const{
    return ((arrayval&(0xFF<<16))>>16);
}
void pixle::setFourth(unsigned char fourth){
    arrayval = (arrayval&(0x00FFFFFF))|((fourth<<24)&(0xFF000000));
}
unsigned char pixle::getFourth()const{
    return ((arrayval&(0xFF<<24))>>24);
}
void pixle::setArrayval(unsigned int inarrayval){
    arrayval = inarrayval;

}
void pixle::setArrayval(char inarrayval){
    char c=inarrayval;
    // get correct mask then multiply by 1010101 = 0x55
    arrayval = (((((0x3<<0)&c)>>0)*0x55)<<0) | (((((0x3<<2)&c)>>2)*0x55)<<8) | (((((0x3<<4)&c)>>4)*0x55)<<16) | (((((0x3<<6)&c)>>6)*0x55)<<24);
}
unsigned int pixle::getArrayval()const{
    return arrayval;
}


class bmp{
    private:
    char signature[2];
    unsigned int size;
    unsigned int imgaddress;
    unsigned int headersize;
 //should work to remove this from the public's eye
 public:
    int width;
    int height;
    short int bitsperpixle;
    pixle** pixlearray;
    
    //internal function...
    int uintbinprint(unsigned int, FILE*);
    unsigned int color16to32(unsigned int);

    
    public:
    bmp(FILE*);
    bmp(int inwidth= 10,int inheight = 10);
    ~bmp();
    int print_report();
    void printbmp(int color);
    int writeBMP(FILE*);
    void setcolors();
    
    //these need to be adapted from C form.
    void reducerange();
//    int downsample(bmpfile*,bmpfile*);
    //operator overloads to support: + (takes average) - (takes inverse) []? / (downsample)
};

bmp::bmp(FILE* fin){
    int i,ii;
    signature[0] = fgetc(fin);
    signature[1] = fgetc(fin);
    size =READ4BYTESFROM(fin);
    
    fseek(fin, 0XA, SEEK_SET);

    imgaddress = READ4BYTESFROM(fin);
    headersize = READ2BYTESFROM(fin);
    
    
    //      BITMAPINFOHEADER (nearly ubiquitous)
    if (headersize == 40) {
        fseek(fin, 0x12, SEEK_SET);
        width =      READ4BYTESFROM(fin);
        height =     READ4BYTESFROM(fin);
        
        fseek(fin, 0x1C, SEEK_SET);
        bitsperpixle = READ2BYTESFROM(fin);
    }
    //      OS/2 BITMAPCOREHEADER (sometimes used)
    else if (headersize == 12) {
        fseek(fin, 0x12, SEEK_SET);
        width =      (unsigned int)READ2BYTESFROM(fin);
        height =     (unsigned int)READ2BYTESFROM(fin);
        
        fseek(fin, 0x18, SEEK_SET);
        bitsperpixle = READ2BYTESFROM(fin);
    }
    
    else if (headersize == 64){
        printf("BITMAPCOREHEADER2 not supported");
        printf("\nData aquisition failed.");
        throw;
    }
    else {
        printf("Header size:%i not supported.",headersize);
        throw;
    }

// start reading in the actual pixels!!!!!!
    fseek(fin, imgaddress, SEEK_SET);

    width = abs(width);
    height = abs(height);

    pixlearray = new pixle*[width];
    for(i=0;i<width;i++){
        pixlearray[i] = new pixle[height];
    }

 //4 bytes per pixel
    if (bitsperpixle == 32) {
        for (i =0; i<width; i++) {
            for (ii=0; ii<height;ii++) {
                pixlearray[i][ii].setArrayval(unsigned(READ4BYTESFROM(fin)));
            }
        }
    }
    
 //2 bytes per pixel
    else if (bitsperpixle == 16) {
        for (i =0; i<width; i++) {
            for (ii=0; ii<height; ii++) {
                pixlearray[i][ii].setArrayval(color16to32((READ2BYTESFROM(fin))));
            }
        }
        bitsperpixle = 32;
    }

 //1 byte per pixel
 ////doesn't realllly work...
    else if (bitsperpixle ==8) {
        for (i =0; i<width; i++) {
            for (ii=0; ii<height; ii++) {
                pixlearray[i][ii].setArrayval(char(fgetc(fin)));
            }
        }
    }

    else {
        printf("given BPP not supported");
        throw;
    }
 
}

bmp::bmp(int inwidth,int inheight){
    short int bpp = 32;

     signature[0] = 'B';
     signature[1] = 'M';
     width = inwidth;
     height= inheight;
     size = height*width*((bpp+7)/8)+40+14;
     headersize = 40;
     bitsperpixle =bpp;
     imgaddress = 40+14;
     
     pixlearray = new pixle*[width];
     for(int i=0;i<width;i++){
         pixlearray[i] = new pixle[height];
     }
}
bmp::~bmp(){
    for(int i=0;i<width;i++){
        delete[] pixlearray[i];
    }
    delete[] pixlearray;
}

//not sure if the enum will work with regards to "color"
void bmp::printbmp(int color){
    int i,ii;
    for(i = 0; i<abs(height);i++){
        for (ii = 0; ii<abs(width); ii++) {
            switch(color){
                case RED:
                    printf("%0x ",pixlearray[i][ii].getRed());
                    break;
                case BLUE:
                    printf("%0x ",pixlearray[i][ii].getBlue());
                    break;
                case GREEN:
                    printf("%0x ",pixlearray[i][ii].getGreen());
                    break;
                case VAL:
                    printf("%08x ",pixlearray[i][ii].getArrayval());
                    break;
                case FOURTH:
                    printf("%0x ",pixlearray[i][ii].getFourth());
                    break;
                default:
                    printf("failure");
                    break;
            }
        }
        printf("\n");
    }
}

int bmp::print_report(){

    printf("length: %u\n",size);
    printf("offset: %u\n",imgaddress);
    printf("size:   %i x %i\n",height,width);
    printf("bpp:    %hi\n",bitsperpixle);

    //    reducerange(&filein);
    printf("blue:\n");
    printbmp(BLUE);
    printf("red:\n");
    printbmp(RED);
    printf("green:\n");
    printbmp(GREEN);
    printf("fourth:\n");
    printbmp(FOURTH);
    printf("bytes:\n");
    printbmp(VAL);

    return 0;
}

int bmp::uintbinprint(unsigned int num, FILE* file){
    fputc((num)&0xff,file);
    fputc((num>>8 ),file);
    fputc((num>>16),file);
    fputc((num>>24),file);
    return 0;
}
unsigned int bmp::color16to32(unsigned int in){
    unsigned int ret= (((in&0x0000000f))<<4)|(((in&0x000000f0))<<8)|(((in&0x00000f00))<<12)|(((in&0x0000f000))<<16);
    ret |= ret>>4;
    return ret;
}


int bmp::writeBMP(FILE*file){
int i,ii;

    //first header
    fputc(signature[0],file);
    fputc(signature[1],file);
    printf("\n\n--------------------debugger---------------------\n");
    printf("size:%u\n",size);
    printf("-----------------------end-----------------------\n\n");
    uintbinprint(size,file);
    //should instead rewind and write ending byte index here...
    uintbinprint((unsigned int)0,file);
    uintbinprint(imgaddress,file);
    //second header
    uintbinprint((unsigned int)40,file);
    uintbinprint((int)(width),file);
    uintbinprint((int)(height),file);
    uintbinprint((unsigned int)(0x00000001|bitsperpixle<<16),file);
    uintbinprint((unsigned int) 0,file);
    // this one is wrong... It should be the size of the bitmap but we don't maintain that information so it will have to be calculated (more carefully for unsopported dtyps)
    // ... looks like most renderers don't even use this information so letting it be 0 works too...
    uintbinprint(abs(width*height*bitsperpixle),file);
    //horizontal res.
    uintbinprint(width*100,file);
    //vertical res.
    uintbinprint(height*100,file);
    uintbinprint((unsigned int)0,file);
    uintbinprint((unsigned int)0,file);
    // end of secondary header

    while(ftell(file)<imgaddress){
        fputc((char)0,file);
    }
    if(bitsperpixle == 32){
        for(i=0;i<abs(width);i++){
            for(ii=0;ii<abs(height);ii++){
                uintbinprint(pixlearray[i][ii].getArrayval(),file);
            }
        }
    }
    else if(bitsperpixle == 16){
        for(i=0;i<abs(width);i++){
            for(ii=0;ii<abs(height);ii++){
                //need to watch the bitshift here... it could be unneccissary or in the wrong direction...
        // this is probably so wrong its painful. Should seriously consider converting everything to 32 bit color. It would be so much more robust
                fputc((char)((pixlearray[i][ii].getArrayval())&0xff),file);
                fputc((char)((pixlearray[i][ii].getArrayval()>>8 )&0xff),file);
            }
        }
    }

}




/*
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
            //need to update the color listings here...
            out->pixlearray[i/dw][ii/dh].red = sred/(dh*dw);
            out->pixlearray[i/dw][ii/dh].blue = sblue/(dh*dw);
            out->pixlearray[i/dw][ii/dh].yellow = syellow/(dh*dw);
            out->pixlearray[i/dw][ii/dh].fourth = sfourth/(dh*dw);
        }
    }
    setvalue(out);
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
*/
