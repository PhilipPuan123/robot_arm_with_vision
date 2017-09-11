#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <condition_variable>
#include "IK.h"
#include "Serial.h"
#include "cam.h"
#include "tricks.h"
#include "PathPlanning.h"
#include "Routines.h"
#include <algorithm>
#include <chrono>
#include <math.h>
#include "doublefann.h"


#define x_comp      0
#define y_comp      1
#define z_comp      2

#define d1  12.5   //ground to q1
#define d6  12 //gripper to wrist
#define a2 15.0    //q1 to q2
#define d4 19.2  //q2 to wrist

#define degtorad 0.01745329251994329576923690768488612713
#define radtodeg 57.2957795130823208767981548141051703324
#define pi  3.14159265358979323846264338327950288419716939937510

#define PI (3.141592653589793)
#define HALF_PI (1.570796326794897)
#define DOF 6 /* degrees of freedom in the system */

using namespace std;
/* I feel safer globally defining these*/
mutex mu,grabmu;
condition_variable cond;

char const * portName = "\\\\.\\COM3";
Tricks tricks = Tricks();
PathPlanning pp = PathPlanning();
cam CAM = cam(0,30); /* 30 is as high as she'll go*/
IK ik = IK();
Serial *arduino;
Routines rout = Routines();

double w[3][3]= {{0,1,0},    //the target rotation matrix
                 {0,0,1},
                 {1,0,0}};

struct Pos{
    double x; double y; double z;
    double alpha; double beta; double gamma; /* euler angles for the target orientation */
    int grip;
};

int main(void){
    arduino = new Serial(portName);
    cout << "is connected: " << arduino->IsConnected() << endl;

//    int flip = 1;
//
//    struct Pos start, stop, test;
//    tricks.setPos(&start,-20,30,15,0,0,0,10);
//    tricks.setPos(&stop,20,25,25,0,0,0,10);
//    tricks.setPos(&test,0,30,15,0,0,0,10);

//    tricks.setArmPos(test,flip);
//    pp.lineOO(start,stop,flip);
//    tricks.wait();
//    pp.lineOO(stop,start,flip);


//    rout.stacking(15,flip);
//    rout.stackingOO(15,flip);
//    rout.monkeySeeMonkeyDo();
    rout.showOff(10);
//    ik.inverseKinematicsRaw(0,20,10,w,angles1,0);
//    ik.forwardKinematics(angles1,jointPos);




    return 1;
}
