param(
	[String]$targetpath,
	[String]$committag,
	[String]$buildname,
	[String]$commitrefslug
	[String]$commitsha
)

$datestr=get-date -format "yyyyMMdd-HHmm"

if ($committag -and $committag[0] -eq "v")
{
  $committype='release\' + $committag
}
else if ($buildname -eq "nightly")
{
  $committype='nightly-builds\' + $datestr + '_' + $commitrefslug + '_' + $commitsha
}
else
{
  $committype='commits\' + $datestr + '_' + $commitrefslug + '_' + $commitsha
}

Write-Host "deploying bin to $targetpath\$committype.zip"
7z.exe a $targetpath\$committype.zip bin

#Compress-Archive -Path $InstallerPath\Installer\* -Update -DestinationPath $InstallerPath\Installer\Release_$version.zip
#Send the archive in the SharePoint
#Copy-Item "$InstallerPath\Installer\Release_$version.zip" "C:\Users\mouvent\MOUVENT AG\Workflow solutions - GitLab\Release $version.zip"