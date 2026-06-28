$ErrorActionPreference = "Stop"
Set-StrictMode -Version Latest

$RootDir = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path
$Version = "1.0.0"
$Product = "AudioReactiveImagePOC"
$BuildDir = Join-Path $RootDir "build\$($Product)_artefacts\Release"
$DistDir = Join-Path $RootDir "dist"
$PackageDir = Join-Path $DistDir "$Product-v$Version-windows"
$ZipPath = Join-Path $DistDir "$Product-v$Version-windows.zip"

function Require-Path {
    param([Parameter(Mandatory = $true)][string]$Path)

    if (-not (Test-Path $Path)) {
        Write-Error "Missing required release artifact: $Path`nRun: cmake -S . -B build -A x64 && cmake --build build --config Release"
    }
}

$Vst3Path = Join-Path $BuildDir "VST3\$Product.vst3"
$StandalonePath = Join-Path $BuildDir "Standalone\$Product.exe"
$ReadmePath = Join-Path $RootDir "README.md"
$ReleaseNotesPath = Join-Path $RootDir "RELEASE_NOTES.md"

Require-Path $Vst3Path
Require-Path $ReadmePath
Require-Path $ReleaseNotesPath

if (Test-Path $PackageDir) {
    Remove-Item $PackageDir -Recurse -Force
}

if (Test-Path $ZipPath) {
    Remove-Item $ZipPath -Force
}

New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "Plugins\VST3") | Out-Null
New-Item -ItemType Directory -Force -Path (Join-Path $PackageDir "Standalone") | Out-Null

Copy-Item $Vst3Path (Join-Path $PackageDir "Plugins\VST3\") -Recurse -Force

if (Test-Path $StandalonePath) {
    Copy-Item $StandalonePath (Join-Path $PackageDir "Standalone\") -Force
}

Copy-Item $ReadmePath (Join-Path $PackageDir "README.md") -Force
Copy-Item $ReleaseNotesPath (Join-Path $PackageDir "RELEASE_NOTES.md") -Force

@"
$Product v$Version

VST3 install:
  Copy Plugins\VST3\$Product.vst3 to:
  C:\Program Files\Common Files\VST3\

Standalone:
  If present, run Standalone\$Product.exe.

After installing, restart or rescan plugins in your DAW.
"@ | Set-Content -Path (Join-Path $PackageDir "INSTALL_WINDOWS.txt") -Encoding UTF8

Compress-Archive -Path $PackageDir -DestinationPath $ZipPath -CompressionLevel Optimal -Force

Write-Host "Release package created:"
Write-Host $ZipPath

