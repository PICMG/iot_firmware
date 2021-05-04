#include <iostream>
#include <string>
#include <map>
#include <list>
#include <dirent.h>
#include <chrono>
#include "uart.h"
#include "mctp.h"
#include "clientNode.h"
#include "PdrRepository.h"
#include "PldmEntity.h"
#include "Terminus.h"
#include "PldmEngine.h"

using namespace std;

void dumpResults(map<int,string> results) {
    for (map<int, string>::iterator it = results.begin(); it!=results.end(); it++) {
        cout<<(*it).first<<": "<<(*it).second<<endl;
    }
}

//*******************************************************************
// main entry point.
// this function attempts to detect PICMG IoT devices attached to 
// a subset of serial ports (specified by the command line parameter).
//
// The process is as follows:
// I. For each port:
//    A. Initialize the uart connection
//    B. attempt a PLDM getTID command.  If the command completes
//        1. set the TID using the setTID command
//        2. remove any pdr records from the repository that are
//           associated with the TID.
//        3. read the PDR records into the local PDR.
// 
// once the ports are initialized, the discovery agent will periodically
// poll the ports to see if there have been any changes.  The process is
// the same as discovery.
int main(int argc, char *argv[]) {
    PldmEngine pldmengine;

    // check the number of arguments
    if (argc!=2) {
        cerr<<"Wrong number of arguments."<<endl;
        cerr<<"Usage: discovery devpath"<<endl;
        cerr<<"devpath is the path to the devices to check along with the beginning"<<endl;
        cerr<<"part of the device name to match."<<endl;
        cerr<<"Examples:"<<endl;
        cerr<<"    /dev/ttyUSB will match all devices that start with ttyUSB"<<endl;
        cerr<<"    /dev/       will match all devices in the dev folder"<<endl;
        return -1;
    }

    pldmengine.init(argv[1]);
    pldmengine.showMap(cout);
    map<int, string> parameters = {{0,"1"}};

    cout<<"Getting Pfinal Value:"<<endl;
    map<int, string> getEffecterParameters;
    // pfinal   - 4
    dumpResults(pldmengine.sendPldmCommand("GET_NUMERIC_EFFECTER_VALUE", "System/IO_module_1/MotionController_1/NumericEffecter_4",getEffecterParameters));
    cout<<endl;

    cout<<"Setting New PFinal = 500000:"<<endl;
    map<int, string> setEffecterParameters = {{0,"sint32"},{1,"500000"}};
    // pfinal   - 4
    dumpResults(pldmengine.sendPldmCommand("SET_NUMERIC_EFFECTER_VALUE", "System/IO_module_1/MotionController_1/NumericEffecter_4",setEffecterParameters));
    cout<<endl;

    cout<<"Verifying New PFinal:"<<endl;
    dumpResults(pldmengine.sendPldmCommand("GET_NUMERIC_EFFECTER_VALUE", "System/IO_module_1/MotionController_1/NumericEffecter_4",getEffecterParameters));
    cout<<endl;

    cout<<"Starting Motor"<<endl;
    map<int, string> parameters2 = {{0,"1"},{1,"requestSet"},{2,"Run"}};
    dumpResults(pldmengine.sendPldmCommand("SET_STATE_EFFECTER_STATES", "System/IO_module_1/MotionController_1/StateEffecter_7",parameters2));
    cout<<endl;

    map<int, string> results;
    results = pldmengine.sendPldmCommand("GET_STATE_SENSOR_READINGS", "System/IO_module_1/MotionController_1/StateSensor_5",parameters);
    while (results.find(4)->second!="Done") {
        cout<<results.find(4)->second<<endl;
        results.clear();
        results = pldmengine.sendPldmCommand("GET_STATE_SENSOR_READINGS", "System/IO_module_1/MotionController_1/StateSensor_5",parameters);
    }

    cout<<"Motor Done - Sending Stop Command"<<endl;
    map<int, string> parameters3 = {{0,"1"},{1,"requestSet"},{2,"Stop"}};
    dumpResults(pldmengine.sendPldmCommand("SET_STATE_EFFECTER_STATES", "System/IO_module_1/MotionController_1/StateEffecter_7",parameters3));
    cout<<endl;
    
    results = pldmengine.sendPldmCommand("GET_STATE_SENSOR_READINGS", "System/IO_module_1/MotionController_1/StateSensor_5",parameters);
    cout<<results.find(4)->second<<endl;
 
    return 0;
}