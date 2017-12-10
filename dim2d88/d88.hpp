//
//  d88.hpp
//  dim2d88
//
//  Created by mura on 2017/11/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef d88_hpp
#define d88_hpp

#include <iostream>
#include <string>
#include "common.hpp"

/*
 * d88形式
 */
class D88Track;
class D88
{
public:
    static constexpr int MAX_TRACKS=164;
    static constexpr int HDR_SIZE=0x2b0;
    
    enum class FDType : Byte
    {
        FD2D =0,
        FD2DD=0x10,
        FD2HD=0x20,
    };
    
    using Header=struct{
        Byte name[17];
        Byte reserve[9];
        Byte protect;
        FDType type;
        DWord size;
        DWord table[MAX_TRACKS];
    };
    
public:
    D88(FDType type,const std::string &name);
    virtual ~D88();
    
    //floppy disk
    bool setFDParm(FDType type);
    void setFDParm(int trk,int side,int sector,int secsize,FDType type);
    
    int numTrack() const;
    int sectorSize() const;
    Byte numSector() const;
    FDType fdType() const;
    
    //トラックが存在しないとき、トラックを作成するなら２番めの引数に true を指定
    //未指定（または false）ならNULLが返る
    D88Track* track(int trk,bool createifnull=false);
    
    const Header* header() const;
    
    //ヘッダのtableを確定する
    //ファイル出力をする前にかならずcallすること
    void fixHeader();
    
    bool write(const char *filename);
    
private:
    //static const Byte _fdtype[];
    
protected:
    Header _header;
    D88Track* _track[MAX_TRACKS];
    
    int _numtrack;
    int _numside;
    int _numsector;
    int _sectorsize;
    FDType _type;
};

//ファイル出力
std::ostream& operator<<(std::ostream& str,D88& d88);



/*
 * トラック
 */
//class D88;

class D88Track
{
public:
    static constexpr int D88SECTOR_HDR_SIZE=0x10;
    
    /*
     * セクタサイズ
     */
    enum class SectorSize : Byte{
        S128=0,
        S256=1,
        S512=2,
        S1024=3
    };

    enum class Density : Byte{
        FDENS_D=0,
        FDENS_S=0x40,
        FDENS_HD=1
    };
    
    /*
     *  セクタヘッダ
     */
    using SectorHeader=struct
    {
        Byte track;
        Byte side;
        Byte sector;
        SectorSize sectorsize;
        Word numsector;
        Density density;
        Byte deleted;
        Byte status;
        Byte reserve[5];
        Word size;
    };

public:
    D88Track(D88 *d88,int track);
    virtual ~D88Track();
    
    void setData(const Byte *data,int size);
    
    int sectorSize() const;
    
    const SectorHeader* header() const;
    const Byte* data() const;
    const int dataSize() const;    //データのサイズ
    const int wholeSize() const;    //全体サイズ（ヘッダ＋データ）

    using SectorFunc=std::function<void(const SectorHeader &hdr,Byte &data,int size)>;
    void eachSector(SectorFunc func);
    
    //ファイル出力
    friend std::ostream& operator<<(std::ostream& str,D88Track& sec);

    
private:
//    void initSector();
//    int incrementSector();

private:
    //static const Byte _density[];
    
    bool setSectorSize(int size);
    bool setDensity(D88::FDType type);
    
private:
    SectorHeader _header;
    Byte *_data;
    int _fullsectorsize;
};



#endif /* d88_hpp */
