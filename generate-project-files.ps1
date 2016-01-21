# Script to bootstrap the generation of project files for OSVR Unreal.

# Copyright 2016 Sensics, Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

$UETargetVersion = "4.10"

# Gets the locations of Unreal Engine versions installed by the Epic Games Launcher,
# which stores its data in ProgramData in a json file with a funny extension. Optional
# parameter lets you pass a version number as a string to filter, or * for all versions.
function Get-LauncherInstalledUEVersions($Ver = "*")
{
    $Ret = [System.Collections.ArrayList]@()
    $ProgramData = [Environment]::GetFolderPath([Environment+SpecialFolder]::CommonApplicationData)
    $LauncherInstalledFile = Join-Path $ProgramData "Epic\UnrealEngineLauncher\LauncherInstalled.dat"
    $JSONData = Get-Content -Raw $LauncherInstalledFile | ConvertFrom-JSON
    $JSONData.InstallationList | Where-Object {$_.AppName -like "UE_$Ver"} | ForEach-Object {[String]$_.InstallLocation}
}

function Get-SourceBuiltUEVersions() {
    $RegKey = Get-Item "hkcu:\Software\Epic Games\Unreal Engine\Builds"
    $RegKey.GetValueNames() | ForEach-Object {
        $RegKey.GetValue($_)
        # TODO how to filter by version? not sure how to check version in these builds...
    }
}

#Get-SourceBuiltUEVersions
#Get-LauncherInstalledUEVersions
#Get-LauncherInstalledUEVersions "4.9"

# Including the source built ones as backup since we can't filter by version right now and this is just for bootstrapping.

function Get-VersionSelectorFromLauncherEngine($Engine) {
    return @(
        (Join-Path $Engine "..\Launcher\Engine\Binaries\Win64\UnrealVersionSelector.exe"),
        (Join-Path $Engine "..\Launcher\Engine\Binaries\Win32\UnrealVersionSelector.exe"))
}
function Get-VersionSelectorFromBuiltEngine($Engine) {
    $PotentialPaths = @()
    @(64, 32) | ForEach-Object {
        $Bits = $_
        $UVS = Join-Path $Engine "Engine\Binaries\Win$Bits\UnrealVersionSelector.exe"
        $PotentialPaths += $UVS
        @('Shipping', 'Debug') | ForEach-Object {
            $Variant = $_
            $UVS = Join-Path $Engine "Engine\Binaries\Win$Bits\UnrealVersionSelector-Win$Bits-$Variant.exe"
            $PotentialPaths += $UVS
        }
    }
    return $PotentialPaths
}

function GetVersionSelectorsFromEngines($Engine) {
    @(
        Get-VersionSelectorFromLauncherEngine $Engine
        Get-VersionSelectorFromBuiltEngine $Engine
    ) | Where-Object { (Test-Path $_) }
}

function Generate-ProjectFiles($UProject) {

    $BootstrapUEVersions = @(Get-LauncherInstalledUEVersions $UETargetVersion) + @(Get-SourceBuiltUEVersions)
    Write-Host "Found these installations to potentially bootstrap the project file, will search in order:"
    $BootstrapUEVersions | ForEach-Object {
        Write-Host " - $_"
    }

    #$VersionSelectorExecutables = $BootstrapUEVersions | ForEach-Object { GetVersionSelectorsFromEngines($_) }
    $UBTs = $BootstrapUEVersions | ForEach-Object { Join-Path $_ "Engine\Binaries\DotNET\UnrealBuildTool.exe" } | Where-Object { (Test-Path $_) }
    $UBT = $UBTs[0]
    $UBTdir = (Get-Item $UBT).Directory.FullName
    # The corresponding "/Engine/Source" directory for the UBT we're using.
    $EngineSource = (Get-Item (Join-Path $UBTdir "..\..\Source")).FullName
    $FullProjectPath  = (Get-Item $UProject).FullName

    
    Write-Host "Using build tool $UBT to generate project files..."
    $Args = @(
        '-projectfiles',
        "-project=""$FullProjectPath""",
        '-game',
        '-rocket')
    Write-Host "Arguments: $Args"
    Start-Process -FilePath $UBT -ArgumentList $Args -NoNewWindow -Wait -WorkingDirectory $EngineSource

}
#Running C:/Program Files (x86)/Epic Games/4.10/Engine/Binaries/DotNET/UnrealBuildTool.exe  -projectfiles -project="C:/Users/Ryan/Desktop/src/OSVR-Unreal/OSVRUnreal/OSVRUnreal.uproject" -game -rocket -progress
Generate-ProjectFiles "OSVRUnreal\OSVRUnreal.uproject"