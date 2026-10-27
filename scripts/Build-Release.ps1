param(
    [string]$Version,
    [string]$Payload,
    [string]$InnoCompiler
)
$ErrorActionPreference = 'Stop'
$ExcellentZipRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
Set-Location -LiteralPath $ExcellentZipRoot
if (-not $Version) {
    [xml]$versionProperties = Get-Content -LiteralPath 'Project/ExcellentZip.Project.Version.props'
    $Version = $versionProperties.Project.PropertyGroup.ExcellentZipDisplayVersion
}
if ($Version -notmatch '^\d{4}-\d{2}-\d{2}\.[1-9]\d*$') { throw 'Use an app version like 2026-10-27.1.' }
$parts = $Version.Split('.')
$date = [datetime]::ParseExact($parts[0], 'yyyy-MM-dd', [Globalization.CultureInfo]::InvariantCulture)
$revision = [int]$parts[1]
if ($revision -gt 65535) { throw 'Windows version revision must not exceed 65535.' }
$numericVersion = '{0}.{1}.{2}.{3}' -f $date.Year, $date.Month, $date.Day, $revision
& python scripts\generate_icon_download.py --check
if ($LASTEXITCODE) { throw 'Optional icon download metadata is stale; regenerate it before building.' }
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
$vsPath = & $vswhere -latest -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vsPath) { throw 'Visual Studio C++ build tools are required.' }
$msbuild = Join-Path $vsPath 'MSBuild\Current\Bin\MSBuild.exe'

if (-not $Payload) {
    & $msbuild Package\ExcellentZipPackage.wapproj /restore /t:Build /m:2 /p:CL_MPCount=2 /nologo /v:minimal `
        /p:Configuration=Release /p:Platform=x64 /p:AppxBundle=Never /p:AppxPackageSigningEnabled=false `
        "/p:MileProjectVersion=$numericVersion" "/p:ExcellentZipDisplayVersion=$Version"
    if ($LASTEXITCODE) { throw 'Windows app build failed.' }
    $package = Get-ChildItem -LiteralPath (Join-Path $ExcellentZipRoot 'Output\Binaries\AppPackages') `
        -Filter "ExcellentZipPackage_${numericVersion}_x64.msix" -File -Recurse |
        Sort-Object LastWriteTimeUtc -Descending | Select-Object -First 1
    if (-not $package) { throw 'The app build did not produce the expected x64 MSIX payload.' }
    $Payload = Join-Path $ExcellentZipRoot ('Output\ReleaseLayout\' + [guid]::NewGuid().ToString('N'))
    [IO.Compression.ZipFile]::ExtractToDirectory($package.FullName, $Payload)
}
& python scripts\test_release_smoke.py --payload $Payload --version $Version
if ($LASTEXITCODE) { throw 'Archive engine smoke checks failed.' }
if (-not $InnoCompiler) {
    $InnoCompiler = Join-Path $ExcellentZipRoot '.tools\InnoSetup6.4.2\tools\ISCC.exe'
    if (-not (Test-Path -LiteralPath $InnoCompiler)) {
        & $msbuild packaging\InnoSetup.proj /t:Restore /nologo /v:minimal
        if ($LASTEXITCODE) { throw 'Inno Setup package restore failed.' }
        $InnoCompiler = Join-Path $env:USERPROFILE '.nuget\packages\tools.innosetup\6.4.2\tools\ISCC.exe'
    }
}
if (-not (Test-Path -LiteralPath $InnoCompiler)) { throw 'Inno Setup 6.4.2 compiler is required.' }
& python scripts\package_release.py --payload $Payload --compiler $InnoCompiler --version $Version
if ($LASTEXITCODE) { throw 'Release packaging failed.' }
Write-Host "ExcellentZip $Version installer and portable ZIP are in dist."
