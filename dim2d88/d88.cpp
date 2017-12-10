//
//  d88.cpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <fstream>

#include "d88.hpp"
#include "string.h"

/*
 * D88Track
 */

D88Track::D88Track(D88 *d88,int track)
{
    _header.track=track>>1;
    _header.side=track&1;
    
    _header.sector=0;
    _fullsectorsize=d88->sectorSize();
    setSectorSize(_fullsectorsize);
    _header.numsector=d88->numSector();

    setDensity(d88->fdType());

    _header.deleted=0;
    _header.status=0;
    _header.size=0;
    _data=nullptr;
    memset(_header.reserve,0,5);
}

D88Track::~D88Track()
{
    if (_data)  delete [] _data;
}

bool D88Track::setSectorSize(int size)
{
    if (size!=128 && size!=256 && size!=512 && size!=1024)  return false;

    _header.sectorsize=static_cast<SectorSize>((size>>7)&0x03);
    return true;
}

bool D88Track::setDensity(D88::FDType type)
{
    switch (type) {
        case D88::FDType::FD2D:
        case D88::FDType::FD2DD:
            _header.density=Density::FDENS_D;
            break;
        case D88::FDType::FD2HD:
            _header.density=Density::FDENS_HD;
            break;
            
        default:
            return false;
    }
    return true;
}

void D88Track::setData(const Byte *data, int size)
{
    if (_data)  delete [] _data;

    int bufsz=_header.numsector*_fullsectorsize;
    _data=new Byte[bufsz];
    _header.size=_fullsectorsize;

    if (data){
        memcpy(_data,data,size);
    }
    else{
        size=0;
    }
    memset(_data+size,0,bufsz-size);
}

int D88Track::sectorSize() const{return(_fullsectorsize);}
const D88Track::SectorHeader* D88Track::header() const{return(&_header);}
const Byte* D88Track::data() const{return(_data);}
const int D88Track::dataSize() const{return(_header.size);}
const int D88Track::wholeSize() const{return((_header.size+D88SECTOR_HDR_SIZE)*_header.numsector);}


void D88Track::eachSector(SectorFunc func)
{
    Byte* dataptr=_data;
    for (Byte sector=1; sector<=_header.numsector; sector++,dataptr+=_fullsectorsize) {
        _header.sector=sector;
        func(_header,*dataptr,_fullsectorsize);
    }
}

std::ostream& operator<<(std::ostream& str,D88Track& trk)
{
    trk.eachSector([&str,&trk](const D88Track::SectorHeader &hdr,Byte &data,int size){
        str.write(reinterpret_cast<const char*>(&hdr),D88Track::D88SECTOR_HDR_SIZE);
        str.write(reinterpret_cast<const char*>(&data),size);
    });
    return(str);
}

/*
 * D88
 */

D88::D88(FDType type, const std::string &name)
{
    setFDParm(type);
    strncpy((char *)&_header.name,name.c_str(),16);
    _header.name[16]=0;
    memset(_header.reserve,0,9);
    _header.protect=0;
    
    for(int i=0;i<MAX_TRACKS;i++){
        _track[i]=nullptr;
    }
}

D88::~D88()
{
    for(int i=0;i<MAX_TRACKS;i++){
        if (_track[i])    delete _track[i];
    }
}

bool D88::setFDParm(FDType type)
{
    switch(type){
        case FDType::FD2D:
            setFDParm(40,2,16,256,type);break;
        case FDType::FD2DD:
            setFDParm(80,2,16,256,type);break;
        case FDType::FD2HD:
            setFDParm(77,2,8,1024,type);break;
     default:
            return false;
    }
    return true;
}

void D88::setFDParm(int trk, int side, int sector, int secsize,FDType type)
{
    _numtrack=trk;
    _numside=side;
    _numsector=sector;
    _sectorsize=secsize;
    _header.type=type;
    _type=type;
}

int D88::numTrack() const{return(_numtrack);}
int D88::sectorSize() const{return(_sectorsize);}
Byte D88::numSector() const{return(_numsector);}
D88::FDType D88::fdType() const{return(_type);}
const D88::Header* D88::header() const{return(&_header);}



D88Track* D88::track(int trk,bool createifnull)
{
    if (trk<0 || trk>=MAX_TRACKS)   return nullptr;
    if (!_track[trk] && createifnull){
        _track[trk]=new D88Track(this,trk);
    }
    return(_track[trk]);
}



void D88::fixHeader()
{
    DWord secptr=HDR_SIZE;
    for(int i=0;i<MAX_TRACKS;i++){
        if (_track[i]){
            _header.table[i]=secptr;
            secptr+=_track[i]->wholeSize();
        }
        else{
            _header.table[i]=0;
        }
    }
    _header.size=secptr;
}

bool D88::write(const char *filename)
{
    std::ofstream out;
    out.open(filename,std::ios_base::binary);
    if (!out)   throw std::runtime_error("file write error");
    out<<*this;
    out.close();
    return true;
}

std::ostream& operator<<(std::ostream& str,D88& d88)
{
    d88.fixHeader();
    
    D88Track *trk;
    str.write(reinterpret_cast<const char*>(d88.header()),D88::HDR_SIZE);
    for(int i=0;i<D88::MAX_TRACKS;i++){
        trk=d88.track(i);
        if (trk){
            str<<(*trk);
        }
    }
    return(str);
}

