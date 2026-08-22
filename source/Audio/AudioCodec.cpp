#include "Homestead/Audio/AudioCodec.hpp"
#include <algorithm>
namespace Homestead {
bool DecodeAdpcm2(const std::uint8_t* data,std::size_t size,std::vector<std::int16_t>& samples) noexcept {
    constexpr int steps[]={16,18,20,22,25,28,31,35,39,44,49,55,62,69,78,87,98,110,123,138,155,174,195,219,246,276,310,348,391,439,493,553,621,697,782,878,986,1107,1243,1395,1566,1758,1973,2215,2486,2790,3132,3516,3947,4431,4975,5585,6270,7040,7904,8874,9963,11188,12561,14102,15836,17783,19970,22426,25184,28281,31759};
    if (!data || size<17 || data[0]!='H'||data[1]!='S'||data[2]!='A'||data[3]!='2'||
        data[4]!=1||data[5]!=0||data[6]!=0x40||data[7]!=0x1F||data[12]!=0||data[13]!=0||data[14]!=40||data[15]!=0) return false;
    const std::uint32_t count=data[8]|(static_cast<std::uint32_t>(data[9])<<8U)|
        (static_cast<std::uint32_t>(data[10])<<16U)|(static_cast<std::uint32_t>(data[11])<<24U);
    if (count==0 || 16ULL+(count+3ULL)/4ULL!=size) return false;
    samples.resize(count); int predictor=0,index=40;
    for(std::uint32_t i=0;i<count;++i){const std::uint8_t code=(data[16+i/4]>>((i&3U)*2U))&3U;
        const int magnitude=code&1U; const int delta=steps[index]/4+(magnitude?steps[index]/2:0);
        predictor=std::clamp(predictor+((code&2U)?-delta:delta),-32768,32767);
        index=std::clamp(index+(magnitude?2:-1),0,67); samples[i]=static_cast<std::int16_t>(predictor);}
    return true;
}
}
