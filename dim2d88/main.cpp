//
//  main.cpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include <fstream>
#include <string>

#include "dim.hpp"
#include "d88.hpp"

using namespace std;

void setFileExtension(string &fname,const string &ext);

int main(int argc, const char * argv[]) {
    string outfilename;
    
    switch (argc) {
        case 1:
            cout<<"usage: dim2d88 <dim file name> [<d88 file name>]"<<endl;
            exit(4);
        
        case 2:
            outfilename=argv[1];
            setFileExtension(outfilename,".d88");
            break;
            
        default:
            outfilename=argv[2];
    }

    cout<<"convert "<<argv[1]<<" to "<<outfilename<<endl;
    
    try {
        //read dim image
        DimFile dim(argv[1]);
        if (dim.type()!=DimFile::FDType::FT_2HD)    throw std::runtime_error("this file not supported");
        
        //set image into d88
        D88 d88(D88::FDType::FD2HD,"hoge");
        D88Track *track;
        dim.eachTrack([&d88,&track](int trk,int trksize,const Byte *data){
            track=d88.track(trk,true);
            if (track){
                track->setData(data, trksize);
            }
        });
        
        //write d88 image
        d88.write(outfilename.c_str());

    } catch (std::exception &e) {
        std::cerr<<e.what()<<std::endl;
        return 8;
    }
    
    cout<<"finished successfully"<<endl;
    return 0;
}

void setFileExtension(string &fname,const string &ext)
{
    auto pos=fname.find_last_of('.');
    if (pos!=string::npos){
        fname.replace(pos, fname.length()-pos, ext);
    }
    else{
        fname.append(".").append(ext);
    }
}
