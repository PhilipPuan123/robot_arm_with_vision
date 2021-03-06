#include "Routines.h"
#define pi  3.141592653589
#define half_pi 1.570796326794897
#define degtorad 0.0174532925199432957
#define radtodeg 57.295779513082320876

using namespace cv;
using namespace std;

const float arucoSquareDimension = 0.0265f; //in meters

struct Pos{
    double x; double y; double z;
    double alpha; double beta; double gamma; /* euler angles for the target orientation */
    int grip;
};

Routines::Routines()
{
    return;
}
/* command the robot arm to pick up a block and stack it on other picked up blocks */
int Routines::returnBlock(double x, double y, double z, double temptheta, double speed, int flip, struct Pos& drop,int counter){
    unique_lock<mutex> locker(grabmu,defer_lock);
    if(!locker.try_lock()){
        cout << "already in use!" << endl;
        tricks.msleep(100);
        return 0;
    }
    double delta = 3.1; /* height of the block */
    double theta,r,time;
    double pitchdown = -45*degtorad;
    if(y < 12){
        cout << "ain't gonna wreck myself!!!" << endl;
        return 0;
    }
    theta = tricks.fixtheta(temptheta);
    cout << "x=" << x << "  y=" << y << "   theta=" <<theta << endl;
    struct Pos  obj, objup, dropup;

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);
    tricks.setPos(&objup, x,y,z + 12,theta,0,pitchdown,100);
    tricks.setPos(&obj,x,y,z,theta,0,pitchdown,0);

    r = sqrt(pow(dropup.x-objup.x,2) + pow(dropup.x - objup.x,2));
    time = r/speed;

    tricks.line(drop,dropup,speed,flip);
    tricks.pointToPoint(dropup,objup,time,flip);
    tricks.line(objup,obj,speed/2,flip);
    tricks.msleep(100);
    /* there should be a better way to close the gripper at all points...*/
    tricks.setPos(&objup, x,y,z + 10,theta,0,pitchdown,0);
    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,0);

    tricks.line(obj,objup,speed/2,flip);
    tricks.msleep(100);
    tricks.pointToPoint(objup,dropup,time,flip);
    tricks.line(dropup,drop,speed/4,flip);

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,100);
    tricks.line(drop,dropup,speed,flip);

    tricks.setPos(&drop,drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);

    locker.unlock();
    return 1;
}

/* the same as returnBlock except that the inverse kinematics are being handled by neural networks */
int Routines::returnBlockNN(double x, double y, double z, double temptheta, double speed, int flip, struct Pos& drop,int counter){
    unique_lock<mutex> locker(grabmu,defer_lock);
    if(!locker.try_lock()){
        cout << "already in use!" << endl;
        tricks.msleep(100);
        return 0;
    }
    double delta = 3.1; /* height of the block */
    double theta,r,time;
    double pitchdown = -45*degtorad;
    double anglesInternal[6] = {0,0,0,0,-1,0};

    if(y < 12){
        cout << "ain't gonna wreck myself!!!" << endl;
        return 0;
    }
    theta = tricks.fixtheta(temptheta);
    cout << "x=" << x << "  y=" << y << "   theta=" <<theta << endl;
    struct Pos  obj, objup, dropup;

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);
    tricks.setPos(&objup, x,y,z + 12,theta,0,pitchdown,100);
    tricks.setPos(&obj,x,y,z,theta,0,pitchdown,0);

    r = sqrt(pow(dropup.x-objup.x,2) + pow(dropup.x - objup.x,2));
    time = r/speed;

    tricks.lineNN(drop,dropup,speed,flip,anglesInternal);
    tricks.pointToPointNN(dropup,objup,time,flip,anglesInternal);
    tricks.lineNN(objup,obj,speed/2,flip,anglesInternal);
    tricks.msleep(100);
    /* there should be a better way to close the gripper at all points...*/
    tricks.setPos(&objup, x,y,z + 10,theta,0,pitchdown,0);
    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,0);

    tricks.lineNN(obj,objup,speed/2,flip,anglesInternal);
    tricks.msleep(100);
    tricks.pointToPointNN(objup,dropup,time,flip,anglesInternal);
    tricks.lineNN(dropup,drop,speed/4,flip,anglesInternal);

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,100);
    tricks.lineNN(drop,dropup,speed,flip,anglesInternal);

    tricks.setPos(&drop,drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);

    locker.unlock();
    return 1;
}

/* same as returnBlock but this one can also avoid obstacles */
int Routines::returnBlockOO(double x, double y, double z, double temptheta, double speed, int flip, struct Pos& drop,int counter,
                            vector<vector<vector<double > > >& objectPoints, vector<int >& foundMarkers){
    unique_lock<mutex> locker(grabmu,defer_lock);
    if(!locker.try_lock()){
        cout << "already in use!" << endl;
        tricks.msleep(100);
        return 0;
    }
    double delta = 3.1; /* height of the block */
    double theta;
    double pitchdown = -45*degtorad;
    if(y < 12){
        cout << "ain't gonna wreck myself!!!" << endl;
        return 0;
    }
    theta = tricks.fixtheta(temptheta);
    cout << "x=" << x << "  y=" << y << "   theta=" <<theta << endl;
    struct Pos  obj, objup, dropup;

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);
    tricks.setPos(&objup, x,y,z + 12,theta,0,pitchdown,100);
    tricks.setPos(&obj,x,y,z,theta,0,pitchdown,0);

    tricks.line(drop,dropup,speed,flip);

    pp.line(dropup,objup,flip,objectPoints,foundMarkers);

    tricks.line(objup,obj,speed/2,flip);
    tricks.msleep(100);
    /* there should be a better way to close the gripper at all points...*/
    tricks.setPos(&objup, x,y,z + 10,theta,0,pitchdown,0);
    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,0);

    tricks.line(obj,objup,speed/2,flip);
    tricks.msleep(100);
    pp.line(objup,dropup,flip,objectPoints,foundMarkers);

    tricks.line(dropup,drop,speed/4,flip);

    tricks.setPos(&dropup, drop.x,drop.y,drop.z + delta,0,0,drop.gamma,100);
    tricks.line(drop,dropup,speed,flip);

    tricks.setPos(&drop,drop.x,drop.y,drop.z + delta,0,0,drop.gamma,drop.grip);

    locker.unlock();
    return 1;
}

/* find blocks and stack them up*/
void Routines::stacking(double speed, int flip){
    double x,y,z,theta; /* Pose of the block we want to grab */
    int toFind = 42; /* The marker to pick up, blocks have markers 42 to 49 glued on top of them */
    int looptieloop = 1;
    vector<double> relPos1(3);
    bool getVecs = false;

    Mat cameraMatrix = Mat::eye(3,3, CV_64F);
    Mat distanceCoefficients = Mat::zeros(5,1, CV_64F);
    Mat relativeMatrix = Mat::zeros(3,3, CV_64F);
    CAM.getMatrixFromFile("CameraCalibration720.dat", cameraMatrix, distanceCoefficients);

    /* the drop off location for the tower of blocks */
    struct Pos drop;
    tricks.setPos(&drop, -20,25,2,0,0,-45*degtorad,100);
    tricks.setArmPos(drop, flip);

    /* start the webcam, every time toFind is changed it spits out the corresponding Pose as soon as it can*/
    thread t(&cam::startWebcamMonitoring, &CAM, ref(cameraMatrix), ref(distanceCoefficients), ref(arucoSquareDimension),
             ref(relPos1) ,ref(relativeMatrix) ,ref(toFind), ref(getVecs), ref(looptieloop) );
    t.detach();

    looptieloop = tricks.wait();
    int counter = 0; /* to jump to the next block */

    while(looptieloop){
        getVecs = true; /* signal that we want to find a block, startWebcamMonitoring() sets it to False when it's done */

        unique_lock<mutex> locker(mu);
        cond.wait(locker, [&]{return !getVecs;}); /* check that the markers is actually found */

        /* convert computed values to centimeters and get the rotation about the z axis of the robot frame, relPos1 is in meters.
           Note that x should be 100*relPos1[0] but it's not because of errors in the camera or robot or whatever... this works! */
        x = 90*relPos1[0] + 1; y = 100*relPos1[1] + 10; z = 1.5;
        theta = atan2(relativeMatrix.at<double>(1,0),relativeMatrix.at<double>(0,0));
        locker.unlock();
        toFind++;

        /* pick up the block */
        returnBlock(x,y,z,theta,speed,flip,drop,counter);
        counter++;

        if(toFind>47){ /* every block has a unique marker on top of it so here I only pick up the first few blocks */
            looptieloop = 0;
            break;
        }
    }
}
/* same as stacking but now it can also avoid obstacles */
void Routines::stackingOO(double speed, int flip){
    double x,y,z,temptheta;
    int toFind = 42;
    int looptieloop = 1;
    vector<double> relPos1(3);
    bool getVecs = false;
    Mat cameraMatrix = Mat::eye(3,3, CV_64F);
    Mat distanceCoefficients = Mat::zeros(5,1, CV_64F);
    Mat relativeMatrix = Mat::zeros(3,3, CV_64F);
    CAM.getMatrixFromFile("CameraCalibration720.dat", cameraMatrix, distanceCoefficients);

    struct Pos drop;
    tricks.setPos(&drop, -20,25,2,0,0,-45*degtorad,100);
    tricks.setArmPos(drop, flip);

    vector<vector<vector<double > > > objectPoints;
    vector<int > foundMarkers;

    pp.prepareOOThread(objectPoints, foundMarkers);
    tricks.msleep(1000);

    thread t(&cam::startWebcamMonitoring, &CAM, ref(cameraMatrix), ref(distanceCoefficients), ref(arucoSquareDimension),
             ref(relPos1) ,ref(relativeMatrix) ,ref(toFind), ref(getVecs), ref(looptieloop) );
    t.detach();
    looptieloop = tricks.wait();
    int counter = 0;
    while(looptieloop){
        getVecs = true;
        unique_lock<mutex> locker(mu);
        cond.wait(locker, [&]{return !getVecs;});
        x = 90*relPos1[0] + 1; y = 100*relPos1[1] + 10; z = 1.5;
        temptheta = atan2(relativeMatrix.at<double>(1,0),relativeMatrix.at<double>(0,0));
        locker.unlock();
        toFind++;
        returnBlockOO(x,y,z,temptheta,speed,flip,drop,counter,objectPoints,foundMarkers);
        counter++;
        if(toFind>45){
            looptieloop = 0;
            break;
        }
    }
}

/* move around a bit to show of the inverse kinematics */
void Routines::showOff(double speed){
    int flip = 0;

    struct Pos start, leftlow, rightlow, leftup, rightup;
    tricks.setPos(&start,0,25,20,0,0,0,10);
    tricks.setPos(&leftlow,-20,25,5,0,0,0,10);
    tricks.setPos(&rightlow,20,25,5,0,0,0,10);
    tricks.setPos(&leftup,-20,25,25,0,0,0,10);
    tricks.setPos(&rightup,20,25,25,0,0,0,10);

    tricks.setArmPos(start,flip);
    tricks.wait();
    tricks.line(start,leftlow,speed,flip);
    tricks.line(leftlow,leftup,speed,flip);
    flip = 0;
    tricks.setArmPos(leftup,flip);
    tricks.msleep(500);
    tricks.line(leftup,rightup,speed,flip);
    flip = 0;
    tricks.setArmPos(rightup,flip);
    tricks.msleep(500);
    tricks.line(rightup,rightlow,speed,flip);
    tricks.line(rightlow,leftlow,speed,flip);
    tricks.line(leftlow,start,speed,flip);

    double dummy = 40;

    for (int j=0; j<=dummy; j++){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik.inverseKinematics(0,25,20,t,angles,flip);
       tricks.commandArduino(angles,10);
       tricks.msleep(50);
    }

    for (int j=dummy; j>=-dummy; j--){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik.inverseKinematics(0,25,20,t,angles,flip);
       tricks.commandArduino(angles,10);
       tricks.msleep(50);
    }

    for (int j=-dummy; j<=0; j++){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik.inverseKinematics(0,25,20,t,angles,flip);
       tricks.commandArduino(angles,10);
       tricks.msleep(50);
   }
}

/* same as showOff but now the inverse kinematics are done by neural networks */
void Routines::showOffNN(double speed){

    int flip = 1;
    struct Pos start, leftlow, rightlow, leftup, rightup;
    double anglesInternal[6] = {0,0,0,0,-1,0};
    double rawAngles[7];
    double arduinoAngles[7];

    tricks.setPos(&start,0,25,20,0,0,0,10);
    tricks.setPos(&leftlow,-20,25,5,0,0,0,10);
    tricks.setPos(&rightlow,20,25,5,0,0,0,10);
    tricks.setPos(&leftup,-20,25,25,0,0,0,10);
    tricks.setPos(&rightup,20,25,25,0,0,0,10);

    tricks.setArmPosNN(start,flip,anglesInternal);
    tricks.wait();
    tricks.lineNN(start,leftlow,speed,flip,anglesInternal);
    tricks.lineNN(leftlow,leftup,speed,flip,anglesInternal);
    tricks.msleep(500);
    tricks.lineNN(leftup,rightup,speed,flip,anglesInternal);
    tricks.msleep(500);
    tricks.lineNN(rightup,rightlow,speed,flip,anglesInternal);
    tricks.lineNN(rightlow,leftlow,speed,flip,anglesInternal);
    tricks.lineNN(leftlow,start,speed,flip,anglesInternal);

    double dummy = 70;

    for (int j=0; j<=dummy; j++){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik_nn.inverseKinematicsNNRawDelta(0,25,20,t,anglesInternal,rawAngles);
       ik.convertAngles(rawAngles,arduinoAngles);
       tricks.commandArduino(arduinoAngles,10);
       tricks.msleep(50);
    }

    for (int j=dummy; j>=-dummy; j--){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik_nn.inverseKinematicsNNRawDelta(0,25,20,t,anglesInternal,rawAngles);
       ik.convertAngles(rawAngles,arduinoAngles);
       tricks.commandArduino(arduinoAngles,10);
       tricks.msleep(50);
    }

    for (int j=-dummy; j<=0; j++){
       ik.eulerMatrix( sin((j/(float)dummy)*half_pi)*half_pi,0,0,t);
       ik_nn.inverseKinematicsNNRawDelta(0,25,20,t,anglesInternal,rawAngles);
       ik.convertAngles(rawAngles,arduinoAngles);
       tricks.commandArduino(arduinoAngles,10);
       tricks.msleep(50);
   }
}

/* make the robot track a charucoboard in real time */
void Routines::monkeySeeMonkeyDo(){
    double x,y,z;
    vector<double> relPos1(3);
    bool getVecs = false;
    int looptieloop = 1;
    int flip = 0;
    double w[3][3]={{0,1,0},    //the target rotation matrix
                    {0,0,1},
                    {1,0,0}};
    Mat cameraMatrix = Mat::eye(3,3, CV_64F);
    Mat distanceCoefficients = Mat::zeros(5,1, CV_64F);
    Mat relativeMatrix = Mat::zeros(3,3, CV_64F);
    CAM.getMatrixFromFile("CameraCalibration720.dat", cameraMatrix, distanceCoefficients);

    thread t(&cam::copyMovement, &CAM, ref(cameraMatrix), ref(distanceCoefficients),ref(relPos1) ,ref(relativeMatrix), ref(getVecs), ref(looptieloop) );
    t.detach();
    looptieloop = tricks.wait();
    getVecs = true;
    while(true){
        unique_lock<mutex> locker(mu);
        cond.wait(locker, [&]{return !getVecs;});
            x = 90*relPos1[0] - 4*relativeMatrix.at<double>(0,1) - 27 ; y = 100*relPos1[1] + 30 - 4*relativeMatrix.at<double>(1,1); z = 100*relPos1[2] + 5 - 6*relativeMatrix.at<double>(2,1);
            //cout << "\r" << " x=" << x << " y=" << y << "  z" << z <<"                   " << flush;
            w[0][0] = relativeMatrix.at<double>(0,2);   w[0][1] = relativeMatrix.at<double>(0,0);   w[0][2] = relativeMatrix.at<double>(0,1);
            w[1][0] = relativeMatrix.at<double>(1,2);   w[1][1] = relativeMatrix.at<double>(1,0);   w[1][2] = relativeMatrix.at<double>(1,1);
            w[2][0] = relativeMatrix.at<double>(2,2);   w[2][1] = relativeMatrix.at<double>(2,0);   w[2][2] = relativeMatrix.at<double>(2,1);
            ik.inverseKinematics(x,y,z,w,angles,flip);
            tricks.commandArduino(angles,10);
            locker.unlock();
            getVecs = true;
            tricks.msleep(20);
    }
}
