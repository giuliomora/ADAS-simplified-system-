
#ifndef DEF_H
#define DEF_H

#define BRAKE_BY_WIRE "bin/brakeByWire"
#define FORWARD_FACING_RADAR "bin/forwardFacingRadar"
#define STEER_BY_WIRE "bin/steerByWire"
#define THROTTLE_CONTROL "bin/throttleControl"
#define FRONT_WINDSHIELD_CAMERA "bin/frontWindshieldCamera"
#define HMI_INPUT "bin/hmiInput"
#define HMI_OUTPUT "bin/hmiOutput"
#define PARK_ASSIST "bin/parkAssist"
#define SURROUND_VIEW_CAMERAS "bin/surroundViewCameras"

#define ECU_LOG "log/ECU.log"
#define STEER_LOG "log/steer.log"
#define THROTTLE_LOG "log/throttle.log"
#define BRAKE_LOG "log/brake.log"
#define CAMERA_LOG "log/camera.log"
#define RADAR_LOG "log/radar.log"
#define CAR_SPEED "log/car_speed"
#define ASSIST_LOG "log/assist.log"
#define CAMERAS_LOG "log/cameras.log"

#define FRONT_CAMERA_DATA "res/frontCamera.data"
#define URANDOM "/dev/urandom"
#define URANDOM_ARTIFICIAL "res/urandomARTIFICIALE.binary"
#define RANDOM "/dev/random"
#define RANDOM_ARTIFICIAL "res/randomARTIFICIALE.binary"
#define NORMAL "NORMALE"
#define ARTIFICIAL "ARTIFICIALE"
#define READ 0
#define WRITE 1

struct Component
{
    char *name;
    int fd;
    int pid;
    char buffer[1024];
};

#endif