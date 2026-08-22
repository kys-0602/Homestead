#include "Homestead/Audio/AudioCodec.hpp"
#include <cstdint>
#include <vector>
int main(){
    std::vector<std::uint8_t> bytes{'H','S','A','2',1,0,0x40,0x1F,4,0,0,0,0,0,40,0,0x55};
    std::vector<std::int16_t> samples;
    if(!Homestead::DecodeAdpcm2(bytes.data(),bytes.size(),samples)||samples.size()!=4) return 1;
    bytes[0]='X'; if(Homestead::DecodeAdpcm2(bytes.data(),bytes.size(),samples)) return 2;
    bytes[0]='H'; bytes.pop_back(); if(Homestead::DecodeAdpcm2(bytes.data(),bytes.size(),samples)) return 3;
    return 0;
}
