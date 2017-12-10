//
//  dim.hpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef dim_hpp
#define dim_hpp

#include <iostream>
#include "common.hpp"

class DimFile
{
public:
    static constexpr int MAX_TRACK=170;                 //maximum number of tracks
    static constexpr int MAX_DATA_SIZE=MAX_TRACK*9*1024; //maximum dim data size
    static constexpr Byte INIT_VALUE=0xe5;              //initial value of disk image
    
    // disk type
    enum class FDType : Byte{
        FT_2HD=0,
        FT_2HS=1,
        FT_2HC=2,
        FT_2HDE=3,
        FT_2HQ=9,
    };
    
    //dim file header
    using Header=struct{
        FDType type;
        Byte trkflag[MAX_TRACK];
        Byte info[15];
        Byte date[4];
        Byte time[4];
        Byte comment[61];
        Byte overtrack;
    };

public:
    DimFile() noexcept;
    DimFile(const char *filename) noexcept(false);
    
    virtual ~DimFile()=default;
    
    //read dim file
    void read(const char *filename) noexcept(false);
    friend std::istream& operator>>(std::istream& in,DimFile &dim) noexcept(false);
    
    //get property
    int trackSize() const;
    const Byte* trackData(int trk) const;
    FDType type() const;

    bool isValidFDType() const;

    //
    using TrackFunc=std::function<void(int trk,int trksize,const Byte* data)>;
    void eachTrack(TrackFunc func);
    
private:
    void init();
    
    static const int _trksize[];    //FDType->track size
    
    int _maxtrk;        //maximum track number
    Header _header;     //header
    ByteVector _data;   //row data
};

#endif /* dim_hpp */
