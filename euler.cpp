/*
NOLAN MILLER
*/

#include <stdio.h>
#define STDIO
#include <stdlib.h>
#define STDLIB
#include <math.h>
#define MATH
#include "cppbmp.h"

typedef struct{
    double x;
    double y;
}pt;

pt euler_step(double(*dFunct)(pt in),pt pt0,double delta);
void graph(double(*dFunct)(pt in),pt pt0,double delta,pt max, pt min,FILE* output);
int ptwithin(pt in, pt min, pt max);

double function(pt in){
    return in.x+in.y;
}


    
int main(){

    double(*fp)(double,double) = &function;

    printf("%lf",fp(3,5));
    
    return 0;
}


pt euler_step(double(*dFunct)(pt in),pt pt0,double delta){
    pt out = pt0;
    out.y += dFunct(out)*delta;
    out.x += delta;
    return out;
}

void graph(double(*dFunct)(pt in),pt pt0,double delta,pt max, pt min,FILE* output){
    bmp out = bmp(500,500);
    int i,ii;
    pt origin;
    pt size;
    size.x = max.x-min.x;
    size.y = max.y-min.y;
    origin.x = out.width*(-min.x/(size.x));
    origin.y = out.height*(-min.y/(size.y));
    
    if(origin.x<0)
        origin.x = 0;
    if(origin.y<0)
        origin.y = 0;

    for(i = 0;i<out.width;i++){
        out.pixlearray[i][origin.y];
    }
    for(i = 0;i<out.height;i++){
        out.pixlearray[origin.x][ii];
    }

    pt loc = pt0;
    while(ptwithin(loc,min,max)){
        out.pxilearray[((loc.x*out.width)/size.x)-min.x][((loc.y*out.height)/size.y)-min.y];
        loc = euler_step(dFunct,loc,delta);
    }
}


int ptwithin(pt in, pt min, pt max){
    return in.x<=max.x&&in.y<=max.y&&in.x>=min.x&&in.y>=min.y;
}
