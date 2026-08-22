param(
    [string]$OutputDirectory = (Join-Path $PSScriptRoot "..\assets-src\audio-preview")
)

$ErrorActionPreference = "Stop"
$sampleRate = 22050
$seed = [uint32]0x13579BDF
$softNoise = 0.0
$fastNoise = 0.0
$slowNoise = 0.0

function Get-Noise {
    $script:seed = [uint32]((([uint64]$script:seed * 1664525) + 1013904223) % 4294967296)
    return ([double](($script:seed -shr 8) -band 0xFFFF) / 32767.5) - 1.0
}

function Get-SoftNoise([double]$Amount = 0.12) {
    $script:softNoise += $Amount * ((Get-Noise) - $script:softNoise)
    return $script:softNoise
}

function Get-WaterNoise {
    $noise = Get-Noise
    $script:fastNoise += 0.085 * ($noise - $script:fastNoise)
    $script:slowNoise += 0.009 * ($noise - $script:slowNoise)
    return $script:fastNoise - $script:slowNoise
}

function Get-Triangle([double]$Phase) {
    $fraction = $Phase - [Math]::Floor($Phase)
    return 1.0 - 4.0 * [Math]::Abs($fraction - 0.5)
}

function Write-Wave([string]$Name, [double]$Duration, [scriptblock]$Sample) {
    $script:softNoise = 0.0
    $script:fastNoise = 0.0
    $script:slowNoise = 0.0
    $count = [int]($sampleRate * $Duration)
    $path = Join-Path $OutputDirectory $Name
    $stream = [IO.File]::Open($path, [IO.FileMode]::Create, [IO.FileAccess]::Write)
    $writer = [IO.BinaryWriter]::new($stream)
    $dataBytes = $count * 2
    $writer.Write([Text.Encoding]::ASCII.GetBytes("RIFF"))
    $writer.Write([int](36 + $dataBytes))
    $writer.Write([Text.Encoding]::ASCII.GetBytes("WAVEfmt "))
    $writer.Write([int]16); $writer.Write([int16]1); $writer.Write([int16]1)
    $writer.Write([int]$sampleRate); $writer.Write([int]($sampleRate * 2))
    $writer.Write([int16]2); $writer.Write([int16]16)
    $writer.Write([Text.Encoding]::ASCII.GetBytes("data")); $writer.Write([int]$dataBytes)
    for ($index = 0; $index -lt $count; ++$index) {
        $time = [double]$index / $sampleRate
        $value = & $Sample $time $Duration
        $value = [Math]::Max(-1.0, [Math]::Min(1.0, $value))
        $writer.Write([int16]($value * 28000.0))
    }
    $writer.Dispose()
}

New-Item -ItemType Directory -Force -Path $OutputDirectory | Out-Null

Write-Wave "hoe.wav" 0.22 {
    param($t, $d)
    $attack = [Math]::Min(1.0, $t * 100.0)
    $envelope = $attack * [Math]::Exp(-14.0 * $t)
    $thud = [Math]::Sin(2.0 * [Math]::PI * (82.0 - 45.0 * $t) * $t)
    return $envelope * (0.62 * $thud + 0.18 * (Get-SoftNoise 0.08))
}

Write-Wave "watering.wav" 0.70 {
    param($t, $d)
    $edge = [Math]::Min(1.0, $t * 7.0) * [Math]::Min(1.0, ($d - $t) * 7.0)
    $flow = Get-WaterNoise
    $body = Get-SoftNoise 0.018
    $movement = 0.82 +
        0.10 * [Math]::Sin(2.0 * [Math]::PI * 1.7 * $t) +
        0.06 * [Math]::Sin(2.0 * [Math]::PI * 2.9 * $t + 1.1)
    return $edge * $movement * (0.105 * $flow + 0.038 * $body)
}

Write-Wave "plant.wav" 0.24 {
    param($t, $d)
    $drop = [Math]::Sin(2.0 * [Math]::PI * (230.0 - 260.0 * $t) * $t) * [Math]::Exp(-13.0 * $t)
    $soil = (Get-SoftNoise 0.07) * [Math]::Exp(-24.0 * [Math]::Max(0.0, $t - 0.07))
    return 0.44 * $drop + $(if ($t -gt 0.07) { 0.16 * $soil } else { 0.0 })
}

Write-Wave "harvest.wav" 0.32 {
    param($t, $d)
    $edge = [Math]::Min(1.0, $t * 45.0) * [Math]::Min(1.0, ($d - $t) * 15.0)
    $rustle = 0.14 * (Get-WaterNoise) * [Math]::Exp(-5.0 * $t)
    $pull = 0.17 * [Math]::Sin(2.0 * [Math]::PI * (105.0 - 75.0 * $t) * $t) *
        [Math]::Exp(-18.0 * $t)
    $local = $t - 0.075
    $release = if ($local -ge 0.0) {
        (0.075 * (Get-SoftNoise 0.24) +
         0.10 * [Math]::Sin(2.0 * [Math]::PI * 82.0 * $local)) *
            [Math]::Exp(-38.0 * $local)
    } else { 0.0 }
    return $edge * ($rustle + $pull + $release)
}

Write-Wave "ui_move.wav" 0.08 {
    param($t, $d)
    $wood = Get-Triangle (185.0 * $t)
    $click = Get-SoftNoise 0.32
    return (0.18 * $wood + 0.06 * $click) * [Math]::Exp(-42.0 * $t)
}

Write-Wave "ui_confirm.wav" 0.18 {
    param($t, $d)
    $first = (Get-Triangle (220.0 * $t)) * [Math]::Exp(-38.0 * $t)
    $local = $t - 0.065
    $second = if ($local -ge 0.0) {
        (Get-Triangle (293.66 * $local)) * [Math]::Exp(-34.0 * $local)
    } else { 0.0 }
    return 0.20 * $first + 0.17 * $second
}

Get-ChildItem -LiteralPath $OutputDirectory -Filter *.wav | Select-Object FullName, Length
