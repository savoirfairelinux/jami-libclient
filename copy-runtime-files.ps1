[cmdletbinding()]
param (
    [string]$mode,
    [string]$qtver,
    [string]$daemonDir,
    [string]$lrcDir
);

write-host "Copying runtime files..." -ForegroundColor Green

# default values
$qtver = If ($qtver) {$qtver} Else {"5.15.0"}
$mode  = If ($mode)  {$mode} Else {"Release"}

$qtverSplit1, $qtverSplit2 ,$qtverSplit3 = $qtver.Split('.')
$qtMsvcDir = If((([int]$qtverSplit1) -ge 6) -OR ( (([int]$qtverSplit1) -eq 5) -AND (([int]$qtverSplit2) -ge 15))) {
    "msvc2019_64"
} Else {
    "msvc2017_64"
}

$QtDir = "C:\Qt\$qtver\$qtMsvcDir"

$ClientDir = split-path -parent $MyInvocation.MyCommand.Definition

$OutDir = $ClientDir + "\x64\" + $mode
If(!(test-path $OutDir)) { New-Item -ItemType directory -Path $OutDir -Force }

if (!$daemonDir) { $daemonDir = $ClientDir + '\..\daemon' }
if (!$lrcDir) { $lrcDir = $ClientDir + '\..\lrc' }

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
    "$ClientDir\qt.conf",
    "$ClientDir\images\jami.ico",
    "$ClientDir\License.rtf"
    )
foreach ($i in $FilesToCopy) {
    write-host "copying: " $i " => " $OutDir -ForegroundColor Cyan
    Copy-Item -Path $i -Recurse -Destination $OutDir -Force -Container
}

############
# qt
############
$windeployqt = "$QtDir\bin\windeployqt.exe --qmldir $ClientDir\src --release $OutDir\Jami.exe"
iex $windeployqt

# ringtones
$CopyDir = $OutDir + "\ringtones"
If(!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
$RingtonePath = "$ClientDir\..\daemon\ringtones"
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
$CopyDir = $OutDir + "\share\libringclient\translations"
If(!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
write-host "copying lrc translations..."
Get-ChildItem -Path $lrcTSPath -Include *.qm -Recurse | ForEach-Object {
    write-host "copying translation file: " $_.FullName " => " $CopyDir -ForegroundColor Cyan
    Copy-Item -Path $_.FullName -Destination $CopyDir -Force –Recurse
}

# client translations
$clientTSPath = "$ClientDir\translations"
Get-ChildItem -Path $clientTSPath -Include *.ts -Recurse | ForEach-Object {
    & $lrelease $_.FullName
}
$CopyDir = $OutDir + "\share\ring\translations"
If(!(test-path $CopyDir)) { New-Item -ItemType directory -Path $CopyDir -Force }
write-host "copying client translations..."
Get-ChildItem -Path $clientTSPath -Include *.qm -Recurse | ForEach-Object {
    write-host "copying translation file: " $_.FullName " => " $CopyDir -ForegroundColor Cyan
    Copy-Item -Path $_.FullName -Destination $CopyDir -Force –Recurse
}

write-host "copy completed" -NoNewline -ForegroundColor Green