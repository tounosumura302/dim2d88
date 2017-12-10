//
//  dim.cpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <fstream>
#include "dim.hpp"


const int DimFile::_trksize[]={
    8192,   //2HD
    9216,   //2HS
    7680,   //2HC
    9216,   //2HDE
    0,0,0,0,0,
    9216    //2HQ
};



DimFile::DimFile() noexcept
{
    init();
}

DimFile::DimFile(const char *filename)
{
    init();
    read(filename);
}

/*
DimFile::~DimFile()
{
}
*/

void DimFile::init()
{
    _data.reserve(MAX_DATA_SIZE);
    _data.resize(MAX_DATA_SIZE);
    _maxtrk=0;
}

void DimFile::read(const char *filename)
{
    std::ifstream infile;
    infile.open(filename,std::ios_base::binary);
    if (!infile)    throw std::runtime_error("cannot open");
    
    infile>>*this;
    if (!infile)    throw std::runtime_error("file read error");
    
    infile.close();
}

std::istream& operator>>(std::istream& in,DimFile &dim)
{
    //read header
    in.read(reinterpret_cast<char*>(&(dim._header)),sizeof(DimFile::Header));
    if (!in)    throw std::runtime_error("dim header read error");
    if (!dim.isValidFDType())   throw std::runtime_error("dim header invalid");
    
    //read data
    Byte* ptr=dim._data.data();
    int trksize=dim.trackSize();
    dim._maxtrk=0;
    for(int i=0;i<DimFile::MAX_TRACK;i++,ptr+=trksize){
        if (dim._header.trkflag[i]){
            in.read(reinterpret_cast<char*>(ptr), trksize);
            if (!in)    throw std::runtime_error("dim image read error");
            dim._maxtrk=i;
        }
        else{
            memset(ptr, DimFile::INIT_VALUE, trksize);
        }
    }
    
    return(in);
}

int DimFile::trackSize() const
{
    return(_trksize[static_cast<int>(_header.type)]);
}

const Byte* DimFile::trackData(int trk) const
{
    if (trk<0 || trk>_maxtrk)   return nullptr;
    return(_data.data()+trk*trackSize());
}


void DimFile::eachTrack(TrackFunc func)
{
    int trksize=trackSize();
    for(int i=0;i<=_maxtrk;i++){
        func(i,trksize,trackData(i));
    }
}

DimFile::FDType DimFile::type() const
{
    return _header.type;
}

bool DimFile::isValidFDType() const
{
    //header.type
    switch (_header.type) {
        case FDType::FT_2HC:
        case FDType::FT_2HD:
        case FDType::FT_2HDE:
        case FDType::FT_2HQ:
        case FDType::FT_2HS:
            break;
            
        default:
            return false;
    }
    
    return true;
}
