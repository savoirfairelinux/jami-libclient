[cmdletbinding()]
param (
    [string]$qtver
);

$clientDir = split-path -parent $MyInvocation.MyCommand.Definition
$qtver = If ($qtver) { $qtver } Else { "5.15.0" }
$qtMinorVer = $qtver.split('.')[1]
$qtMsvcDir = If (([int]$qtMinorVer) -le 14) {"msvc2017_64"} Else {"msvc2019_64"}
$QtDir = "C:\Qt\$qtver\$qtMsvcDir"

$tsFileNames = Get-ChildItem -Path "$clientDir\translations" -Recurse -Include *.ts
$lupdate = "$QtDir\bin\lupdate.exe"

Invoke-Expression("$lupdate $clientDir\src -ts $tsFileNames -no-obsolete")
