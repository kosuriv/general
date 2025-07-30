#include <math.h>
#include <iostream>
using namespace std;

////////////////////////////////////// Global variables/////////////////////////////

//Desired value "Vd"
float Vd = 1500; // we are assuming it is 1500 PPM by default. At this point, the programmers should send from the main program the corresponding value
//to Vd variable and replace it with the information (value) that comes from the keyboard.

float e1,e2;
//e1 is the error = Vd - CO2 (CO2 value measurement)
//e2 is the derivate of e1


//CO2 value measurement
float CO2 = 600;//This is any value, it was placed for reference. Programmers should send from the main program the corresponding value
//to the measured value of CO2 and enter it in the variable CO2.


//Delta
float Delta=0;//is a variable to be used in calculate the derivative.

////////////////////////////////////////////////////////////////////////////////////


//void Membership functions
/*****************************************************************************************************************************/    
void Trapezoid(float *x, float *y, const float params[4], int num) {
	for (int i = 0; i < num; i++) {
      if (x[i]<params[0]||x[i]>=params[3]){
      	y[i]=0;
      }
      if (x[i]>=params[0] && x[i]<params[1]){
	    y[i]=(x[i]-params[0])/(params[1]-params[0]);
	  }
      if (x[i]>=params[1] && x[i]<params[2]){
      	y[i]=1;
      	
      }
      if (x[i]<=params[3] && x[i]>=params[2]){
      	y[i]=1+(x[i]-params[2])/(params[2]-params[3]);
      }
	}	
}

void Triangle(float *x, float *y, const float params[3], int num) {
	for (int i = 0; i < num; i++) {
      if (x[i]<params[0]||x[i]>params[2]){
      	y[i]=0;
      }
      if (x[i]>=params[0] && x[i]<params[1]){
	    y[i]=(x[i]-params[0])/(params[1]-params[0]);
	  }
      if (x[i]>=params[1] && x[i]<=params[2]){
      	y[i]=(params[2] - x[i])/(params[2]-params[1]);
      	
      }
	}	
}


/////Some functions to make the algorithm
/*****************************************************************************************************************************/   
void alphacut(float *y, float *Out, float alpha, int num) {
	for (int i = 0; i < num; i++) {
    if (y[i] > alpha){
    	Out[i] = alpha;
    }
    else {
    	Out[i] = y[i];
    }

	}	
}

void VecsMax(float *Vx, float *Vy, float *Out, int num) {
	for (int i = 0; i < num; i++) {
    if (Vx[i] > Vy[i]){
    	Vy[i] = Vx[i];
    }   
    	Out[i] = Vy[i];
	}	
}


#ifndef NO_EC
#define num   20 //181
float x[num];//for input 1
float x2[num];//for input 2


 //for input 1

 float *ENG = new float[num];
 float *ENP = new float[num];
 float *EC = new float[num];
 float *EPP = new float[num];
 float *EPG = new float[num];
 
 
 //for input 2
 
 float *ENG2 = new float[num];
 float *ENP2 = new float[num];
 float *EC2 = new float[num];
 float *EPP2 = new float[num];
 float *EPG2 = new float[num];


 float *VNG = new float[num];
 float *VNP = new float[num];
 float *VC = new float[num];
 float *VPP = new float[num];
 float *VPG = new float[num];

//For input 1
 float *a1 = new float[num];
 float *a2 = new float[num];
 float *a3 = new float[num];
 float *a4 = new float[num];
 float *a5 = new float[num];

//For input 2
 float *a6 = new float[num];
 float *a7 = new float[num];
 float *a8 = new float[num];
 float *a9 = new float[num];
 float *a10 = new float[num];


float *b1 = new float[num];// For Trapezoid 
float *b2 = new float[num];// For Triangle
float *b3 = new float[num];// For Triangle
float *b4 = new float[num];// For Triangle
float *b5 = new float[num];// For Trapezoid 

float m1[num];
float m2[num];
float m3[num];

float *f = new float[num];


int id[num];
int id2[num];

#endif
/*****************************************************************************************************************************/  


//Main Program
/*****************************************************************************************************************************/  
#ifdef NO_EC
main() {
#else
int CO2_Calculate(int co2_error , int co2_rate) {
#endif
//////////////////////////////////////////// Values for the programm/////////////////////////////////

///The values of the ranges of the membership functions for the inputs and output were used in the tests carried out and are the ones we recommend using. 
//The flow at the tank outlet valve was set at 1 SCFH.

///Input range of Membership functions of Input 1
float xmin = -1500/12;
float xmax =  1500/12;

///Input range of Membership functions of Input 2
float xmin2 = -250/12;
float xmax2 =  250/12;

///Output Range of Membership functions
float vmin = -25/12;
float vmax =  25/12;

#ifdef NO_EC
////Inputs Values
//In this case, we have two Error Inputs value, e1 is the error = Vd - CO2 (Vd is desired value and CO2 is the value measurement)
//and e2 is the derivate of e1.

e1 = Vd - CO2;
e2 = (Delta - e1)/0.1;//calculation of the derivate of e1

Delta = e1;//it is updated with the last value of e1 for when the next measurement and the next calculation of the derivative are expected.


//must be xmin1 < e1 < xmax1 and xmin2 < e2 < xmax2.

if (e1>=xmax){
    e1 = xmax-1;
} else if (e1<xmin){
     e1 = xmin;
}


if (e2>=xmax2){
    e2 = xmax2-1;
    } else if (e2<xmin2){
     e2 = xmin2;
}


#else
e1 = co2_error;

e2 = co2_rate;

#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////
	
	
/////////////////////////////////////////Signal Input////////////////////////////////////////////////////
///Points number for Membership functions
#ifdef NO_EC
int num = 181;
#endif
float step = (xmax-xmin)/(num-1);//for input 1
float step2 = (xmax2-xmin2)/(num-1);// for input 2

float xp=xmin;//for input 1
float xp2=xmin2;//for input 2
#ifdef NO_EC
float x[num];//for input 1
float x2[num];//for input 2
#endif
for (int i = 0; i < num; i++) {
    x[i]=xp;
    xp=x[i]+step;
}

for (int i = 0; i < num; i++) {
    x2[i]=xp2;
    xp2=x2[i]+step2;
}

/////////////////////////////////////////// Input Membership functions/////////////////
#ifdef NO_EC

 //for input 1

 float *ENG = new float[num];
 float *ENP = new float[num];
 float *EC = new float[num];
 float *EPP = new float[num];
 float *EPG = new float[num];
 
 
 //for input 2
 
 float *ENG2 = new float[num];
 float *ENP2 = new float[num];
 float *EC2 = new float[num];
 float *EPP2 = new float[num];
 float *EPG2 = new float[num];
#endif
 
//for input 1

const float funcE1[4] ={xmin,xmin,xmin/2,xmin/4};//ENG trapezoid function
const float funcE2[4] ={xmin/2,xmin/4,0};//ENP triangle function
const float funcE3[4] ={xmin/4,0,xmax/4};//EC triangle function
const float funcE4[4] ={0,xmax/4,xmax/2};//EPP triangle function
const float funcE5[4] ={xmax/4,xmax/2,xmax,xmax};//EPG trapezoid function

//for input 2

const float funcE6[4] ={xmin2,xmin2,xmin2/2,xmin2/4};//ENG trapezoid function
const float funcE7[4] ={xmin2/2,xmin2/4,0};//ENP triangle function
const float funcE8[4] ={xmin2/4,0,xmax2/4};//EC triangle function
const float funcE9[4] ={0,xmax2/4,xmax2/2};//EPP triangle function
const float funcE10[4] ={xmax2/4,xmax2/2,xmax2,xmax2};//EPG trapezoid function
 
//functions

//for input 1 
Trapezoid(x,ENG,funcE1,num);
Triangle(x,ENP,funcE2,num);
Triangle(x,EC,funcE3,num);
Triangle(x,EPP,funcE4,num);
Trapezoid(x,EPG,funcE5,num);

//for input 2
Trapezoid(x2,ENG2,funcE6,num);
Triangle(x2,ENP2,funcE7,num);
Triangle(x2,EC2,funcE8,num);
Triangle(x2,EPP2,funcE9,num);
Trapezoid(x2,EPG2,funcE10,num);


///////////////////////////////////////////Cut points/////////////////////////////////////

//For input 1
float c1;
float c2;
float c3;
float c4;
float c5;

//For input 2
float c6;
float c7;
float c8;
float c9;
float c10;

float error1 = e1;
float error2 = e2; 
int un1 = ceil((error1-xmin+1)/step);//for input 1
int un2 = ceil((error2-xmin2+1)/step2);//for input 2

////// Input 1 cutting the first input membership functions
c1 = ENG[un1-1];
c2 = ENP[un1-1];
c3 = EC[un1-1];
c4 = EPP[un1-1];
c5 = EPG[un1-1];

//Free memory
delete [] ENG;
delete [] ENP;
delete [] EC;
delete [] EPP;
delete [] EPG;

////// Input 2 cutting the second input membership functions
c6 = ENG2[un2-1];
c7 = ENP2[un2-1];
c8 = EC2[un2-1];
c9 = EPP2[un2-1];
c10 = EPG2[un2-1];

//Free memory
delete [] ENG2;
delete [] ENP2;
delete [] EC2;
delete [] EPP2;
delete [] EPG2;

/////////////////////////////////////////Output Signal////////////////////////////////////////////////////

float vpaso = (vmax-vmin)/(num-1);
float vp=vmin;
float v[num];

for (int i = 0; i < num; i++) {
    v[i]=vp;
    vp=v[i]+vpaso;
}

#ifdef NO_EC
///////////////////////////////////////////// Output Membership functions/////////////////
 
 float *VNG = new float[num];
 float *VNP = new float[num];
 float *VC = new float[num];
 float *VPP = new float[num];
 float *VPG = new float[num];
#endif
//The ranges can be changed as we want
const float funcOut1[4] ={-25,-25,-12.5,-6.25};//VNG trapezoid function
const float funcOut2[4] ={-12.5,-6.25,0};//VNP triangle function
const float funcOut3[4] ={-6.25,0,6.25};//VC triangle function
const float funcOut4[4] ={0,6.25,12.5};//VPP triangle function
const float funcOut5[4] ={6.25,12.5,25,25};//VPG trapezoid function

//functions
Trapezoid(v,VNG,funcOut1,num);
Triangle(v,VNP,funcOut2,num);
Triangle(v,VC,funcOut3,num);
Triangle(v,VPP,funcOut4,num);
Trapezoid(v,VPG,funcOut5,num);

///////////Evaluating the cutoff values in each function
#ifdef NO_EC
//For input 1
 float *a1 = new float[num];
 float *a2 = new float[num];
 float *a3 = new float[num];
 float *a4 = new float[num];
 float *a5 = new float[num];

//For input 2
 float *a6 = new float[num];
 float *a7 = new float[num];
 float *a8 = new float[num];
 float *a9 = new float[num];
 float *a10 = new float[num];
#endif

alphacut(VNG,a1,c1,num);//a1
alphacut(VNP,a2,c2,num);//a2
alphacut(VC,a3,c3,num);//a3
alphacut(VPP,a4,c4,num);//a4
alphacut(VPG,a5,c5,num);//a5

alphacut(VNG,a6,c6,num);//a6
alphacut(VNP,a7,c7,num);//a7
alphacut(VC,a8,c8,num);//a8
alphacut(VPP,a9,c9,num);//a9
alphacut(VPG,a10,c10,num);//a10


//Free memory
delete [] VNG;
delete [] VNP;
delete [] VC;
delete [] VPP;
delete [] VPG;

///////////////////////////Here we take the maximum value of the intersection of each membership function
#ifdef NO_EC
float *b1 = new float[num];// For Trapezoid 
float *b2 = new float[num];// For Triangle
float *b3 = new float[num];// For Triangle
float *b4 = new float[num];// For Triangle
float *b5 = new float[num];// For Trapezoid 
#endif
///// Using "or" function in the rules
VecsMax(a1,a6,b1,num);//b1
VecsMax(a2,a7,b2,num);//b2
VecsMax(a3,a8,b3,num);//b3
VecsMax(a4,a9,b4,num);//b4
VecsMax(a5,a10,b5,num);//b5

//Free memory
delete [] a1;
delete [] a2;
delete [] a3;
delete [] a4;
delete [] a5;
delete [] a6;
delete [] a7;
delete [] a8;
delete [] a9;
delete [] a10;


#ifdef NO_EC
///////////////////////////Calculing the maximum value function///////////////
float m1[num];
float m2[num];
float m3[num];

float *f = new float[num];
#endif


VecsMax(b1,b2,m1,num);
VecsMax(b3,m1,m2,num);
VecsMax(b4,m2,m3,num);
VecsMax(b5,m3,f,num);//f: maximum function (resulting function)

//Free memory
delete [] b1;
delete [] b2;
delete [] b3;
delete [] b4;
delete [] b5;

//////////////////////////////Calculing the Centroid Value (Defuzification)/////////////////////
#ifdef NO_EC
int id[num];
#endif
		
		for (int i = 0; i < num; i++) {
    	if (f[i]!=0){
            id[i] = i;
        }
            else {
            id[i] = 0;
		}
	}
	
	int con = 0;

#ifdef NO_EC
	int id2[num];
#endif

	for (int i = 0; i < num; i++) {
		if(id[i]>0){
		id2[con] = i;
		con = con + 1;
		}
	}	
	

	float yr[con];
	float xpp[con];
	
	for (int i = 0; i < con; i++) {
		yr[i] = f[id2[i]];
	}
	
    for (int i = 0; i < con; i++) {
		xpp[i] = v[id2[i]];
	}
	
	
	float h  = xpp[2]-xpp[1];
	float ya = 0;
	float a[con];
	float mp[con];
	float aacum = 0;

	for (int i = 0; i < con; i++) {
		a[i] = h*(yr[i] + ya)/2;
    	aacum = aacum + a[i];
    	mp[i] =xpp[i] - h/2;
    	ya = yr[i];
	}
	
float u = 0;

for (int i = 0; i < con; i++) {
		u = u + a[i]/aacum*mp[i];//Signal controller u
	}

//Free memory
delete [] f;
////////////////////////////////////Controller Input and Output//////////////////////////////////////////////

	cout<<"Input value to Fuzzy Controller, e1 = "<<e1<<endl; /// Input 1
	cout<<"Input value to Fuzzy Controller, e2 = "<<e2<<endl; /// Input 2
 	cout<<"Output value from Fuzzy Controller, u = "<<u<<endl;/// Output*/
 
 if (u < 0){
 u = 0;
 }
 
 if (u > 1){
 u=1;
 }
 return u;  
}
