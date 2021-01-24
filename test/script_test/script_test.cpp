//*******************************************************************
//    script_test.cpp
//
//    This file contains the test code for script based pldm commands.
//    When run, this file should read in scripts from a text file to
//    send client side PLDM commands.
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
#include <list>
using namespace std;

//*******************************************************************
// splitLine()
//
// This helper function takes a string and splits it into a list of strings
// based on a delimiter.
//
// parameters:
//	  line - the larger string being broken up.
//    delimiter - the character to break on.
// returns:
//    a list of the smaller strings
list<string> splitLine(string line,unsigned char delimiter){
    int delimiterCount=0;
    for(int i=0;i<line.length();i++){
        if(line.at(i)==delimiter) delimiterCount++;
    }
    list<string> splitStrings;
    string currentStr="";
    for(int i=0;i<line.length();i++){
        if(line.at(i)==delimiter){
            splitStrings.push_back(currentStr);
            currentStr="";
        }else{
            currentStr = currentStr+line.at(i);
        }
    }
    return splitStrings;
}

//*******************************************************************
// main()
//
// Main function. Runs executable code.
//
// returns:
//    0
int main() {
    //interpreting script
    fstream script;
    script.open("script.txt",ios::in);
    string line = "default";
    list<string> splitLn;
    if(script.is_open()){
        while(getline(script,line)){
            splitLn = splitLine(line,'/');
            string cmd_type = splitLn.front();
            
            //switch on command type
            if(cmd_type=="REQUEST"){
                cout<<"request"<<endl;
            }else if(cmd_type=="INPUT"){
                cout<<"input"<<endl;
            }else if(cmd_type=="GETENUMCHOICE"){
                cout<<"get enum choice"<<endl;
            }else if(cmd_type=="DISP"){
                cout<<"display"<<endl;
            }else if(cmd_type=="WAIT"){
                cout<<"wait"<<endl;
            }else if(cmd_type=="EXEC"){
                cout<<"execute script"<<endl;
            } 
        }
    }else{
        cout<<"failed read"<<endl;
    }
    script.close();
    return 0;
}