//*******************************************************************
//    pldm_cmd_test.cpp
//
//    This file contains the test code for PLDM command line input.
//    when run, this file should display a text based user interface
//    that allows the user to send client side PLDM commands.
//
//    More information on the PICMG IoT data model can be found within
//    the PICMG family of IoT specifications.  For more information,
//    please visit the PICMG web site (www.picmg.org)
//
//    Copyright (C) 2020,  PICMG
//
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.
//

#include <iostream>
#include <fstream>
#include <uchar.h>
#include <map>
#include "uart.h"
#include "mctp.h"
#include "pldm.h"
#include "clientNode.h"
#include "PdrRepository.h"

// global variables for communications channel and PLDM data structures
PdrRepository pdrRepository;   // local copy of platform data repository
mctp_struct   mctp1;           // parameters for MCTP communications
clientNode          node1;           // interface to device node (implements PLDM) 
unsigned int  uart_handle;     // handle for uart device

//*******************************************************************
// printMenu()
//
// This helper function prints out the main menu
//
// parameters:
//	  none
// returns:
//    void
static void printMenu1(){
    cout<<"*******************************************************************"<<endl;
    cout<<"Enter Command:"<<endl;
    cout<<"*******************************************************************"<<endl;
    cout<<"1 - Dump PDR"<<endl;
    cout<<"2 - Set Numeric Effecter Value"<<endl;
    cout<<"3 - Set State Effecter State"<<endl;
    cout<<"4 - Get Numeric Effecter Value"<<endl;
    cout<<"5 - Get State Effecter State"<<endl;
    cout<<"6 - Get State Sensor Reading"<<endl;
    cout<<"7 - Get Sensor Reading"<<endl;
    cout<<"8 - Set Numeric Effecter Enable"<<endl;
    cout<<"9 - Set State Effecter Enable"<<endl;
    cout<<"Q - Quit"<<endl;
}

//*******************************************************************
// getUIch()
//
// This helper function gets user input for a char and then returns it.
// This is a blocking function.
//
// parameters:
//	  prompt - the desplayed message prompting for input.
// returns:
//    the user input
static unsigned char getUIch(const char* prompt){
    char ch;
    cout<<prompt<<endl;
    cin>>ch;
    return ch;
}

//*******************************************************************
// getUIstr()
//
// This helper function gets user input for a string and then returns it.
// This is a blocking function.
//
// parameters:
//	  prompt - the desplayed message prompting for input.
// returns:
//    the user input
static string getUIstr(const char* prompt){
    string str;
    cout<<prompt<<endl;
    cin>>str;
    return str;
}

//*******************************************************************
// getUIlong()
//
// This helper function gets user input for a long and then returns it.
// This is a blocking function.
//
// parameters:
//	  prompt - the desplayed message prompting for input.
// returns:
//    the user input
static unsigned long getUIlong(const char* prompt){
    unsigned long lon;
    cout<<prompt<<endl;
    cin>>lon;
    return lon;
}

//*******************************************************************
// getUIdouble()
//
// This helper function gets user input for a double and then returns it.
// This is a blocking function.
//
// parameters:
//	  prompt - the desplayed message prompting for input.
// returns:
//    the user input
static double getUIdouble(const char* prompt){
    double db;
    cout<<prompt<<endl;
    cin>>db;
    return db;
}


//*******************************************************************
// getUIint()
//
// This helper function gets user input for an int and then returns it.
// This is a blocking function.
//
// parameters:
//	  prompt - the desplayed message prompting for input.
// returns:
//    the user input
static int getUIint(const char* prompt){
    int i;
    cout<<prompt<<endl;
    cin>>i;
    return i;
}

//*******************************************************************
// pickEffecter()
//
// This helper function takes an ID for an effecter pdr, searches
// the repository for the pdr, and, if available, returns it.
//
// parameters:
//	  requestedID - the id of the pdr
//    effecterType - the name of the sensor (StateSensor/NumericSensor)
// returns:
//    the pdr, if it exists, or null
static GenericPdr* pickEffecter(unsigned long requestedID, string effecterType){
    unsigned char buffer[256];
        
    int j = 1;
    while(1){
        GenericPdr* effecterPdr;
        effecterPdr = pdrRepository.getPdrFromRecordHandle(j);
        if(!effecterPdr){
            return 0;
        }else{
            if(effecterPdr->keyExists("effecterID")){
                if(effecterPdr->getValue("PDRType")==effecterType){
                    unsigned long effecterID = atol(effecterPdr->getValue("effecterID").c_str());
                    if(effecterID==requestedID) return effecterPdr;
                }
            }
        }
        j++;
    }
}

//*******************************************************************
// pickSensor()
//
// This helper function takes an ID for an sensor pdr, searches
// the repository for the pdr, and, if available, returns it.
//
// parameters:
//	  requestedID - the id of the pdr
//    sensorType - the name of the sensor (StateSensor/NumericSensor)
// returns:
//    the pdr, if it exists, or null
static GenericPdr* pickSensor(unsigned long requestedID, string sensorType){
    unsigned char buffer[256];
        
    int j = 1;
    while(1){
        GenericPdr* sensorPdr;
        sensorPdr = pdrRepository.getPdrFromRecordHandle(j);
        if(!sensorPdr){
            return 0;
        }else{
            if(sensorPdr->keyExists("sensorID")){
                if(sensorPdr->getValue("PDRType")==sensorType){
                    unsigned long sensorID = atol(sensorPdr->getValue("sensorID").c_str());
                    if(sensorID==requestedID) return sensorPdr;
                }    
            }
        }
        j++;
    }
}

//*******************************************************************
// setNumericEffecterMenu()
//
// This helper function generates a CLI menu for the setNumericEffecter function.
// When called, this functon will get user input to find a pdr for an effecter and
// then change the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void setNumericEffecterMenu(){
    // clear terminal
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr;
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"NumericEffecter");
    //check PDR validity
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        double data = getUIdouble("please enter new value for numeric effecter");
        if(node1.setNumericEffecterValue(effecterID,effecterpdr,data)){
            cout<<"effecter value change successful"<<endl;
        }else{
            cout<<"effecter value change failed"<<endl;
        }
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// getNumericEffecterMenu()
//
// This helper function generates a CLI menu for the GetNumericEffecter function.
// When called, this functon will get user input to find a pdr for an effecter and
// then print the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void getNumericEffecterMenu(){
    // clear terminal
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"NumericEffecter");
    // check PDR validity
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        double scaledData = node1.getNumericEffecterValue(effecterID,effecterpdr); 
        if(scaledData!=-1){
            cout<<"Effecter value is: "<<scaledData<<endl;
        }else{
            cout<<"Effecter value change failed"<<endl;
        }
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// setStateEffecterMenu()
//
// This helper function generates a CLI menu for the setStateEffecter function.
// When called, this functon will get user input to find a pdr for an effecter and
// then change the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void setStateEffecterMenu(){
    // clear terminal
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"StateEffecter");
    // check PDR validity
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        map<unsigned int,string> enums = pdrRepository.getStateSet(atoi(effecterpdr->getValue("stateSetID").c_str()));
        cout << "Please select a state" << endl;
        for(auto i:enums){
            cout << i.first << " - " << i.second << endl; 
        }
        enum8 effecterState = getUIint("");
        if(node1.setStateEffecterStates(effecterID,effecterpdr,effecterState)){
            cout<<"Effecter state change successful"<<endl;
        }else{
            cout<<"Effecter state change failed"<<endl;
        }
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// getStateEffecterMenu()
//
// This helper function generates a CLI menu for the GetStateEffecter function.
// When called, this functon will get user input to find a pdr for an effecter and
// then print the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void getStateEffecterMenu(){
    // clear terminal
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"StateEffecter");
    // check PDR validity
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        uint8 value = (uint8)node1.getStateEffecterStates(effecterID,effecterpdr);
        bool valuefound= false;                
        // process the response
        map<unsigned int,string> enums = pdrRepository.getStateSet(atoi(effecterpdr->getValue("stateSetID").c_str()));
        for (std::map<unsigned int,string>::iterator it=enums.begin(); it!=enums.end(); it++){
            if(it->first==value){
                cout << "presentState: " << it->second << endl;
                valuefound = true;
            }
        }
        if(!valuefound) cout<<"Effecter value retrival failed"<<endl;
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// getSensorMenu()
//
// This helper function generates a CLI menu for the GetSensorReadings function.
// When called, this functon will get user input to find a pdr for an effecter and
// then print the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void getSensorMenu(){
    cout<<"\ec"<<endl;
    GenericPdr* sensorpdr; 
    unsigned long sensorID = getUIlong("please enter an sensor ID");
    sensorpdr = pickSensor(sensorID,"NumericSensor");
    // checking ID
    if(!sensorpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        double scaledData=node1.getSensorReading(sensorID,sensorpdr);

        if(scaledData!=-1){
            cout<<"Sensor value is: "<<scaledData<<endl;

        }else{
            cout<<"Sensor value failed"<<endl;
        }

        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// getStateSensorMenu()
//
// This helper function generates a CLI menu for the GetStateSensorReadings function.
// When called, this functon will get user input to find a pdr for an effecter and
// then print the value of the effecter.
//
// parameters:
//	  none
// returns:
//    void
static void getStateSensorMenu(){
    cout<<"\ec"<<endl;
    GenericPdr* sensorpdr; 
    unsigned long sensorID = getUIlong("please enter an sensor ID");
    sensorpdr = pickSensor(sensorID,"StateSensor");
    // checking ID
    if(!sensorpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        uint8 value = (uint8)node1.getStateSensorReadings(sensorID,sensorpdr);
        bool valuefound= false;                
        // process the response
        map<unsigned int,string> enums = pdrRepository.getStateSet(atoi(sensorpdr->getValue("stateSetID").c_str()));
        for (std::map<unsigned int,string>::iterator it=enums.begin(); it!=enums.end(); it++){
            if(it->first==value){
                cout << "presentState: " << it->second << endl;
                valuefound = true;
            }
        }
        if(!valuefound) cout<<"Sensor state retrival failed"<<endl;
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
            
    }
}

//*******************************************************************
// setNumericEnableMenu()
//
// This helper function generates a CLI menu for the setNumericEffecterEnable function.
// When called, this functon will get user input to find a pdr for an effecter and
// then change the value of the effecter's enable state.
//
// parameters:
//	  none
// returns:
//    void
static void setNumericEnableMenu(){
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"NumericEffecter");
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        cout<<"Please enter enable state:"<<endl;
        cout<<"1 - enabled"<<endl;
        cout<<"2 - disabled"<<endl;
        cout<<"3 - unavailable"<<endl;
        uint8 enableState = getUIint("");
        
        if(node1.setNumericEffecterEnable(effecterID,effecterpdr,enableState)){
            cout<<"Effecter enable state change successful"<<endl;
        }else{
            cout<<"Effecter enable state change failed"<<endl;
        }
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;
    }
}

//*******************************************************************
// setStateEnableMenu()
//
// This helper function generates a CLI menu for the setStateEffecterEnables function.
// When called, this functon will get user input to find a pdr for an effecter and
// then change the value of the effecter's enable state.
//
// parameters:
//	  none
// returns:
//    void
static void setStateEnableMenu(){
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,"StateEffecter");
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        cout<<"Please enter enable state:"<<endl;
        cout<<"1 - enabled"<<endl;
        cout<<"2 - disabled"<<endl;
        cout<<"3 - unavailable"<<endl;
        uint8 enableState = getUIint("");
        
        if(node1.setStateEffecterEnables(effecterID,effecterpdr,enableState)){
            cout<<"Effecter enable state change successful"<<endl;
        }else{
            cout<<"Effecter enable state change failed"<<endl;
        }
        getUIch("B - go back to menu");
        cout<<"\ec"<<endl;

    }
}

//*******************************************************************
// main()
//
// Main function. Runs executable code.
//
// returns:
//    0
int main(unsigned int argc, char * argv[]) {
    unsigned char uiQuit = 1;

    // check for the proper number of program arguments
    if (argc!=2) {
        cerr<<"Wrong number of arguments. "<<endl;
        cerr<<"Usage:  pldm_cmd_test port"<<endl;
        return -1;

    }
    // Attempt to initialize the uart connection
    uart_handle = uart_init(argv[1]);
    if (uart_handle<=0) {
        cerr<<"error establishing uart connection."<<endl;
        return -1;
    }

    // initialize the mctp connection and read the PDRs from the device node
    cout<<"Reading metadata file"<<endl;
    if (!pdrRepository.setDictionary("pldm_definitions.json")) {
        cerr<<"error loading dictionary file."<<endl;
        return -1;
    }

    // initialize the mctp connection
    cout<<"Initializing MCTP/PLDM interface"<<endl;
    mctp_struct mctp;
    mctp_init(uart_handle, &mctp);
    node1.init(&mctp);

    // Initialize the pdr repository from the device node
    cout<<"Initializing local PDR repository"<<endl;
    pdrRepository.addPdrsFromNode(node1);

    cout<<"\ec"<<endl;

    // CLI menu start
    while(uiQuit){
        printMenu1();
        unsigned char input;
        input = getUIch("");
        switch(input){
            case '1': // pdr dump
                cout<<"\ec"<<endl;
                pdrRepository.dump();
                getUIch("B - go back to menu");
            break;
            case '2': // set Numeric Effecter
                setNumericEffecterMenu();
            break;
            case '3': // set State Effecter
                setStateEffecterMenu();
            break;
            case '4': // get Numeric Effecter
                getNumericEffecterMenu();
            break;
            case '5': // get State Effecter
                getStateEffecterMenu();
            break;
            case '6': // get State Sensor Reading
                getStateSensorMenu();
            break;
            case '7': // get Sensor Reading
                getSensorMenu();
            break;
            case '8': // set Numeric Effecter Enable
                setNumericEnableMenu();
            break;
            case '9': // set State Effecter Enable
                setStateEnableMenu();
            break;
            case 'Q':
            case 'q':
                //run quit pdr function
                uiQuit=0;
            break;
            default:
                cout<<"\ec"<<endl;
                cout<<"Incorrect input, please try again\n"<<endl;
                getUIch("B - go back to menu");
                cout<<"\ec"<<endl;
            break;
        }
        cout<<"\ec"<<endl;
    }
    
    uart_close(uart_handle);
    return 0;
}
