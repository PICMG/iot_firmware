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
#include <iostream>
#include <uchar.h>
#include <map>
#include "uart.h"
#include "mctp.h"
#include "pldm.h"
#include "node.h"
#include "PdrRepository.h"

// global variables for communications channel and PLDM data structures
PdrRepository pdrRepository;   // local copy of platform data repository
mctp_struct   mctp1;           // parameters for MCTP communications
node          node1;           // interface to device node (implements PLDM) 
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
    cout<<"*******************************************************************\n"<<endl;
    cout<<"Enter Option:\n"<<endl;
    cout<<"*******************************************************************\n"<<endl;
    cout<<"1 - Dump PDR\n"<<endl;
    cout<<"2 - Set Numeric Effecter Value\n"<<endl;
    cout<<"3 - Set State Effecter State\n"<<endl;
    cout<<"Q - Quit\n"<<endl;
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
// returns:
//    the pdr, if it exists, or null
static GenericPdr* pickEffecter(unsigned long requestedID){
    unsigned char buffer[256];
        
    int j = 1;
    while(1){
        GenericPdr* effecterPdr;
        effecterPdr = pdrRepository.getPdrFromRecordHandle(j);
        if(!effecterPdr){
            return 0;
        }else{
            if(effecterPdr->keyExists("effecterID")){
                unsigned long effecterID = atol(effecterPdr->getValue("effecterID").c_str());
                if(effecterID==requestedID) return effecterPdr;
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
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID);
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        mctp_struct mctp1;
        mctp_init(uart_handle,&mctp1);
        node node1;
        node1.init(&mctp1);

        unsigned char buffer[7]; 
        double data = getUIdouble("please enter new value for numeric effecter");
        string dataSize   = effecterpdr->getValue("effecterDataSize");
        double resolution = atof(effecterpdr->getValue("resolution").c_str());
        double offset     = atof(effecterpdr->getValue("offset").c_str());
        
        // perform the unit conversion (with rounding)
        unsigned long scaledData = (data - offset)/resolution + 0.5;
        unsigned int body_len = 0;

        PldmRequestHeader header;
        header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_VALUE;
        *((uint16*)buffer) = effecterID;

        if(dataSize=="uint8"){
            buffer[2] = 0; //effecter data size is uint8
            buffer[3] = (uint8)scaledData;
            body_len = 4;
        }
        else if(dataSize=="sint8"){
            buffer[2] = 1; //effecter data size is sint8
            buffer[3] = (sint8)scaledData;
            body_len = 4;
        }
        else if(dataSize=="uint16"){
            buffer[2] = 2; //effecter data size is uint16
            *((uint16*)(&buffer[3])) = (uint16)scaledData;
            body_len = 5;
        }
        else if(dataSize=="sint16"){
            buffer[2] = 3; //effecter data size is sint16
            *((sint16*)(&buffer[3])) = (sint16)scaledData;
            body_len = 5;
        }
        else if(dataSize=="uint32"){
            buffer[2] = 4; //effecter data size is uint32
            *((uint32*)(&buffer[3])) = (uint32)scaledData;
            body_len = 7;
        }
        else if(dataSize=="sint32"){
            buffer[2] = 5; //effecter data size is sint32
            *((sint32*)(&buffer[3])) = (sint32)scaledData;
            body_len = 7;
        }

        node1.putCommand(&header, buffer, body_len);

        PldmResponseHeader* response;
        response = (PldmResponseHeader*)node1.getResponse();
        if(response->completionCode==RESPONSE_SUCCESS){
            cout<<"Effecter value change successful"<<endl;
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
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID);
    if(!effecterpdr){
        cout<<"Invalid ID"<<endl;
    }else{
        cout<<"valid ID"<<endl;
        
        mctp_struct mctp1;
        mctp_init(uart_handle,&mctp1);
        node node1;
        node1.init(&mctp1);

        map<unsigned int,string> enums = pdrRepository.getStateSet(atoi(effecterpdr->getValue("stateSetID").c_str()));
        cout << "Please select a state" << endl;
        for(auto i:enums){
            cout << i.first << " - " << i.second << endl; 
        }
        
        // send command
        unsigned char buffer[5]; 
        unsigned int body_len = 5;
        
        PldmRequestHeader header;
        header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_STATE_EFFECTER_STATES;
        *((uint16*)buffer) = effecterID;

        uint8 compositeEffecterCount = 1;
        enum8 setRequest = 1;
        enum8 effecterState = getUIint("");

        buffer[2] = compositeEffecterCount;
        buffer[3] = setRequest;
        buffer[4] = effecterState;

        node1.putCommand(&header, buffer, body_len);

        // get response
        PldmResponseHeader* response;
        response = (PldmResponseHeader*)node1.getResponse();
        if(response->completionCode==RESPONSE_SUCCESS){
            cout<<"Effecter state change successful"<<endl;
        }else{
            cout<<"Effecter state change failed"<<endl;
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
