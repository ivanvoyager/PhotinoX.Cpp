param(
    [ValidateSet("debug", "release")]
    [string] $Preset = "debug"
)

$ErrorActionPreference = "Stop"

$RootDirectory = Split-Path -Parent $PSScriptRoot
$VsWhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"

if (-not (Test-Path $VsWhere)) {
    throw "Visual Studio Installer was not found."
}

$VsInstallationPath = & $VsWhere `
    -latest `
    -products * `
    -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
    -property installationPath

if (-not $VsInstallationPath) {
    throw "Visual Studio with C++ tools was not found."
}

Import-Module `
    (Join-Path $VsInstallationPath "Common7\Tools\Microsoft.VisualStudio.DevShell.dll")

Enter-VsDevShell `
    -VsInstallPath $VsInstallationPath `
    -SkipAutomaticLocation `
    -DevCmdArguments "-arch=x64 -host_arch=x64"

$ExecutablePath = Join-Path `
    $RootDirectory `
    "build/$Preset/samples/HelloWorld/PhotinoX.Cpp.HelloWorld.exe"

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

    & $ExecutablePath

    if ($LASTEXITCODE -ne 0) {
        throw "HelloWorld failed with exit code $LASTEXITCODE."
    }
}
finally {
    Pop-Location
}