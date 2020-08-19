[cmdletbinding()]
param (
    [string]$mode,
    [string]$qtver,
    [string]$daemonDir,
    [string]$lrcDir,
    [string]$outDir
);

write-host "Copying deployment files..." -ForegroundColor Green

# default values
$qtver = If ($qtver) { $qtver } Else { "5.15.0" }
$mode = If ($mode) { $mode } Else { "Release" }

$qtverSplit1, $qtverSplit2 , $qtverSplit3 = $qtver.Split('.')
$qtMsvcDir = "msvc2019_64"

$QtDir = "C:\Qt\$qtver\$qtMsvcDir"

$clientDir = split-path -parent $MyInvocation.MyCommand.Definition

if (!$outDir) { $outDir = $clientDir + "\x64\" + $mode }
If (!(test-path $outDir)) { New-Item -ItemType directory -Path $outDir -Force }

if (!$daemonDir) { $daemonDir = $clientDir + '\..\daemon' }
if (!$lrcDir) { $lrcDir = $clientDir + '\..\lrc' }

write-host "********************************************************************************" -ForegroundColor Magenta
write-host "using daemonDir:    " $daemonDir -ForegroundColor Magenta
write-host "using lrcDir:       " $lrcDir -ForegroundColor Magenta
write-host "using QtDir:        " $QtDir -ForegroundColor Magenta
write-host "********************************************************************************" -ForegroundColor Magenta

# dependency bin files and misc
$FilesToCopy = @(
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\avcodec-58.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\avutil-56.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\avformat-58.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\avdevice-58.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\swresample-3.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\swscale-5.dll",
    "$daemonDir\contrib\build\ffmpeg\Build\win32\x64\bin\avfilter-7.dll",
    "$daemonDir\contrib\build\openssl\out32dll\libeay32.dll",
    "$daemonDir\contrib\build\openssl\out32dll\ssleay32.dll",
    "$clientDir\qt.conf",
    "$clientDir\images\jami.ico",
    "$clientDir\License.rtf"
)
foreach ($i in $FilesToCopy) {
    write-host "copying: " $i " => " $outDir -ForegroundColor Cyan
    Copy-Item -Path $i -Recurse -Destination $outDir -Force -Container
}

############
# qt
############
$windeployqt = "$QtDir\bin\windeployqt.exe --qmldir $clientDir\src --release $outDir\Jami.exe"
Invoke-Expression $windeployqt

# ringtones
$CopyDir = $outDir + "\ringtones"
If (!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
$RingtonePath = "$clientDir\..\daemon\ringtones"
write-host "copying ringtones..."
Get-ChildItem -Path $RingtonePath -Include *.ul, *.ogg, *.wav, *.opus -Recurse | ForEach-Object {
    write-host "copying ringtone: " $_.FullName " => " $CopyDir -ForegroundColor Cyan
    Copy-Item -Path $_.FullName -Destination $CopyDir -Force –Recurse
}

# qt translations
$lrelease = "$QtDir\bin\lrelease.exe"

# lrc translations
$lrcTSPath = "$lrcDir\translations"
Get-ChildItem -Path $lrcTSPath -Include *.ts -Recurse | ForEach-Object {
    & $lrelease $_.FullName
}
$CopyDir = $outDir + "\share\libringclient\translations"
If (!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
write-host "copying lrc translations..."
Get-ChildItem -Path $lrcTSPath -Include *.qm -Recurse | ForEach-Object {
    write-host "copying translation file: " $_.FullName " => " $CopyDir -ForegroundColor Cyan
    Copy-Item -Path $_.FullName -Destination $CopyDir -Force –Recurse
}

# client translations
$clientTSPath = "$clientDir\translations"
Get-ChildItem -Path $clientTSPath -Include *.ts -Recurse | ForEach-Object {
    & $lrelease $_.FullName
}
$CopyDir = $outDir + "\share\ring\translations"
If (!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
write-host "copying client translations..."
Get-ChildItem -Path $clientTSPath -Include *.qm -Recurse | ForEach-Object {
    write-host "copying translation file: " $_.FullName " => " $CopyDir -ForegroundColor Cyan
    Copy-Item -Path $_.FullName -Destination $CopyDir -Force –Recurse
}

write-host "copy completed" -NoNewline -ForegroundColor Green