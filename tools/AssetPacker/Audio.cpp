#include "Audio.hpp"

#include <algorithm>
#include <array>
#include <cstdint>
#include <fstream>
#include <limits>

namespace Homestead::AssetPacker {
namespace {

constexpr std::array<int, 68> Steps{
    16,18,20,22,25,28,31,35,39,44,49,55,62,69,78,87,98,110,123,138,155,174,
    195,219,246,276,310,348,391,439,493,553,621,697,782,878,986,1107,1243,1395,
    1566,1758,1973,2215,2486,2790,3132,3516,3947,4431,4975,5585,6270,7040,7904,
    8874,9963,11188,12561,14102,15836,17783,19970,22426,25184,28281,31759};

std::uint16_t U16(const std::uint8_t* p) { return p[0] | static_cast<std::uint16_t>(p[1] << 8U); }
std::uint32_t U32(const std::uint8_t* p) { return p[0] | (p[1]<<8U) | (p[2]<<16U) | (p[3]<<24U); }
void Put16(std::vector<std::uint8_t>& b, std::uint16_t v) { b.push_back(static_cast<std::uint8_t>(v)); b.push_back(static_cast<std::uint8_t>(v>>8U)); }
void Put32(std::vector<std::uint8_t>& b, std::uint32_t v) { Put16(b, static_cast<std::uint16_t>(v)); Put16(b, static_cast<std::uint16_t>(v>>16U)); }

std::uint8_t Encode(std::int16_t sample, int& predictor, int& index) {
    const int difference = static_cast<int>(sample) - predictor;
    const int magnitude = std::abs(difference) >= Steps[index] / 2 ? 1 : 0;
    const int delta = Steps[index] / 4 + (magnitude != 0 ? Steps[index] / 2 : 0);
    predictor = std::clamp(predictor + (difference < 0 ? -delta : delta), -32768, 32767);
    index = std::clamp(index + (magnitude != 0 ? 2 : -1), 0, static_cast<int>(Steps.size() - 1));
    return static_cast<std::uint8_t>((difference < 0 ? 2 : 0) | magnitude);
}

} // namespace

bool BuildAdpcm2(const std::filesystem::path& inputPath, std::vector<std::uint8_t>& output,
                 std::string& error) {
    std::ifstream stream(inputPath, std::ios::binary | std::ios::ate);
    if (!stream) { error = "cannot open audio " + inputPath.string(); return false; }
    const std::streamoff length = stream.tellg();
    if (length < 44 || length > 64 * 1024 * 1024) { error = "invalid audio size"; return false; }
    std::vector<std::uint8_t> wave(static_cast<std::size_t>(length));
    stream.seekg(0); stream.read(reinterpret_cast<char*>(wave.data()), length);
    if (!stream || !std::equal(wave.begin(), wave.begin()+4, "RIFF") ||
        !std::equal(wave.begin()+8, wave.begin()+12, "WAVE")) { error = "invalid WAV"; return false; }
    std::uint16_t channels=0, bits=0, format=0; std::uint32_t rate=0, dataOffset=0, dataSize=0;
    std::size_t offset=12;
    while (offset + 8 <= wave.size()) {
        const std::uint32_t size=U32(wave.data()+offset+4); const std::size_t payload=offset+8;
        if (payload > wave.size() || size > wave.size()-payload) { error="invalid WAV chunk"; return false; }
        if (std::equal(wave.begin()+offset,wave.begin()+offset+4,"fmt ") && size>=16) {
            format=U16(wave.data()+payload); channels=U16(wave.data()+payload+2);
            rate=U32(wave.data()+payload+4); bits=U16(wave.data()+payload+14);
        } else if (std::equal(wave.begin()+offset,wave.begin()+offset+4,"data")) {
            dataOffset=static_cast<std::uint32_t>(payload); dataSize=size; break;
        }
        offset=payload+size+(size&1U);
    }
    if (format!=1 || (channels!=1 && channels!=2) || bits!=16 || rate<8000 || dataSize==0) {
        error="audio must be 16-bit PCM mono/stereo WAV"; return false;
    }
    const std::uint32_t inputFrames=dataSize/(channels*2U);
    constexpr std::uint32_t outputRate=8000;
    const std::uint32_t sampleCount=static_cast<std::uint32_t>(
        static_cast<std::uint64_t>(inputFrames)*outputRate/rate);
    if (sampleCount==0) { error="empty audio"; return false; }
    output.clear(); output.insert(output.end(), {'H','S','A','2'}); Put16(output,1); Put16(output,outputRate);
    Put32(output,sampleCount); Put16(output,0); output.push_back(40); output.push_back(0);
    output.resize(16U+(sampleCount+3U)/4U,0);
    int predictor=0, stepIndex=40;
    for (std::uint32_t out=0; out<sampleCount; ++out) {
        std::uint32_t begin=static_cast<std::uint32_t>(static_cast<std::uint64_t>(out)*rate/outputRate);
        std::uint32_t end=static_cast<std::uint32_t>(static_cast<std::uint64_t>(out+1U)*rate/outputRate);
        if (end<=begin) end=begin+1;
        std::int64_t sum=0; std::uint32_t values=0;
        for (std::uint32_t frame=begin; frame<end && frame<inputFrames; ++frame) {
            for (std::uint16_t channel=0; channel<channels; ++channel) {
                const std::size_t p=dataOffset+(static_cast<std::size_t>(frame)*channels+channel)*2U;
                sum += static_cast<std::int16_t>(U16(wave.data()+p)); ++values;
            }
        }
        const auto sample=static_cast<std::int16_t>(sum/static_cast<std::int64_t>(values));
        output[16U+out/4U] |= static_cast<std::uint8_t>(Encode(sample,predictor,stepIndex)<<((out&3U)*2U));
    }
    return true;
}

} // namespace Homestead::AssetPacker
