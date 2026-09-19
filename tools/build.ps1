param(
    [ValidateSet("debug", "release", "debug-local", "release-local")]
    [string] $Preset
)

$ErrorActionPreference = "Stop"

$RootDirectory = Split-Path -Parent $PSScriptRoot
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not $Preset) {
    $Preset = "debug"

    $UserPresetsPath = Join-Path $RootDirectory "CMakeUserPresets.json"

    if (Test-Path $UserPresetsPath) {
        $UserPresets = Get-Content $UserPresetsPath -Raw | ConvertFrom-Json
        $LocalPreset = $UserPresets.configurePresets |
            Where-Object { $_.name -eq "debug-local" } |
            Select-Object -First 1

        if ($LocalPreset) {
            $LocalPackage = $LocalPreset.cacheVariables.PHOTINOX_NATIVE_PACKAGE

            if ($LocalPackage) {
                if ([System.IO.Path]::IsPathRooted($LocalPackage)) {
                    $LocalPackagePath = $LocalPackage
                }
                else {
                    $LocalPackagePath = Join-Path $RootDirectory $LocalPackage
                }

                if (Test-Path $LocalPackagePath) {
                    $Preset = "debug-local"
                    Write-Host "Using local PhotinoX.Native package: $LocalPackagePath"
                }
            }
        }
    }
}

$VsInstallationPath = & $VsWhere `
    -latest `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not (Test-Path $VsWhere)) {
    throw "Visual Studio Installer was not found."
}

if (-not $VsInstallationPath) {
    throw "Visual Studio with C++ tools was not found."
}

Import-Module `
    (Join-Path $VsInstallationPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")

Enter-VsDevShell `
    -VsInstallPath $VsInstallationPath `
    -SkipAutomaticLocation `
    -DevCmdArguments "-arch=x64 -host_arch=x64"

Push-Location $RootDirectory

try {
    cmake --preset $Preset

    if ($LASTEXITCODE -ne 0) {
        throw "CMake configuration failed with exit code $LASTEXITCODE."
    }

    cmake --build --preset $Preset

    if ($LASTEXITCODE -ne 0) {
        throw "CMake build failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}