param(
    [Parameter(Mandatory = $true)][string]$InputPath,
    [Parameter(Mandatory = $true)][string]$EncodedPath,
    [Parameter(Mandatory = $true)][string]$PreviewPath
)

$ErrorActionPreference = "Stop"

Add-Type -TypeDefinition @'
using System;
using System.IO;
using System.Text;

public static class Adpcm2Preview {
    static readonly int[] Steps = {
        16,18,20,22,25,28,31,35,39,44,49,55,62,69,78,87,
        98,110,123,138,155,174,195,219,246,276,310,348,391,439,493,553,
        621,697,782,878,986,1107,1243,1395,1566,1758,1973,2215,2486,2790,
        3132,3516,3947,4431,4975,5585,6270,7040,7904,8874,9963,11188,12561,
        14102,15836,17783,19970,22426,25184,28281,31759
    };

    static short Clamp(int value) {
        return (short)Math.Max(short.MinValue, Math.Min(short.MaxValue, value));
    }

    static byte EncodeSample(short sample, ref int predictor, ref int stepIndex) {
        int difference = sample - predictor;
        int sign = difference < 0 ? 2 : 0;
        int magnitude = Math.Abs(difference) >= Steps[stepIndex] / 2 ? 1 : 0;
        int delta = Steps[stepIndex] / 4 + (magnitude != 0 ? Steps[stepIndex] / 2 : 0);
        predictor = Clamp(predictor + (sign != 0 ? -delta : delta));
        stepIndex = Math.Max(0, Math.Min(Steps.Length - 1, stepIndex + (magnitude != 0 ? 2 : -1)));
        return (byte)(sign | magnitude);
    }

    static short DecodeSample(byte code, ref int predictor, ref int stepIndex) {
        int magnitude = code & 1;
        int delta = Steps[stepIndex] / 4 + (magnitude != 0 ? Steps[stepIndex] / 2 : 0);
        predictor = Clamp(predictor + ((code & 2) != 0 ? -delta : delta));
        stepIndex = Math.Max(0, Math.Min(Steps.Length - 1, stepIndex + (magnitude != 0 ? 2 : -1)));
        return (short)predictor;
    }

    public static void Convert(string inputPath, string encodedPath, string previewPath) {
        byte[] wave = File.ReadAllBytes(inputPath);
        if (wave.Length < 44 || Encoding.ASCII.GetString(wave, 0, 4) != "RIFF" ||
            Encoding.ASCII.GetString(wave, 8, 4) != "WAVE") throw new InvalidDataException("Not a RIFF WAVE file.");
        int offset = 12, channels = 0, sampleRate = 0, bits = 0, dataOffset = 0, dataSize = 0;
        while (offset + 8 <= wave.Length) {
            string id = Encoding.ASCII.GetString(wave, offset, 4);
            int size = BitConverter.ToInt32(wave, offset + 4);
            int payload = offset + 8;
            if (size < 0 || payload + size > wave.Length) throw new InvalidDataException("Invalid WAVE chunk.");
            if (id == "fmt ") {
                if (BitConverter.ToInt16(wave, payload) != 1) throw new InvalidDataException("Only PCM is supported.");
                channels = BitConverter.ToInt16(wave, payload + 2);
                sampleRate = BitConverter.ToInt32(wave, payload + 4);
                bits = BitConverter.ToInt16(wave, payload + 14);
            } else if (id == "data") { dataOffset = payload; dataSize = size; break; }
            offset = payload + size + (size & 1);
        }
        if (channels != 2 || bits != 16 || sampleRate <= 0 || dataSize == 0)
            throw new InvalidDataException("Expected stereo 16-bit PCM.");

        int inputFrames = dataSize / 4;
        const int outputRate = 8000;
        int outputSamples = (int)((long)inputFrames * outputRate / sampleRate);
        short[] mono = new short[outputSamples];
        for (int output = 0; output < outputSamples; ++output) {
            int begin = (int)((long)output * sampleRate / outputRate);
            int end = (int)((long)(output + 1) * sampleRate / outputRate);
            if (end <= begin) end = begin + 1;
            long sum = 0;
            for (int frame = begin; frame < end && frame < inputFrames; ++frame) {
                int sample = dataOffset + frame * 4;
                sum += BitConverter.ToInt16(wave, sample);
                sum += BitConverter.ToInt16(wave, sample + 2);
            }
            mono[output] = (short)(sum / ((end - begin) * 2));
        }

        byte[] codes = new byte[(outputSamples + 3) / 4];
        int predictor = 0, stepIndex = 40;
        for (int index = 0; index < outputSamples; ++index)
            codes[index / 4] |= (byte)(EncodeSample(mono[index], ref predictor, ref stepIndex) << ((index & 3) * 2));

        using (var writer = new BinaryWriter(File.Create(encodedPath))) {
            writer.Write(Encoding.ASCII.GetBytes("HSA2")); writer.Write((ushort)1); writer.Write((ushort)outputRate);
            writer.Write(outputSamples); writer.Write((short)0); writer.Write((byte)40); writer.Write((byte)0);
            writer.Write(codes);
        }

        short[] decoded = new short[outputSamples]; predictor = 0; stepIndex = 40;
        for (int index = 0; index < outputSamples; ++index) {
            byte code = (byte)((codes[index / 4] >> ((index & 3) * 2)) & 3);
            decoded[index] = DecodeSample(code, ref predictor, ref stepIndex);
        }
        using (var writer = new BinaryWriter(File.Create(previewPath))) {
            int bytes = decoded.Length * 2;
            writer.Write(Encoding.ASCII.GetBytes("RIFF")); writer.Write(36 + bytes);
            writer.Write(Encoding.ASCII.GetBytes("WAVEfmt ")); writer.Write(16);
            writer.Write((short)1); writer.Write((short)1); writer.Write(outputRate);
            writer.Write(outputRate * 2); writer.Write((short)2); writer.Write((short)16);
            writer.Write(Encoding.ASCII.GetBytes("data")); writer.Write(bytes);
            foreach (short sample in decoded) writer.Write(sample);
        }
    }
}
'@

$inputFile = (Resolve-Path -LiteralPath $InputPath).Path
$encodedFile = [IO.Path]::GetFullPath($EncodedPath)
$previewFile = [IO.Path]::GetFullPath($PreviewPath)
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($encodedFile)) | Out-Null
[IO.Directory]::CreateDirectory([IO.Path]::GetDirectoryName($previewFile)) | Out-Null
[Adpcm2Preview]::Convert($inputFile, $encodedFile, $previewFile)
Get-Item -LiteralPath $encodedFile, $previewFile | Select-Object FullName, Length
