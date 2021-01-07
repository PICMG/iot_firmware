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


static void printMenu1(){
    cout<<"*******************************************************************\n"<<endl;
    cout<<"Enter Option:\n"<<endl;
    cout<<"*******************************************************************\n"<<endl;
    cout<<"1 - Dump PDR\n"<<endl;
    cout<<"2 - Effecter #\n"<<endl;
    cout<<"3 - Sensor #\n"<<endl;
    cout<<"Q - Quit\n"<<endl;
}

static unsigned char getUIch(const char* prompt){
    char ch;
    cout<<prompt<<endl;
    cin>>ch;
    return ch;
}

static string getUIstr(const char* prompt){
    string str;
    cout<<prompt<<endl;
    cin>>str;
    return str;
}

static unsigned long getUIlong(const char* prompt){
    unsigned long lon;
    cout<<prompt<<endl;
    cin>>lon;
    return lon;
}

static double getUIdouble(const char* prompt){
    double db;
    cout<<prompt<<endl;
    cin>>db;
    return db;
}

unsigned int dumpPDR(unsigned int uart_handle){
    unsigned char buffer[256];
    node node1;
    
    // load the dictionary from the specified file.  The dictionary
    // includes meta-data that helps this program interpret the 
    // information it receives from the server node.
    PdrRepository pdrRepository;
    if (!pdrRepository.setDictionary("pldm_definitions.json")) return -1;

    // initialize the mctp connection
    mctp_struct mctp;
    mctp_init(uart_handle, &mctp);
    node1.init(&mctp);

    // get the repository information by sending the request through
    // the mctp interface
    PldmRequestHeader hdr;
    hdr.flags1 = 0; hdr.flags2 = 0; hdr.command = CMD_GET_PDR_REPOSITORY_INFO;
    node1.putCommand(&hdr, buffer, 0);

    // wait for the packet to be available, processing bytes as they come in
    while(mctp_isPacketAvailable(&mctp)==0){
        mctp_updateRxFSM(&mctp);
    }

    unsigned char *response = mctp_getPacket(&mctp);
    PldmResponseHeader* rxHeader = (PldmResponseHeader*)response;
    GetPdrRepositoryInfoResponse* infoResponse = (GetPdrRepositoryInfoResponse*)(response + sizeof(PldmResponseHeader)-1);
    if (infoResponse->completionCode != RESPONSE_SUCCESS) {
        std::cout << "Error Getting PDR Info" << std::endl;
        return -1;
    }
    cout << "PDR Info:" << endl;
    cout << "   Records       " << infoResponse->recordCount << endl;
    cout << "   MaxRecordSize " << infoResponse->largestRecordSize << endl;
    cout << "   TotalSize     " << infoResponse->repositorySize << endl;
    cout << endl;

    pdrRepository.addPdrsFromNode(node1);

    pdrRepository.dump();
    return 1;
}

static GenericPdr* pickEffecter(unsigned long requestedID, unsigned int uart_handle){
    unsigned char buffer[256];
    node node1;
    
    // load the dictionary from the specified file.  The dictionary
    // includes meta-data that helps this program interpret the 
    // information it receives from the server node.
    PdrRepository pdrRepository;
    if (!pdrRepository.setDictionary("pldm_definitions.json")) return 0;

    // initialize the mctp connection
    mctp_struct mctp;
    mctp_init(uart_handle, &mctp);
    node1.init(&mctp);

    pdrRepository.addPdrsFromNode(node1);
    
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


static void setNumericEffecterMenu(unsigned int uart_handle){
    cout<<"\ec"<<endl;
    GenericPdr* effecterpdr; 
    unsigned long effecterID = getUIlong("please enter an effecter ID");
    effecterpdr = pickEffecter(effecterID,uart_handle);
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
        double resolution = atol(effecterpdr->getValue("resolution").c_str());
        double offset     = atol(effecterpdr->getValue("offset").c_str());
        data = (data - offset)/resolution;
        unsigned int body_len = 0;

        PldmRequestHeader header;
        header.flags1 = 0; header.flags2 = 0; header.command = CMD_SET_NUMERIC_EFFECTER_VALUE;
        *((uint16*)buffer) = effecterID;

        if(dataSize=="uint8"){
            buffer[2] = 0; //effecter data size is uint8
            buffer[3] = (uint8)data;
            body_len = 4;
        }
        else if(dataSize=="sint8"){
            buffer[2] = 1; //effecter data size is sint8
            buffer[3] = (sint8)data;
            body_len = 4;
        }
        else if(dataSize=="uint16"){
            buffer[2] = 2; //effecter data size is uint16
            *((uint16*)(&buffer[3])) = (uint16)data;
            body_len = 5;
        }
        else if(dataSize=="sint16"){
            buffer[2] = 3; //effecter data size is sint16
            *((sint16*)(&buffer[3])) = (sint16)data;
            body_len = 5;
        }
        else if(dataSize=="uint32"){
            buffer[2] = 4; //effecter data size is uint32
            *((uint32*)(&buffer[3])) = (uint32)data;
            body_len = 7;
        }
        else if(dataSize=="sint32"){
            buffer[2] = 5; //effecter data size is sint32
            *((sint32*)(&buffer[3])) = (sint32)data;
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


// there will be a different version of this for every type of sensor/effector
void effecterTypeAMenu(){
    //placeholder code... WIP

    //print options
    //get ui
    //switch case commmands
}


//*******************************************************************
// main()
//
// Main function. Runs executable code.
//
// returns:
//    0
int main(){
    unsigned char uiQuit = 1;

    // initialize the uart connection
    unsigned int uart_handle = uart_init("/dev/ttyUSB0");
    if (uart_handle<=0) {
        std::cerr<<"error establishing uart connection."<<endl;
        return -1;
    }

    while(uiQuit){
        printMenu1();
        unsigned char input;
        input = getUIch("");
        switch(input){
            case '1':
                cout<<"\ec"<<endl;
                dumpPDR(uart_handle);
                getUIch("B - go back to menu");
            break;
            case '2':
                setNumericEffecterMenu(uart_handle);
                getUIch("B - go back to menu");
            break;
            case '3':
                //run sensor pdr function
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
    return 0;
}
