Param(
    [Parameter(Mandatory=$false)]
    [Switch]$clean
)

$ErrorActionPreference = 'Stop'

# if user specified clean, remove all build files
if ($clean.IsPresent)
{
    if (Test-Path -Path "build")
    {
        remove-item build -R
    }
}

$NDKPath = Get-Content $PSScriptRoot/ndkpath.txt

if (($clean.IsPresent) -or (-not (Test-Path -Path "build")))
{
    $out = new-item -Path build -ItemType Directory
}

Push-Location "$PSScriptRoot/build"
try {
    & cmake -G "Ninja" -DCMAKE_BUILD_TYPE="RelWithDebInfo" ../
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed ($LASTEXITCODE)." }
    & cmake --build .
    if ($LASTEXITCODE -ne 0) { throw "CMake build failed ($LASTEXITCODE)." }
}
finally {
    Pop-Location
}
