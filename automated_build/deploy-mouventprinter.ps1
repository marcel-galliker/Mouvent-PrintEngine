param(
	[String]$targetpath,
	[String]$committag,
	[String]$buildname,
	[String]$commitrefslug,
	[String]$commitsha
)

Write-Host "targetpath = $targetpath"
Write-Host "committag = $committag"
Write-Host "buildname = $buildname"
Write-Host "commitrefslug = $commitrefslug"
Write-Host "commitsh = $commitsha"

$datestr=get-date -format "yyyyMMdd-HHmm"

if ($committag -and $committag[0] -eq "v")
{
  $committype='release\' + $committag
}
elseif ($buildname -eq "nightly")
{
  $committype='nightly-builds\' + $datestr + '_' + $commitrefslug + '_' + $commitsha
}
else
{
  $committype='commits\' + $datestr + '_' + $commitrefslug + '_' + $commitsha
}

Write-Host "deploying bin to $targetpath\$committype.zip"
7z.exe a $targetpath\$committype.zip bin
