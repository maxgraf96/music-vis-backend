//
// Created by Max on 11/06/2020.
//

#ifndef MUSIC_VIS_BACKEND_BINARYDATA_H
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sys/stat.h>
#define MUSIC_VIS_BACKEND_BINARYDATA_H

using namespace std;

namespace MyBinaryData {
    static const char* getMagicXML(){
//        char * dir = getcwd(NULL, 0); // Platform-dependent, see reference link below
//        DBG("Current dir: ");
//        DBG(dir);

        auto name = "music-vis-backend.xml";
        // Get file size
        std::ifstream in(name, ifstream::ate | ifstream::binary);
        int size = in.tellg();

        if (size > 0){
            ifstream xml_file;
            xml_file.open(name, ios::in | ios::binary);
            char* buffer = new char[size];
            xml_file.read(buffer, size);

            return buffer;
        }
        else{
            DBG("Couldn't read XML file size");
        }

        return nullptr;
    };

    static const int getMagicXMLSize(){
        auto name = "music-vis-backend.xml";
        // Get file size
        std::ifstream in(name, ifstream::ate | ifstream::binary);
        return static_cast<int>(in.tellg());
    }
};


#endif //MUSIC_VIS_BACKEND_BINARYDATA_H
