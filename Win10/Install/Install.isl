<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<?xml-stylesheet type="text/xsl" href="is.xsl" ?>
<!DOCTYPE msi [
   <!ELEMENT msi   (summary,table*)>
   <!ATTLIST msi version    CDATA #REQUIRED>
   <!ATTLIST msi xmlns:dt   CDATA #IMPLIED
                 codepage   CDATA #IMPLIED
                 compression (MSZIP|LZX|none) "LZX">
   
   <!ELEMENT summary       (codepage?,title?,subject?,author?,keywords?,comments?,
                            template,lastauthor?,revnumber,lastprinted?,
                            createdtm?,lastsavedtm?,pagecount,wordcount,
                            charcount?,appname?,security?)>
                            
   <!ELEMENT codepage      (#PCDATA)>
   <!ELEMENT title         (#PCDATA)>
   <!ELEMENT subject       (#PCDATA)>
   <!ELEMENT author        (#PCDATA)>
   <!ELEMENT keywords      (#PCDATA)>
   <!ELEMENT comments      (#PCDATA)>
   <!ELEMENT template      (#PCDATA)>
   <!ELEMENT lastauthor    (#PCDATA)>
   <!ELEMENT revnumber     (#PCDATA)>
   <!ELEMENT lastprinted   (#PCDATA)>
   <!ELEMENT createdtm     (#PCDATA)>
   <!ELEMENT lastsavedtm   (#PCDATA)>
   <!ELEMENT pagecount     (#PCDATA)>
   <!ELEMENT wordcount     (#PCDATA)>
   <!ELEMENT charcount     (#PCDATA)>
   <!ELEMENT appname       (#PCDATA)>
   <!ELEMENT security      (#PCDATA)>                            
                                
   <!ELEMENT table         (col+,row*)>
   <!ATTLIST table
                name        CDATA #REQUIRED>

   <!ELEMENT col           (#PCDATA)>
   <!ATTLIST col
                 key       (yes|no) #IMPLIED
                 def       CDATA #IMPLIED>
                 
   <!ELEMENT row            (td+)>
   
   <!ELEMENT td             (#PCDATA)>
   <!ATTLIST td
                 href       CDATA #IMPLIED
                 dt:dt     (string|bin.base64) #IMPLIED
                 md5        CDATA #IMPLIED>
]>
<msi version="2.0" xmlns:dt="urn:schemas-microsoft-com:datatypes" codepage="65001">
	
	<summary>
		<codepage>1252</codepage>
		<title>Installation Database</title>
		<subject>Surface Installer</subject>
		<author>##ID_STRING4##</author>
		<keywords>Installer,MSI,Database</keywords>
		<comments>Contact:  Your local administrator</comments>
		<template>Intel;1033</template>
		<lastauthor>Administrator</lastauthor>
		<revnumber>{D457FCD2-AA59-4741-8D17-0B0B5B3702ED}</revnumber>
		<lastprinted/>
		<createdtm>06/21/1999 15:00</createdtm>
		<lastsavedtm>07/14/2000 18:50</lastsavedtm>
		<pagecount>200</pagecount>
		<wordcount>0</wordcount>
		<charcount/>
		<appname>InstallShield Express</appname>
		<security>1</security>
	</summary>
	
	<table name="ActionText">
		<col key="yes" def="s72">Action</col>
		<col def="L64">Description</col>
		<col def="L128">Template</col>
		<row><td>Advertise</td><td>##IDS_ACTIONTEXT_Advertising##</td><td/></row>
		<row><td>AllocateRegistrySpace</td><td>##IDS_ACTIONTEXT_AllocatingRegistry##</td><td>##IDS_ACTIONTEXT_FreeSpace##</td></row>
		<row><td>AppSearch</td><td>##IDS_ACTIONTEXT_SearchInstalled##</td><td>##IDS_ACTIONTEXT_PropertySignature##</td></row>
		<row><td>BindImage</td><td>##IDS_ACTIONTEXT_BindingExes##</td><td>##IDS_ACTIONTEXT_File##</td></row>
		<row><td>CCPSearch</td><td>##IDS_ACTIONTEXT_UnregisterModules##</td><td/></row>
		<row><td>CostFinalize</td><td>##IDS_ACTIONTEXT_ComputingSpace3##</td><td/></row>
		<row><td>CostInitialize</td><td>##IDS_ACTIONTEXT_ComputingSpace##</td><td/></row>
		<row><td>CreateFolders</td><td>##IDS_ACTIONTEXT_CreatingFolders##</td><td>##IDS_ACTIONTEXT_Folder##</td></row>
		<row><td>CreateShortcuts</td><td>##IDS_ACTIONTEXT_CreatingShortcuts##</td><td>##IDS_ACTIONTEXT_Shortcut##</td></row>
		<row><td>DeleteServices</td><td>##IDS_ACTIONTEXT_DeletingServices##</td><td>##IDS_ACTIONTEXT_Service##</td></row>
		<row><td>DuplicateFiles</td><td>##IDS_ACTIONTEXT_CreatingDuplicate##</td><td>##IDS_ACTIONTEXT_FileDirectorySize##</td></row>
		<row><td>FileCost</td><td>##IDS_ACTIONTEXT_ComputingSpace2##</td><td/></row>
		<row><td>FindRelatedProducts</td><td>##IDS_ACTIONTEXT_SearchForRelated##</td><td>##IDS_ACTIONTEXT_FoundApp##</td></row>
		<row><td>GenerateScript</td><td>##IDS_ACTIONTEXT_GeneratingScript##</td><td>##IDS_ACTIONTEXT_1##</td></row>
		<row><td>ISLockPermissionsCost</td><td>##IDS_ACTIONTEXT_ISLockPermissionsCost##</td><td/></row>
		<row><td>ISLockPermissionsInstall</td><td>##IDS_ACTIONTEXT_ISLockPermissionsInstall##</td><td/></row>
		<row><td>InstallAdminPackage</td><td>##IDS_ACTIONTEXT_CopyingNetworkFiles##</td><td>##IDS_ACTIONTEXT_FileDirSize##</td></row>
		<row><td>InstallFiles</td><td>##IDS_ACTIONTEXT_CopyingNewFiles##</td><td>##IDS_ACTIONTEXT_FileDirSize2##</td></row>
		<row><td>InstallODBC</td><td>##IDS_ACTIONTEXT_InstallODBC##</td><td/></row>
		<row><td>InstallSFPCatalogFile</td><td>##IDS_ACTIONTEXT_InstallingSystemCatalog##</td><td>##IDS_ACTIONTEXT_FileDependencies##</td></row>
		<row><td>InstallServices</td><td>##IDS_ACTIONTEXT_InstallServices##</td><td>##IDS_ACTIONTEXT_Service2##</td></row>
		<row><td>InstallValidate</td><td>##IDS_ACTIONTEXT_Validating##</td><td/></row>
		<row><td>LaunchConditions</td><td>##IDS_ACTIONTEXT_EvaluateLaunchConditions##</td><td/></row>
		<row><td>MigrateFeatureStates</td><td>##IDS_ACTIONTEXT_MigratingFeatureStates##</td><td>##IDS_ACTIONTEXT_Application##</td></row>
		<row><td>MoveFiles</td><td>##IDS_ACTIONTEXT_MovingFiles##</td><td>##IDS_ACTIONTEXT_FileDirSize3##</td></row>
		<row><td>PatchFiles</td><td>##IDS_ACTIONTEXT_PatchingFiles##</td><td>##IDS_ACTIONTEXT_FileDirSize4##</td></row>
		<row><td>ProcessComponents</td><td>##IDS_ACTIONTEXT_UpdateComponentRegistration##</td><td/></row>
		<row><td>PublishComponents</td><td>##IDS_ACTIONTEXT_PublishingQualifiedComponents##</td><td>##IDS_ACTIONTEXT_ComponentIDQualifier##</td></row>
		<row><td>PublishFeatures</td><td>##IDS_ACTIONTEXT_PublishProductFeatures##</td><td>##IDS_ACTIONTEXT_FeatureColon##</td></row>
		<row><td>PublishProduct</td><td>##IDS_ACTIONTEXT_PublishProductInfo##</td><td/></row>
		<row><td>RMCCPSearch</td><td>##IDS_ACTIONTEXT_SearchingQualifyingProducts##</td><td/></row>
		<row><td>RegisterClassInfo</td><td>##IDS_ACTIONTEXT_RegisterClassServer##</td><td>##IDS_ACTIONTEXT_ClassId##</td></row>
		<row><td>RegisterComPlus</td><td>##IDS_ACTIONTEXT_RegisteringComPlus##</td><td>##IDS_ACTIONTEXT_AppIdAppTypeRSN##</td></row>
		<row><td>RegisterExtensionInfo</td><td>##IDS_ACTIONTEXT_RegisterExtensionServers##</td><td>##IDS_ACTIONTEXT_Extension2##</td></row>
		<row><td>RegisterFonts</td><td>##IDS_ACTIONTEXT_RegisterFonts##</td><td>##IDS_ACTIONTEXT_Font##</td></row>
		<row><td>RegisterMIMEInfo</td><td>##IDS_ACTIONTEXT_RegisterMimeInfo##</td><td>##IDS_ACTIONTEXT_ContentTypeExtension##</td></row>
		<row><td>RegisterProduct</td><td>##IDS_ACTIONTEXT_RegisteringProduct##</td><td>##IDS_ACTIONTEXT_1b##</td></row>
		<row><td>RegisterProgIdInfo</td><td>##IDS_ACTIONTEXT_RegisteringProgIdentifiers##</td><td>##IDS_ACTIONTEXT_ProgID2##</td></row>
		<row><td>RegisterTypeLibraries</td><td>##IDS_ACTIONTEXT_RegisterTypeLibs##</td><td>##IDS_ACTIONTEXT_LibId##</td></row>
		<row><td>RegisterUser</td><td>##IDS_ACTIONTEXT_RegUser##</td><td>##IDS_ACTIONTEXT_1c##</td></row>
		<row><td>RemoveDuplicateFiles</td><td>##IDS_ACTIONTEXT_RemovingDuplicates##</td><td>##IDS_ACTIONTEXT_FileDir##</td></row>
		<row><td>RemoveEnvironmentStrings</td><td>##IDS_ACTIONTEXT_UpdateEnvironmentStrings##</td><td>##IDS_ACTIONTEXT_NameValueAction2##</td></row>
		<row><td>RemoveExistingProducts</td><td>##IDS_ACTIONTEXT_RemoveApps##</td><td>##IDS_ACTIONTEXT_AppCommandLine##</td></row>
		<row><td>RemoveFiles</td><td>##IDS_ACTIONTEXT_RemovingFiles##</td><td>##IDS_ACTIONTEXT_FileDir2##</td></row>
		<row><td>RemoveFolders</td><td>##IDS_ACTIONTEXT_RemovingFolders##</td><td>##IDS_ACTIONTEXT_Folder1##</td></row>
		<row><td>RemoveIniValues</td><td>##IDS_ACTIONTEXT_RemovingIni##</td><td>##IDS_ACTIONTEXT_FileSectionKeyValue##</td></row>
		<row><td>RemoveODBC</td><td>##IDS_ACTIONTEXT_RemovingODBC##</td><td/></row>
		<row><td>RemoveRegistryValues</td><td>##IDS_ACTIONTEXT_RemovingRegistry##</td><td>##IDS_ACTIONTEXT_KeyName##</td></row>
		<row><td>RemoveShortcuts</td><td>##IDS_ACTIONTEXT_RemovingShortcuts##</td><td>##IDS_ACTIONTEXT_Shortcut1##</td></row>
		<row><td>Rollback</td><td>##IDS_ACTIONTEXT_RollingBack##</td><td>##IDS_ACTIONTEXT_1d##</td></row>
		<row><td>RollbackCleanup</td><td>##IDS_ACTIONTEXT_RemovingBackup##</td><td>##IDS_ACTIONTEXT_File2##</td></row>
		<row><td>SelfRegModules</td><td>##IDS_ACTIONTEXT_RegisteringModules##</td><td>##IDS_ACTIONTEXT_FileFolder##</td></row>
		<row><td>SelfUnregModules</td><td>##IDS_ACTIONTEXT_UnregisterModules##</td><td>##IDS_ACTIONTEXT_FileFolder2##</td></row>
		<row><td>SetODBCFolders</td><td>##IDS_ACTIONTEXT_InitializeODBCDirs##</td><td/></row>
		<row><td>StartServices</td><td>##IDS_ACTIONTEXT_StartingServices##</td><td>##IDS_ACTIONTEXT_Service3##</td></row>
		<row><td>StopServices</td><td>##IDS_ACTIONTEXT_StoppingServices##</td><td>##IDS_ACTIONTEXT_Service4##</td></row>
		<row><td>UnmoveFiles</td><td>##IDS_ACTIONTEXT_RemovingMoved##</td><td>##IDS_ACTIONTEXT_FileDir3##</td></row>
		<row><td>UnpublishComponents</td><td>##IDS_ACTIONTEXT_UnpublishQualified##</td><td>##IDS_ACTIONTEXT_ComponentIdQualifier2##</td></row>
		<row><td>UnpublishFeatures</td><td>##IDS_ACTIONTEXT_UnpublishProductFeatures##</td><td>##IDS_ACTIONTEXT_Feature##</td></row>
		<row><td>UnpublishProduct</td><td>##IDS_ACTIONTEXT_UnpublishingProductInfo##</td><td/></row>
		<row><td>UnregisterClassInfo</td><td>##IDS_ACTIONTEXT_UnregisterClassServers##</td><td>##IDS_ACTIONTEXT_ClsID##</td></row>
		<row><td>UnregisterComPlus</td><td>##IDS_ACTIONTEXT_UnregisteringComPlus##</td><td>##IDS_ACTIONTEXT_AppId##</td></row>
		<row><td>UnregisterExtensionInfo</td><td>##IDS_ACTIONTEXT_UnregisterExtensionServers##</td><td>##IDS_ACTIONTEXT_Extension##</td></row>
		<row><td>UnregisterFonts</td><td>##IDS_ACTIONTEXT_UnregisteringFonts##</td><td>##IDS_ACTIONTEXT_Font2##</td></row>
		<row><td>UnregisterMIMEInfo</td><td>##IDS_ACTIONTEXT_UnregisteringMimeInfo##</td><td>##IDS_ACTIONTEXT_ContentTypeExtension2##</td></row>
		<row><td>UnregisterProgIdInfo</td><td>##IDS_ACTIONTEXT_UnregisteringProgramIds##</td><td>##IDS_ACTIONTEXT_ProgID##</td></row>
		<row><td>UnregisterTypeLibraries</td><td>##IDS_ACTIONTEXT_UnregTypeLibs##</td><td>##IDS_ACTIONTEXT_Libid2##</td></row>
		<row><td>WriteEnvironmentStrings</td><td>##IDS_ACTIONTEXT_EnvironmentStrings##</td><td>##IDS_ACTIONTEXT_NameValueAction##</td></row>
		<row><td>WriteIniValues</td><td>##IDS_ACTIONTEXT_WritingINI##</td><td>##IDS_ACTIONTEXT_FileSectionKeyValue2##</td></row>
		<row><td>WriteRegistryValues</td><td>##IDS_ACTIONTEXT_WritingRegistry##</td><td>##IDS_ACTIONTEXT_KeyNameValue##</td></row>
	</table>

	<table name="AdminExecuteSequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
		<row><td>CostFinalize</td><td/><td>1000</td><td>CostFinalize</td><td/></row>
		<row><td>CostInitialize</td><td/><td>800</td><td>CostInitialize</td><td/></row>
		<row><td>FileCost</td><td/><td>900</td><td>FileCost</td><td/></row>
		<row><td>InstallAdminPackage</td><td/><td>3900</td><td>InstallAdminPackage</td><td/></row>
		<row><td>InstallFiles</td><td/><td>4000</td><td>InstallFiles</td><td/></row>
		<row><td>InstallFinalize</td><td/><td>6600</td><td>InstallFinalize</td><td/></row>
		<row><td>InstallInitialize</td><td/><td>1500</td><td>InstallInitialize</td><td/></row>
		<row><td>InstallValidate</td><td/><td>1400</td><td>InstallValidate</td><td/></row>
		<row><td>ScheduleReboot</td><td>ISSCHEDULEREBOOT</td><td>4010</td><td>ScheduleReboot</td><td/></row>
	</table>

	<table name="AdminUISequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
		<row><td>AdminWelcome</td><td/><td>1010</td><td>AdminWelcome</td><td/></row>
		<row><td>CostFinalize</td><td/><td>1000</td><td>CostFinalize</td><td/></row>
		<row><td>CostInitialize</td><td/><td>800</td><td>CostInitialize</td><td/></row>
		<row><td>ExecuteAction</td><td/><td>1300</td><td>ExecuteAction</td><td/></row>
		<row><td>FileCost</td><td/><td>900</td><td>FileCost</td><td/></row>
		<row><td>SetupCompleteError</td><td/><td>-3</td><td>SetupCompleteError</td><td/></row>
		<row><td>SetupCompleteSuccess</td><td/><td>-1</td><td>SetupCompleteSuccess</td><td/></row>
		<row><td>SetupInitialization</td><td/><td>50</td><td>SetupInitialization</td><td/></row>
		<row><td>SetupInterrupted</td><td/><td>-2</td><td>SetupInterrupted</td><td/></row>
		<row><td>SetupProgress</td><td/><td>1020</td><td>SetupProgress</td><td/></row>
	</table>

	<table name="AdvtExecuteSequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
		<row><td>CostFinalize</td><td/><td>1000</td><td>CostFinalize</td><td/></row>
		<row><td>CostInitialize</td><td/><td>800</td><td>CostInitialize</td><td/></row>
		<row><td>CreateShortcuts</td><td/><td>4500</td><td>CreateShortcuts</td><td/></row>
		<row><td>InstallFinalize</td><td/><td>6600</td><td>InstallFinalize</td><td/></row>
		<row><td>InstallInitialize</td><td/><td>1500</td><td>InstallInitialize</td><td/></row>
		<row><td>InstallValidate</td><td/><td>1400</td><td>InstallValidate</td><td/></row>
		<row><td>MsiPublishAssemblies</td><td/><td>6250</td><td>MsiPublishAssemblies</td><td/></row>
		<row><td>PublishComponents</td><td/><td>6200</td><td>PublishComponents</td><td/></row>
		<row><td>PublishFeatures</td><td/><td>6300</td><td>PublishFeatures</td><td/></row>
		<row><td>PublishProduct</td><td/><td>6400</td><td>PublishProduct</td><td/></row>
		<row><td>RegisterClassInfo</td><td/><td>4600</td><td>RegisterClassInfo</td><td/></row>
		<row><td>RegisterExtensionInfo</td><td/><td>4700</td><td>RegisterExtensionInfo</td><td/></row>
		<row><td>RegisterMIMEInfo</td><td/><td>4900</td><td>RegisterMIMEInfo</td><td/></row>
		<row><td>RegisterProgIdInfo</td><td/><td>4800</td><td>RegisterProgIdInfo</td><td/></row>
		<row><td>RegisterTypeLibraries</td><td/><td>4910</td><td>RegisterTypeLibraries</td><td/></row>
		<row><td>ScheduleReboot</td><td>ISSCHEDULEREBOOT</td><td>6410</td><td>ScheduleReboot</td><td/></row>
	</table>

	<table name="AdvtUISequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="AppId">
		<col key="yes" def="s38">AppId</col>
		<col def="S255">RemoteServerName</col>
		<col def="S255">LocalService</col>
		<col def="S255">ServiceParameters</col>
		<col def="S255">DllSurrogate</col>
		<col def="I2">ActivateAtStorage</col>
		<col def="I2">RunAsInteractiveUser</col>
	</table>

	<table name="AppSearch">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="s72">Signature_</col>
	</table>

	<table name="BBControl">
		<col key="yes" def="s50">Billboard_</col>
		<col key="yes" def="s50">BBControl</col>
		<col def="s50">Type</col>
		<col def="i2">X</col>
		<col def="i2">Y</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
		<col def="I4">Attributes</col>
		<col def="L50">Text</col>
	</table>

	<table name="Billboard">
		<col key="yes" def="s50">Billboard</col>
		<col def="s38">Feature_</col>
		<col def="S50">Action</col>
		<col def="I2">Ordering</col>
	</table>

	<table name="Binary">
		<col key="yes" def="s72">Name</col>
		<col def="V0">Data</col>
		<col def="S255">ISBuildSourcePath</col>
		<row><td>ISExpHlp.dll</td><td/><td>&lt;ISRedistPlatformDependentFolder&gt;\ISExpHlp.dll</td></row>
		<row><td>ISSELFREG.DLL</td><td/><td>&lt;ISRedistPlatformDependentFolder&gt;\isregsvr.dll</td></row>
		<row><td>NewBinary1</td><td/><td>&lt;ISProductFolder&gt;\Support\Themes\InstallShield Blue Theme\banner.jpg</td></row>
		<row><td>NewBinary10</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\CompleteSetupIco.ibd</td></row>
		<row><td>NewBinary11</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\CustomSetupIco.ibd</td></row>
		<row><td>NewBinary12</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\DestIcon.ibd</td></row>
		<row><td>NewBinary13</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\NetworkInstall.ico</td></row>
		<row><td>NewBinary14</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\DontInstall.ico</td></row>
		<row><td>NewBinary15</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\Install.ico</td></row>
		<row><td>NewBinary16</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\InstallFirstUse.ico</td></row>
		<row><td>NewBinary17</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\InstallPartial.ico</td></row>
		<row><td>NewBinary18</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\InstallStateMenu.ico</td></row>
		<row><td>NewBinary2</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\New.ibd</td></row>
		<row><td>NewBinary3</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\Up.ibd</td></row>
		<row><td>NewBinary4</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\WarningIcon.ibd</td></row>
		<row><td>NewBinary5</td><td/><td>&lt;ISProductFolder&gt;\Support\Themes\InstallShield Blue Theme\welcome.jpg</td></row>
		<row><td>NewBinary6</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\CustomSetupIco.ibd</td></row>
		<row><td>NewBinary7</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\ReinstIco.ibd</td></row>
		<row><td>NewBinary8</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\RemoveIco.ibd</td></row>
		<row><td>NewBinary9</td><td/><td>&lt;ISProductFolder&gt;\Redist\Language Independent\OS Independent\SetupIcon.ibd</td></row>
		<row><td>SetAllUsers.dll</td><td/><td>&lt;ISRedistPlatformDependentFolder&gt;\SetAllUsers.dll</td></row>
	</table>

	<table name="BindImage">
		<col key="yes" def="s72">File_</col>
		<col def="S255">Path</col>
	</table>

	<table name="CCPSearch">
		<col key="yes" def="s72">Signature_</col>
	</table>

	<table name="CheckBox">
		<col key="yes" def="s72">Property</col>
		<col def="S64">Value</col>
		<row><td>ISCHECKFORPRODUCTUPDATES</td><td>1</td></row>
		<row><td>LAUNCHPROGRAM</td><td>1</td></row>
		<row><td>LAUNCHREADME</td><td>1</td></row>
	</table>

	<table name="Class">
		<col key="yes" def="s38">CLSID</col>
		<col key="yes" def="s32">Context</col>
		<col key="yes" def="s72">Component_</col>
		<col def="S255">ProgId_Default</col>
		<col def="L255">Description</col>
		<col def="S38">AppId_</col>
		<col def="S255">FileTypeMask</col>
		<col def="S72">Icon_</col>
		<col def="I2">IconIndex</col>
		<col def="S32">DefInprocHandler</col>
		<col def="S255">Argument</col>
		<col def="s38">Feature_</col>
		<col def="I2">Attributes</col>
	</table>

	<table name="ComboBox">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="i2">Order</col>
		<col def="s64">Value</col>
		<col def="L64">Text</col>
	</table>

	<table name="CompLocator">
		<col key="yes" def="s72">Signature_</col>
		<col def="s38">ComponentId</col>
		<col def="I2">Type</col>
	</table>

	<table name="Complus">
		<col key="yes" def="s72">Component_</col>
		<col key="yes" def="I2">ExpType</col>
	</table>

	<table name="Component">
		<col key="yes" def="s72">Component</col>
		<col def="S38">ComponentId</col>
		<col def="s72">Directory_</col>
		<col def="i2">Attributes</col>
		<col def="S255">Condition</col>
		<col def="S72">KeyPath</col>
		<col def="I4">ISAttributes</col>
		<col def="S255">ISComments</col>
		<col def="S255">ISScanAtBuildFile</col>
		<col def="S255">ISRegFileToMergeAtBuild</col>
		<col def="S0">ISDotNetInstallerArgsInstall</col>
		<col def="S0">ISDotNetInstallerArgsCommit</col>
		<col def="S0">ISDotNetInstallerArgsUninstall</col>
		<col def="S0">ISDotNetInstallerArgsRollback</col>
		<row><td>ISX_DEFAULTCOMPONENT</td><td>{396D5F4F-2322-4F69-8F71-3DE1009A38E2}</td><td>MOUVENT1</td><td>2</td><td/><td/><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>ISX_DEFAULTCOMPONENT1</td><td>{0BF1F1D7-C6C4-46FA-B0D6-F3CE8CBB4DD3}</td><td>INSTALLDIR</td><td>258</td><td/><td/><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>ISX_DEFAULTCOMPONENT2</td><td>{604F3E44-F5A9-4358-8730-3514A0A7371A}</td><td>INSTALLDIR</td><td>2</td><td/><td/><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>ISX_DEFAULTCOMPONENT3</td><td>{B68776FF-9239-4D44-8812-2C0764605213}</td><td>PUTTY</td><td>2</td><td/><td/><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>ISX_DEFAULTCOMPONENT4</td><td>{BC50AEEE-3DF7-445C-9ED5-C0D352B6FB6C}</td><td>WINSCP</td><td>2</td><td/><td/><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>TeamViewer_Setup.exe</td><td>{B776920A-1B92-4799-854B-B1D0BFDEC052}</td><td>MOUVENT1</td><td>2</td><td/><td>teamviewer_setup.exe</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>Win10_RunOnce.exe</td><td>{4D7D1026-9046-402F-8709-FAB2583A6BF0}</td><td>MOUVENT1</td><td>2</td><td/><td>win10_runonce.exe</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>Win10_Startup.exe1</td><td>{961D2774-749C-4B3F-A41D-344ECFAD6EAE}</td><td>MOUVENT1</td><td>2</td><td/><td>win10_startup.exe1</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>WinSCP.exe</td><td>{2D05D7B9-BDA2-47D2-9C27-D39D35CE4F0A}</td><td>WINSCP</td><td>2</td><td/><td>winscp.exe</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>WinpkFilter.exe</td><td>{ABB353CB-E66C-4FEF-A40C-1234DC6E9762}</td><td>MOUVENT1</td><td>2</td><td/><td>winpkfilter.exe</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>ndisapi.dll</td><td>{3196F57E-E367-48B7-AEA2-F57AF88A6956}</td><td>MOUVENT1</td><td>2</td><td/><td>ndisapi.dll</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>putty.exe2</td><td>{BAA5A6B9-D160-478D-8D94-C1B731085BAD}</td><td>PUTTY</td><td>2</td><td/><td>putty.exe2</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
		<row><td>rx_net_bridge.exe</td><td>{6B8A5914-517E-4A1E-8B51-613B444E9134}</td><td>MOUVENT1</td><td>2</td><td/><td>rx_net_bridge.exe</td><td>17</td><td/><td/><td/><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td><td>/LogFile=</td></row>
	</table>

	<table name="Condition">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="i2">Level</col>
		<col def="S255">Condition</col>
	</table>

	<table name="Control">
		<col key="yes" def="s72">Dialog_</col>
		<col key="yes" def="s50">Control</col>
		<col def="s20">Type</col>
		<col def="i2">X</col>
		<col def="i2">Y</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
		<col def="I4">Attributes</col>
		<col def="S72">Property</col>
		<col def="L0">Text</col>
		<col def="S50">Control_Next</col>
		<col def="L50">Help</col>
		<col def="I4">ISWindowStyle</col>
		<col def="I4">ISControlId</col>
		<col def="S255">ISBuildSourcePath</col>
		<col def="S72">Binary_</col>
		<row><td>AdminChangeFolder</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>AdminChangeFolder</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>ComboText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Combo</td><td>DirectoryCombo</td><td>21</td><td>64</td><td>277</td><td>80</td><td>458755</td><td>TARGETDIR</td><td>##IDS__IsAdminInstallBrowse_4##</td><td>Up</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>ComboText</td><td>Text</td><td>21</td><td>50</td><td>99</td><td>14</td><td>3</td><td/><td>##IDS__IsAdminInstallBrowse_LookIn##</td><td>Combo</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsAdminInstallBrowse_BrowseDestination##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsAdminInstallBrowse_ChangeDestination##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>List</td><td>DirectoryList</td><td>21</td><td>90</td><td>332</td><td>97</td><td>7</td><td>TARGETDIR</td><td>##IDS__IsAdminInstallBrowse_8##</td><td>TailText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>NewFolder</td><td>PushButton</td><td>335</td><td>66</td><td>19</td><td>19</td><td>3670019</td><td/><td/><td>List</td><td>##IDS__IsAdminInstallBrowse_CreateFolder##</td><td>0</td><td/><td/><td>NewBinary2</td></row>
		<row><td>AdminChangeFolder</td><td>OK</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_OK##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Tail</td><td>PathEdit</td><td>21</td><td>207</td><td>332</td><td>17</td><td>3</td><td>TARGETDIR</td><td>##IDS__IsAdminInstallBrowse_11##</td><td>OK</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>TailText</td><td>Text</td><td>21</td><td>193</td><td>99</td><td>13</td><td>3</td><td/><td>##IDS__IsAdminInstallBrowse_FolderName##</td><td>Tail</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminChangeFolder</td><td>Up</td><td>PushButton</td><td>310</td><td>66</td><td>19</td><td>19</td><td>3670019</td><td/><td/><td>NewFolder</td><td>##IDS__IsAdminInstallBrowse_UpOneLevel##</td><td>0</td><td/><td/><td>NewBinary3</td></row>
		<row><td>AdminNetworkLocation</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>InstallNow</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>AdminNetworkLocation</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>Browse</td><td>PushButton</td><td>286</td><td>124</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsAdminInstallPoint_Change##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>SetupPathEdit</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsAdminInstallPoint_SpecifyNetworkLocation##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>DlgText</td><td>Text</td><td>21</td><td>51</td><td>326</td><td>40</td><td>131075</td><td/><td>##IDS__IsAdminInstallPoint_EnterNetworkLocation##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsAdminInstallPoint_NetworkLocationFormatted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>InstallNow</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsAdminInstallPoint_Install##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>LBBrowse</td><td>Text</td><td>21</td><td>90</td><td>100</td><td>10</td><td>3</td><td/><td>##IDS__IsAdminInstallPoint_NetworkLocation##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminNetworkLocation</td><td>SetupPathEdit</td><td>PathEdit</td><td>21</td><td>102</td><td>330</td><td>17</td><td>3</td><td>TARGETDIR</td><td/><td>Browse</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>AdminWelcome</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsAdminInstallPointWelcome_Wizard##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>AdminWelcome</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>45</td><td>196611</td><td/><td>##IDS__IsAdminInstallPointWelcome_ServerImage##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CancelSetup</td><td>Icon</td><td>Icon</td><td>15</td><td>15</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary4</td></row>
		<row><td>CancelSetup</td><td>No</td><td>PushButton</td><td>135</td><td>57</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsCancelDlg_No##</td><td>Yes</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CancelSetup</td><td>Text</td><td>Text</td><td>48</td><td>15</td><td>194</td><td>30</td><td>131075</td><td/><td>##IDS__IsCancelDlg_ConfirmCancel##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CancelSetup</td><td>Yes</td><td>PushButton</td><td>62</td><td>57</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsCancelDlg_Yes##</td><td>No</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>CustomSetup</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Tree</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>ChangeFolder</td><td>PushButton</td><td>301</td><td>203</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_Change##</td><td>Help</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Details</td><td>PushButton</td><td>93</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_Space##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>DlgDesc</td><td>Text</td><td>17</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsCustomSelectionDlg_SelectFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>DlgText</td><td>Text</td><td>9</td><td>51</td><td>360</td><td>10</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_ClickFeatureIcon##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>DlgTitle</td><td>Text</td><td>9</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsCustomSelectionDlg_CustomSetup##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>FeatureGroup</td><td>GroupBox</td><td>235</td><td>67</td><td>131</td><td>120</td><td>1</td><td/><td>##IDS__IsCustomSelectionDlg_FeatureDescription##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Help</td><td>PushButton</td><td>22</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_Help##</td><td>Details</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>InstallLabel</td><td>Text</td><td>8</td><td>190</td><td>360</td><td>10</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_InstallTo##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>ItemDescription</td><td>Text</td><td>241</td><td>80</td><td>120</td><td>50</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_MultilineDescription##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Location</td><td>Text</td><td>8</td><td>203</td><td>291</td><td>20</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_FeaturePath##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Size</td><td>Text</td><td>241</td><td>133</td><td>120</td><td>50</td><td>3</td><td/><td>##IDS__IsCustomSelectionDlg_FeatureSize##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetup</td><td>Tree</td><td>SelectionTree</td><td>8</td><td>70</td><td>220</td><td>118</td><td>7</td><td>_BrowseProperty</td><td/><td>ChangeFolder</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>CustomSetupTips</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS_SetupTips_CustomSetupDescription##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS_SetupTips_CustomSetup##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>DontInstall</td><td>Icon</td><td>21</td><td>155</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary14</td></row>
		<row><td>CustomSetupTips</td><td>DontInstallText</td><td>Text</td><td>60</td><td>155</td><td>300</td><td>20</td><td>3</td><td/><td>##IDS_SetupTips_WillNotBeInstalled##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>FirstInstallText</td><td>Text</td><td>60</td><td>180</td><td>300</td><td>20</td><td>3</td><td/><td>##IDS_SetupTips_Advertise##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>Install</td><td>Icon</td><td>21</td><td>105</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary15</td></row>
		<row><td>CustomSetupTips</td><td>InstallFirstUse</td><td>Icon</td><td>21</td><td>180</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary16</td></row>
		<row><td>CustomSetupTips</td><td>InstallPartial</td><td>Icon</td><td>21</td><td>130</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary17</td></row>
		<row><td>CustomSetupTips</td><td>InstallStateMenu</td><td>Icon</td><td>21</td><td>52</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary18</td></row>
		<row><td>CustomSetupTips</td><td>InstallStateText</td><td>Text</td><td>21</td><td>91</td><td>300</td><td>10</td><td>3</td><td/><td>##IDS_SetupTips_InstallState##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>CustomSetupTips</td><td>InstallText</td><td>Text</td><td>60</td><td>105</td><td>300</td><td>20</td><td>3</td><td/><td>##IDS_SetupTips_AllInstalledLocal##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>MenuText</td><td>Text</td><td>50</td><td>52</td><td>300</td><td>36</td><td>3</td><td/><td>##IDS_SetupTips_IconInstallState##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>NetworkInstall</td><td>Icon</td><td>21</td><td>205</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary13</td></row>
		<row><td>CustomSetupTips</td><td>NetworkInstallText</td><td>Text</td><td>60</td><td>205</td><td>300</td><td>20</td><td>3</td><td/><td>##IDS_SetupTips_Network##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>OK</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_SetupTips_OK##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomSetupTips</td><td>PartialText</td><td>Text</td><td>60</td><td>130</td><td>300</td><td>20</td><td>3</td><td/><td>##IDS_SetupTips_SubFeaturesInstalledLocal##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>CustomerInformation</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>NameLabel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>CompanyEdit</td><td>Edit</td><td>21</td><td>100</td><td>237</td><td>17</td><td>3</td><td>COMPANYNAME</td><td>##IDS__IsRegisterUserDlg_Tahoma80##</td><td>SerialLabel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>CompanyLabel</td><td>Text</td><td>21</td><td>89</td><td>75</td><td>10</td><td>3</td><td/><td>##IDS__IsRegisterUserDlg_Organization##</td><td>CompanyEdit</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsRegisterUserDlg_PleaseEnterInfo##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>DlgRadioGroupText</td><td>Text</td><td>21</td><td>161</td><td>300</td><td>14</td><td>2</td><td/><td>##IDS__IsRegisterUserDlg_InstallFor##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsRegisterUserDlg_CustomerInformation##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>NameEdit</td><td>Edit</td><td>21</td><td>63</td><td>237</td><td>17</td><td>3</td><td>USERNAME</td><td>##IDS__IsRegisterUserDlg_Tahoma50##</td><td>CompanyLabel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>NameLabel</td><td>Text</td><td>21</td><td>52</td><td>75</td><td>10</td><td>3</td><td/><td>##IDS__IsRegisterUserDlg_UserName##</td><td>NameEdit</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>RadioGroup</td><td>RadioButtonGroup</td><td>63</td><td>170</td><td>300</td><td>50</td><td>2</td><td>ApplicationUsers</td><td>##IDS__IsRegisterUserDlg_16##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>SerialLabel</td><td>Text</td><td>21</td><td>127</td><td>109</td><td>10</td><td>2</td><td/><td>##IDS__IsRegisterUserDlg_SerialNumber##</td><td>SerialNumber</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>CustomerInformation</td><td>SerialNumber</td><td>MaskedEdit</td><td>21</td><td>138</td><td>237</td><td>17</td><td>2</td><td>ISX_SERIALNUM</td><td/><td>RadioGroup</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>DatabaseFolder</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>ChangeFolder</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>ChangeFolder</td><td>PushButton</td><td>301</td><td>65</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CHANGE##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>DatabaseFolder</td><td>Icon</td><td>21</td><td>52</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary12</td></row>
		<row><td>DatabaseFolder</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__DatabaseFolder_ChangeFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__DatabaseFolder_DatabaseFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>LocLabel</td><td>Text</td><td>57</td><td>52</td><td>290</td><td>10</td><td>131075</td><td/><td>##IDS_DatabaseFolder_InstallDatabaseTo##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Location</td><td>Text</td><td>57</td><td>65</td><td>240</td><td>40</td><td>3</td><td>_BrowseProperty</td><td>##IDS__DatabaseFolder_DatabaseDir##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DatabaseFolder</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>DestinationFolder</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>ChangeFolder</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>ChangeFolder</td><td>PushButton</td><td>301</td><td>65</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__DestinationFolder_Change##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>DestFolder</td><td>Icon</td><td>21</td><td>52</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary12</td></row>
		<row><td>DestinationFolder</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__DestinationFolder_ChangeFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__DestinationFolder_DestinationFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>LocLabel</td><td>Text</td><td>57</td><td>52</td><td>290</td><td>10</td><td>131075</td><td/><td>##IDS__DestinationFolder_InstallTo##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Location</td><td>Text</td><td>57</td><td>65</td><td>240</td><td>40</td><td>3</td><td>_BrowseProperty</td><td>##IDS_INSTALLDIR##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DestinationFolder</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>DiskSpaceRequirements</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>DlgDesc</td><td>Text</td><td>17</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFeatureDetailsDlg_SpaceRequired##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>DlgText</td><td>Text</td><td>10</td><td>185</td><td>358</td><td>41</td><td>3</td><td/><td>##IDS__IsFeatureDetailsDlg_VolumesTooSmall##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>DlgTitle</td><td>Text</td><td>9</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFeatureDetailsDlg_DiskSpaceRequirements##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>List</td><td>VolumeCostList</td><td>8</td><td>55</td><td>358</td><td>125</td><td>393223</td><td/><td>##IDS__IsFeatureDetailsDlg_Numbers##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>DiskSpaceRequirements</td><td>OK</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsFeatureDetailsDlg_OK##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>FilesInUse</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFilesInUse_FilesInUseMessage##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>DlgText</td><td>Text</td><td>21</td><td>51</td><td>348</td><td>33</td><td>3</td><td/><td>##IDS__IsFilesInUse_ApplicationsUsingFiles##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFilesInUse_FilesInUse##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Exit</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsFilesInUse_Exit##</td><td>List</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Ignore</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsFilesInUse_Ignore##</td><td>Exit</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>List</td><td>ListBox</td><td>21</td><td>87</td><td>331</td><td>135</td><td>7</td><td>FileInUseProcess</td><td/><td>Retry</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>FilesInUse</td><td>Retry</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsFilesInUse_Retry##</td><td>Ignore</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>InstallChangeFolder</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>ComboText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Combo</td><td>DirectoryCombo</td><td>21</td><td>64</td><td>277</td><td>80</td><td>4128779</td><td>_BrowseProperty</td><td>##IDS__IsBrowseFolderDlg_4##</td><td>Up</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>ComboText</td><td>Text</td><td>21</td><td>50</td><td>99</td><td>14</td><td>3</td><td/><td>##IDS__IsBrowseFolderDlg_LookIn##</td><td>Combo</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsBrowseFolderDlg_BrowseDestFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsBrowseFolderDlg_ChangeCurrentFolder##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>List</td><td>DirectoryList</td><td>21</td><td>90</td><td>332</td><td>97</td><td>15</td><td>_BrowseProperty</td><td>##IDS__IsBrowseFolderDlg_8##</td><td>TailText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>NewFolder</td><td>PushButton</td><td>335</td><td>66</td><td>19</td><td>19</td><td>3670019</td><td/><td/><td>List</td><td>##IDS__IsBrowseFolderDlg_CreateFolder##</td><td>0</td><td/><td/><td>NewBinary2</td></row>
		<row><td>InstallChangeFolder</td><td>OK</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsBrowseFolderDlg_OK##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Tail</td><td>PathEdit</td><td>21</td><td>207</td><td>332</td><td>17</td><td>15</td><td>_BrowseProperty</td><td>##IDS__IsBrowseFolderDlg_11##</td><td>OK</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>TailText</td><td>Text</td><td>21</td><td>193</td><td>99</td><td>13</td><td>3</td><td/><td>##IDS__IsBrowseFolderDlg_FolderName##</td><td>Tail</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallChangeFolder</td><td>Up</td><td>PushButton</td><td>310</td><td>66</td><td>19</td><td>19</td><td>3670019</td><td/><td/><td>NewFolder</td><td>##IDS__IsBrowseFolderDlg_UpOneLevel##</td><td>0</td><td/><td/><td>NewBinary3</td></row>
		<row><td>InstallWelcome</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Copyright</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>Copyright</td><td>Text</td><td>135</td><td>144</td><td>228</td><td>73</td><td>65539</td><td/><td>##IDS__IsWelcomeDlg_WarningCopyright##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>InstallWelcome</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsWelcomeDlg_WelcomeProductName##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>InstallWelcome</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>45</td><td>196611</td><td/><td>##IDS__IsWelcomeDlg_InstallProductName##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Agree</td><td>RadioButtonGroup</td><td>8</td><td>190</td><td>291</td><td>40</td><td>3</td><td>AgreeToLicense</td><td/><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>LicenseAgreement</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>ISPrintButton</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsLicenseDlg_ReadLicenseAgreement##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsLicenseDlg_LicenseAgreement##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>ISPrintButton</td><td>PushButton</td><td>301</td><td>188</td><td>65</td><td>17</td><td>3</td><td/><td>##IDS_PRINT_BUTTON##</td><td>Agree</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>LicenseAgreement</td><td>Memo</td><td>ScrollableText</td><td>8</td><td>55</td><td>358</td><td>130</td><td>7</td><td/><td/><td/><td/><td>0</td><td/><td>&lt;ISProductFolder&gt;\Redist\0409\Eula.rtf</td><td/></row>
		<row><td>LicenseAgreement</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>MaintenanceType</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>RadioGroup</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsMaintenanceDlg_MaitenanceOptions##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsMaintenanceDlg_ProgramMaintenance##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Ico1</td><td>Icon</td><td>35</td><td>75</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary6</td></row>
		<row><td>MaintenanceType</td><td>Ico2</td><td>Icon</td><td>35</td><td>135</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary7</td></row>
		<row><td>MaintenanceType</td><td>Ico3</td><td>Icon</td><td>35</td><td>195</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary8</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>RadioGroup</td><td>RadioButtonGroup</td><td>21</td><td>55</td><td>290</td><td>170</td><td>3</td><td>_IsMaintenance</td><td/><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Text1</td><td>Text</td><td>80</td><td>72</td><td>260</td><td>35</td><td>3</td><td/><td>##IDS__IsMaintenanceDlg_ChangeFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Text2</td><td>Text</td><td>80</td><td>135</td><td>260</td><td>35</td><td>3</td><td/><td>##IDS__IsMaintenanceDlg_RepairMessage##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceType</td><td>Text3</td><td>Text</td><td>80</td><td>192</td><td>260</td><td>35</td><td>131075</td><td/><td>##IDS__IsMaintenanceDlg_RemoveProductName##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>MaintenanceWelcome</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsMaintenanceWelcome_WizardWelcome##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MaintenanceWelcome</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>50</td><td>196611</td><td/><td>##IDS__IsMaintenanceWelcome_MaintenanceOptionsDescription##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>MsiRMFilesInUse</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Restart</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFilesInUse_FilesInUseMessage##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>DlgText</td><td>Text</td><td>21</td><td>51</td><td>348</td><td>14</td><td>3</td><td/><td>##IDS__IsMsiRMFilesInUse_ApplicationsUsingFiles##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsFilesInUse_FilesInUse##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>List</td><td>ListBox</td><td>21</td><td>66</td><td>331</td><td>130</td><td>3</td><td>FileInUseProcess</td><td/><td>OK</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>OK</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_OK##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>MsiRMFilesInUse</td><td>Restart</td><td>RadioButtonGroup</td><td>19</td><td>187</td><td>343</td><td>40</td><td>3</td><td>RestartManagerOption</td><td/><td>List</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>OutOfSpace</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsDiskSpaceDlg_DiskSpace##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>DlgText</td><td>Text</td><td>21</td><td>51</td><td>326</td><td>43</td><td>3</td><td/><td>##IDS__IsDiskSpaceDlg_HighlightedVolumes##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsDiskSpaceDlg_OutOfDiskSpace##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>List</td><td>VolumeCostList</td><td>21</td><td>95</td><td>332</td><td>120</td><td>393223</td><td/><td>##IDS__IsDiskSpaceDlg_Numbers##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>OutOfSpace</td><td>Resume</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsDiskSpaceDlg_OK##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>PatchWelcome</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsPatchDlg_Update##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsPatchDlg_WelcomePatchWizard##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>PatchWelcome</td><td>TextLine2</td><td>Text</td><td>135</td><td>54</td><td>228</td><td>45</td><td>196611</td><td/><td>##IDS__IsPatchDlg_PatchClickUpdate##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1048579</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>3</td><td/><td/><td>DlgTitle</td><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>ReadmeInformation</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>1048579</td><td/><td>##IDS__IsReadmeDlg_Cancel##</td><td>Readme</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadmeInformation</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>232</td><td>16</td><td>65539</td><td/><td>##IDS__IsReadmeDlg_PleaseReadInfo##</td><td>Back</td><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>ReadmeInformation</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>3</td><td/><td/><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>ReadmeInformation</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>193</td><td>13</td><td>65539</td><td/><td>##IDS__IsReadmeDlg_ReadMeInfo##</td><td>DlgDesc</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>1048579</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadmeInformation</td><td>Readme</td><td>ScrollableText</td><td>10</td><td>55</td><td>353</td><td>166</td><td>3</td><td/><td/><td>Banner</td><td/><td>0</td><td/><td>&lt;ISProductFolder&gt;\Redist\0409\Readme.rtf</td><td/></row>
		<row><td>ReadyToInstall</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>GroupBox1</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>ReadyToInstall</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>CompanyNameText</td><td>Text</td><td>38</td><td>198</td><td>211</td><td>9</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_Company##</td><td>SerialNumberText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>CurrentSettingsText</td><td>Text</td><td>19</td><td>80</td><td>81</td><td>10</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_CurrentSettings##</td><td>InstallNow</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsVerifyReadyDlg_WizardReady##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgText1</td><td>Text</td><td>21</td><td>54</td><td>330</td><td>24</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_BackOrCancel##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgText2</td><td>Text</td><td>21</td><td>99</td><td>330</td><td>20</td><td>2</td><td/><td>##IDS__IsRegisterUserDlg_InstallFor##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65538</td><td/><td>##IDS__IsVerifyReadyDlg_ModifyReady##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgTitle2</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65538</td><td/><td>##IDS__IsVerifyReadyDlg_ReadyRepair##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>DlgTitle3</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65538</td><td/><td>##IDS__IsVerifyReadyDlg_ReadyInstall##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>GroupBox1</td><td>Text</td><td>19</td><td>92</td><td>330</td><td>133</td><td>65541</td><td/><td/><td>SetupTypeText1</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>8388611</td><td/><td>##IDS__IsVerifyReadyDlg_Install##</td><td>InstallPerMachine</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>PushButton</td><td>63</td><td>123</td><td>248</td><td>17</td><td>8388610</td><td/><td>##IDS__IsRegisterUserDlg_Anyone##</td><td>InstallPerUser</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>PushButton</td><td>63</td><td>143</td><td>248</td><td>17</td><td>2</td><td/><td>##IDS__IsRegisterUserDlg_OnlyMe##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>SerialNumberText</td><td>Text</td><td>38</td><td>211</td><td>306</td><td>9</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_Serial##</td><td>CurrentSettingsText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>SetupTypeText1</td><td>Text</td><td>23</td><td>97</td><td>306</td><td>13</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_SetupType##</td><td>SetupTypeText2</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>SetupTypeText2</td><td>Text</td><td>37</td><td>114</td><td>306</td><td>14</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_SelectedSetupType##</td><td>TargetFolderText1</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>TargetFolderText1</td><td>Text</td><td>24</td><td>136</td><td>306</td><td>11</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_DestFolder##</td><td>TargetFolderText2</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>TargetFolderText2</td><td>Text</td><td>37</td><td>151</td><td>306</td><td>13</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_Installdir##</td><td>UserInformationText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>UserInformationText</td><td>Text</td><td>23</td><td>171</td><td>306</td><td>13</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_UserInfo##</td><td>UserNameText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToInstall</td><td>UserNameText</td><td>Text</td><td>38</td><td>184</td><td>306</td><td>9</td><td>3</td><td/><td>##IDS__IsVerifyReadyDlg_UserName##</td><td>CompanyNameText</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>RemoveNow</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>ReadyToRemove</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsVerifyRemoveAllDlg_ChoseRemoveProgram##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgText</td><td>Text</td><td>21</td><td>51</td><td>326</td><td>24</td><td>131075</td><td/><td>##IDS__IsVerifyRemoveAllDlg_ClickRemove##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgText1</td><td>Text</td><td>21</td><td>79</td><td>330</td><td>23</td><td>3</td><td/><td>##IDS__IsVerifyRemoveAllDlg_ClickBack##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgText2</td><td>Text</td><td>21</td><td>102</td><td>330</td><td>24</td><td>3</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsVerifyRemoveAllDlg_RemoveProgram##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>8388611</td><td/><td>##IDS__IsVerifyRemoveAllDlg_Remove##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Finish</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>CheckShowMsiLog</td><td>CheckBox</td><td>151</td><td>172</td><td>10</td><td>9</td><td>2</td><td>ISSHOWMSILOG</td><td/><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>Finish</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsFatalError_Finish##</td><td>Image</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>FinishText1</td><td>Text</td><td>135</td><td>80</td><td>228</td><td>50</td><td>65539</td><td/><td>##IDS__IsFatalError_NotModified##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>FinishText2</td><td>Text</td><td>135</td><td>135</td><td>228</td><td>25</td><td>65539</td><td/><td>##IDS__IsFatalError_ClickFinish##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td>CheckShowMsiLog</td><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SetupCompleteError</td><td>RestContText1</td><td>Text</td><td>135</td><td>80</td><td>228</td><td>50</td><td>65539</td><td/><td>##IDS__IsFatalError_KeepOrRestore##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>RestContText2</td><td>Text</td><td>135</td><td>135</td><td>228</td><td>25</td><td>65539</td><td/><td>##IDS__IsFatalError_RestoreOrContinueLater##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>ShowMsiLogText</td><td>Text</td><td>164</td><td>172</td><td>198</td><td>10</td><td>65538</td><td/><td>##IDS__IsSetupComplete_ShowMsiLog##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>65539</td><td/><td>##IDS__IsFatalError_WizardCompleted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteError</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>25</td><td>196611</td><td/><td>##IDS__IsFatalError_WizardInterrupted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>OK</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_CANCEL##</td><td>Image</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>CheckBoxUpdates</td><td>CheckBox</td><td>135</td><td>164</td><td>10</td><td>9</td><td>2</td><td>ISCHECKFORPRODUCTUPDATES</td><td>CheckBox1</td><td>CheckShowMsiLog</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>CheckForUpdatesText</td><td>Text</td><td>152</td><td>162</td><td>190</td><td>30</td><td>65538</td><td/><td>##IDS__IsExitDialog_Update_YesCheckForUpdates##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>CheckLaunchProgram</td><td>CheckBox</td><td>151</td><td>114</td><td>10</td><td>9</td><td>2</td><td>LAUNCHPROGRAM</td><td/><td>CheckLaunchReadme</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>CheckLaunchReadme</td><td>CheckBox</td><td>151</td><td>148</td><td>10</td><td>9</td><td>2</td><td>LAUNCHREADME</td><td/><td>CheckBoxUpdates</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>CheckShowMsiLog</td><td>CheckBox</td><td>151</td><td>182</td><td>10</td><td>9</td><td>2</td><td>ISSHOWMSILOG</td><td/><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td>CheckLaunchProgram</td><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SetupCompleteSuccess</td><td>LaunchProgramText</td><td>Text</td><td>164</td><td>112</td><td>98</td><td>15</td><td>65538</td><td/><td>##IDS__IsExitDialog_LaunchProgram##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>LaunchReadmeText</td><td>Text</td><td>164</td><td>148</td><td>120</td><td>13</td><td>65538</td><td/><td>##IDS__IsExitDialog_ShowReadMe##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsExitDialog_Finish##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>ShowMsiLogText</td><td>Text</td><td>164</td><td>182</td><td>198</td><td>10</td><td>65538</td><td/><td>##IDS__IsSetupComplete_ShowMsiLog##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>65539</td><td/><td>##IDS__IsExitDialog_WizardCompleted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>45</td><td>196610</td><td/><td>##IDS__IsExitDialog_InstallSuccess##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>TextLine3</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>45</td><td>196610</td><td/><td>##IDS__IsExitDialog_UninstallSuccess##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine1</td><td>Text</td><td>135</td><td>30</td><td>228</td><td>45</td><td>196610</td><td/><td>##IDS__IsExitDialog_Update_SetupFinished##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine2</td><td>Text</td><td>135</td><td>80</td><td>228</td><td>45</td><td>196610</td><td/><td>##IDS__IsExitDialog_Update_PossibleUpdates##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine3</td><td>Text</td><td>135</td><td>120</td><td>228</td><td>45</td><td>65538</td><td/><td>##IDS__IsExitDialog_Update_InternetConnection##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>A</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_Abort##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>C</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL2##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>ErrorIcon</td><td>Icon</td><td>15</td><td>15</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary4</td></row>
		<row><td>SetupError</td><td>ErrorText</td><td>Text</td><td>50</td><td>15</td><td>200</td><td>50</td><td>131075</td><td/><td>##IDS__IsErrorDlg_ErrorText##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>I</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_Ignore##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>N</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_NO##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>O</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_OK##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>R</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_Retry##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupError</td><td>Y</td><td>PushButton</td><td>192</td><td>80</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsErrorDlg_Yes##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>ActionData</td><td>Text</td><td>135</td><td>125</td><td>228</td><td>12</td><td>65539</td><td/><td>##IDS__IsInitDlg_1##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>ActionText</td><td>Text</td><td>135</td><td>109</td><td>220</td><td>36</td><td>65539</td><td/><td>##IDS__IsInitDlg_2##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SetupInitialization</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_NEXT##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsInitDlg_WelcomeWizard##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInitialization</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>30</td><td>196611</td><td/><td>##IDS__IsInitDlg_PreparingWizard##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Finish</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_CANCEL##</td><td>Image</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>CheckShowMsiLog</td><td>CheckBox</td><td>151</td><td>172</td><td>10</td><td>9</td><td>2</td><td>ISSHOWMSILOG</td><td/><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>Finish</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS__IsUserExit_Finish##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>FinishText1</td><td>Text</td><td>135</td><td>80</td><td>228</td><td>50</td><td>65539</td><td/><td>##IDS__IsUserExit_NotModified##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>FinishText2</td><td>Text</td><td>135</td><td>135</td><td>228</td><td>25</td><td>65539</td><td/><td>##IDS__IsUserExit_ClickFinish##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td>CheckShowMsiLog</td><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SetupInterrupted</td><td>RestContText1</td><td>Text</td><td>135</td><td>80</td><td>228</td><td>50</td><td>65539</td><td/><td>##IDS__IsUserExit_KeepOrRestore##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>RestContText2</td><td>Text</td><td>135</td><td>135</td><td>228</td><td>25</td><td>65539</td><td/><td>##IDS__IsUserExit_RestoreOrContinue##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>ShowMsiLogText</td><td>Text</td><td>164</td><td>172</td><td>198</td><td>10</td><td>65538</td><td/><td>##IDS__IsSetupComplete_ShowMsiLog##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>65539</td><td/><td>##IDS__IsUserExit_WizardCompleted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupInterrupted</td><td>TextLine2</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>25</td><td>196611</td><td/><td>##IDS__IsUserExit_WizardInterrupted##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>ProgressBar</td><td>59</td><td>113</td><td>275</td><td>12</td><td>65537</td><td/><td>##IDS__IsProgressDlg_ProgressDone##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>ActionText</td><td>Text</td><td>59</td><td>100</td><td>275</td><td>12</td><td>3</td><td/><td>##IDS__IsProgressDlg_2##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>SetupProgress</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65538</td><td/><td>##IDS__IsProgressDlg_UninstallingFeatures2##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgDesc2</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65538</td><td/><td>##IDS__IsProgressDlg_UninstallingFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgText</td><td>Text</td><td>59</td><td>51</td><td>275</td><td>30</td><td>196610</td><td/><td>##IDS__IsProgressDlg_WaitUninstall2##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgText2</td><td>Text</td><td>59</td><td>51</td><td>275</td><td>30</td><td>196610</td><td/><td>##IDS__IsProgressDlg_WaitUninstall##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>196610</td><td/><td>##IDS__IsProgressDlg_InstallingProductName##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>DlgTitle2</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>196610</td><td/><td>##IDS__IsProgressDlg_Uninstalling##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>LbSec</td><td>Text</td><td>192</td><td>139</td><td>32</td><td>12</td><td>2</td><td/><td>##IDS__IsProgressDlg_SecHidden##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>LbStatus</td><td>Text</td><td>59</td><td>85</td><td>70</td><td>12</td><td>3</td><td/><td>##IDS__IsProgressDlg_Status##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>SetupIcon</td><td>Icon</td><td>21</td><td>51</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary9</td></row>
		<row><td>SetupProgress</td><td>ShowTime</td><td>Text</td><td>170</td><td>139</td><td>17</td><td>12</td><td>2</td><td/><td>##IDS__IsProgressDlg_Hidden##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupProgress</td><td>TextTime</td><td>Text</td><td>59</td><td>139</td><td>110</td><td>12</td><td>2</td><td/><td>##IDS__IsProgressDlg_HiddenTimeRemaining##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>DlgLine</td><td>Line</td><td>0</td><td>234</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>Image</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>234</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SetupResume</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>PreselectedText</td><td>Text</td><td>135</td><td>55</td><td>228</td><td>45</td><td>196611</td><td/><td>##IDS__IsResumeDlg_WizardResume##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>ResumeText</td><td>Text</td><td>135</td><td>46</td><td>228</td><td>45</td><td>196611</td><td/><td>##IDS__IsResumeDlg_ResumeSuspended##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupResume</td><td>TextLine1</td><td>Text</td><td>135</td><td>8</td><td>225</td><td>45</td><td>196611</td><td/><td>##IDS__IsResumeDlg_Resuming##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Banner</td><td>Bitmap</td><td>0</td><td>0</td><td>374</td><td>44</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary1</td></row>
		<row><td>SetupType</td><td>BannerLine</td><td>Line</td><td>0</td><td>44</td><td>374</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>RadioGroup</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>CompText</td><td>Text</td><td>80</td><td>80</td><td>246</td><td>30</td><td>3</td><td/><td>##IDS__IsSetupTypeMinDlg_AllFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>CompleteIco</td><td>Icon</td><td>34</td><td>80</td><td>24</td><td>24</td><td>5242881</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary10</td></row>
		<row><td>SetupType</td><td>CustText</td><td>Text</td><td>80</td><td>171</td><td>246</td><td>30</td><td>2</td><td/><td>##IDS__IsSetupTypeMinDlg_ChooseFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>CustomIco</td><td>Icon</td><td>34</td><td>171</td><td>24</td><td>24</td><td>5242880</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary11</td></row>
		<row><td>SetupType</td><td>DlgDesc</td><td>Text</td><td>21</td><td>23</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsSetupTypeMinDlg_ChooseSetupType##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>DlgText</td><td>Text</td><td>22</td><td>49</td><td>326</td><td>10</td><td>3</td><td/><td>##IDS__IsSetupTypeMinDlg_SelectSetupType##</td><td/><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>SetupType</td><td>DlgTitle</td><td>Text</td><td>13</td><td>6</td><td>292</td><td>25</td><td>65539</td><td/><td>##IDS__IsSetupTypeMinDlg_SetupType##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>MinIco</td><td>Icon</td><td>34</td><td>125</td><td>24</td><td>24</td><td>5242880</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary11</td></row>
		<row><td>SetupType</td><td>MinText</td><td>Text</td><td>80</td><td>125</td><td>246</td><td>30</td><td>2</td><td/><td>##IDS__IsSetupTypeMinDlg_MinimumFeatures##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SetupType</td><td>RadioGroup</td><td>RadioButtonGroup</td><td>20</td><td>59</td><td>264</td><td>139</td><td>1048579</td><td>_IsSetupTypeMin</td><td/><td>Back</td><td/><td>0</td><td>0</td><td/><td/></row>
		<row><td>SplashBitmap</td><td>Back</td><td>PushButton</td><td>164</td><td>243</td><td>66</td><td>17</td><td>1</td><td/><td>##IDS_BACK##</td><td>Next</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SplashBitmap</td><td>Branding1</td><td>Text</td><td>4</td><td>229</td><td>50</td><td>13</td><td>3</td><td/><td>##IDS_INSTALLSHIELD_FORMATTED##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SplashBitmap</td><td>Branding2</td><td>Text</td><td>3</td><td>228</td><td>50</td><td>13</td><td>65537</td><td/><td>##IDS_INSTALLSHIELD##</td><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SplashBitmap</td><td>Cancel</td><td>PushButton</td><td>301</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_CANCEL##</td><td>Back</td><td/><td>0</td><td/><td/><td/></row>
		<row><td>SplashBitmap</td><td>DlgLine</td><td>Line</td><td>48</td><td>234</td><td>326</td><td>0</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td/></row>
		<row><td>SplashBitmap</td><td>Image</td><td>Bitmap</td><td>13</td><td>12</td><td>349</td><td>211</td><td>1</td><td/><td/><td/><td/><td>0</td><td/><td/><td>NewBinary5</td></row>
		<row><td>SplashBitmap</td><td>Next</td><td>PushButton</td><td>230</td><td>243</td><td>66</td><td>17</td><td>3</td><td/><td>##IDS_NEXT##</td><td>Cancel</td><td/><td>0</td><td/><td/><td/></row>
	</table>

	<table name="ControlCondition">
		<col key="yes" def="s72">Dialog_</col>
		<col key="yes" def="s50">Control_</col>
		<col key="yes" def="s50">Action</col>
		<col key="yes" def="s255">Condition</col>
		<row><td>CustomSetup</td><td>ChangeFolder</td><td>Hide</td><td>Installed</td></row>
		<row><td>CustomSetup</td><td>Details</td><td>Hide</td><td>Installed</td></row>
		<row><td>CustomSetup</td><td>InstallLabel</td><td>Hide</td><td>Installed</td></row>
		<row><td>CustomerInformation</td><td>DlgRadioGroupText</td><td>Hide</td><td>NOT Privileged</td></row>
		<row><td>CustomerInformation</td><td>DlgRadioGroupText</td><td>Hide</td><td>ProductState &gt; 0</td></row>
		<row><td>CustomerInformation</td><td>DlgRadioGroupText</td><td>Hide</td><td>Version9X</td></row>
		<row><td>CustomerInformation</td><td>DlgRadioGroupText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>CustomerInformation</td><td>RadioGroup</td><td>Hide</td><td>NOT Privileged</td></row>
		<row><td>CustomerInformation</td><td>RadioGroup</td><td>Hide</td><td>ProductState &gt; 0</td></row>
		<row><td>CustomerInformation</td><td>RadioGroup</td><td>Hide</td><td>Version9X</td></row>
		<row><td>CustomerInformation</td><td>RadioGroup</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>CustomerInformation</td><td>SerialLabel</td><td>Show</td><td>SERIALNUMSHOW</td></row>
		<row><td>CustomerInformation</td><td>SerialNumber</td><td>Show</td><td>SERIALNUMSHOW</td></row>
		<row><td>InstallWelcome</td><td>Copyright</td><td>Hide</td><td>SHOWCOPYRIGHT="No"</td></row>
		<row><td>InstallWelcome</td><td>Copyright</td><td>Show</td><td>SHOWCOPYRIGHT="Yes"</td></row>
		<row><td>LicenseAgreement</td><td>Next</td><td>Disable</td><td>AgreeToLicense &lt;&gt; "Yes"</td></row>
		<row><td>LicenseAgreement</td><td>Next</td><td>Enable</td><td>AgreeToLicense = "Yes"</td></row>
		<row><td>ReadyToInstall</td><td>CompanyNameText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>CurrentSettingsText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>DlgText2</td><td>Hide</td><td>VersionNT &lt; "601" OR NOT ISSupportPerUser OR Installed</td></row>
		<row><td>ReadyToInstall</td><td>DlgText2</td><td>Show</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>DlgTitle</td><td>Show</td><td>ProgressType0="Modify"</td></row>
		<row><td>ReadyToInstall</td><td>DlgTitle2</td><td>Show</td><td>ProgressType0="Repair"</td></row>
		<row><td>ReadyToInstall</td><td>DlgTitle3</td><td>Show</td><td>ProgressType0="install"</td></row>
		<row><td>ReadyToInstall</td><td>GroupBox1</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>Disable</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>Enable</td><td>VersionNT &lt; "601" OR NOT ISSupportPerUser OR Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>Hide</td><td>VersionNT &lt; "601" OR NOT ISSupportPerUser OR Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>Show</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>Hide</td><td>VersionNT &lt; "601" OR NOT ISSupportPerUser OR Installed</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>Show</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>SerialNumberText</td><td>Hide</td><td>NOT SERIALNUMSHOW</td></row>
		<row><td>ReadyToInstall</td><td>SerialNumberText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>SetupTypeText1</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>SetupTypeText2</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>TargetFolderText1</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>TargetFolderText2</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>UserInformationText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>ReadyToInstall</td><td>UserNameText</td><td>Hide</td><td>VersionNT &gt;= "601" AND ISSupportPerUser AND NOT Installed</td></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>Default</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>Disable</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>Enable</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>Cancel</td><td>Disable</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>Cancel</td><td>Enable</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>CheckShowMsiLog</td><td>Show</td><td>MsiLogFileLocation</td></row>
		<row><td>SetupCompleteError</td><td>Finish</td><td>Default</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>FinishText1</td><td>Hide</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>FinishText1</td><td>Show</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>FinishText2</td><td>Hide</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>FinishText2</td><td>Show</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>RestContText1</td><td>Hide</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>RestContText1</td><td>Show</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>RestContText2</td><td>Hide</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>RestContText2</td><td>Show</td><td>UpdateStarted</td></row>
		<row><td>SetupCompleteError</td><td>ShowMsiLogText</td><td>Show</td><td>MsiLogFileLocation</td></row>
		<row><td>SetupCompleteSuccess</td><td>CheckBoxUpdates</td><td>Show</td><td>ISENABLEDWUSFINISHDIALOG And NOT Installed And ACTION="INSTALL"</td></row>
		<row><td>SetupCompleteSuccess</td><td>CheckForUpdatesText</td><td>Show</td><td>ISENABLEDWUSFINISHDIALOG And NOT Installed And ACTION="INSTALL"</td></row>
		<row><td>SetupCompleteSuccess</td><td>CheckLaunchProgram</td><td>Show</td><td>SHOWLAUNCHPROGRAM="-1" And PROGRAMFILETOLAUNCHATEND &lt;&gt; "" And NOT Installed And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>CheckLaunchReadme</td><td>Show</td><td>SHOWLAUNCHREADME="-1"  And READMEFILETOLAUNCHATEND &lt;&gt; "" And NOT Installed And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>CheckShowMsiLog</td><td>Show</td><td>MsiLogFileLocation And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>LaunchProgramText</td><td>Show</td><td>SHOWLAUNCHPROGRAM="-1" And PROGRAMFILETOLAUNCHATEND &lt;&gt; "" And NOT Installed And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>LaunchReadmeText</td><td>Show</td><td>SHOWLAUNCHREADME="-1"  And READMEFILETOLAUNCHATEND &lt;&gt; "" And NOT Installed And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>ShowMsiLogText</td><td>Show</td><td>MsiLogFileLocation And NOT ISENABLEDWUSFINISHDIALOG</td></row>
		<row><td>SetupCompleteSuccess</td><td>TextLine2</td><td>Show</td><td>ProgressType2="installed" And ((ACTION&lt;&gt;"INSTALL") OR (NOT ISENABLEDWUSFINISHDIALOG) OR (ISENABLEDWUSFINISHDIALOG And Installed))</td></row>
		<row><td>SetupCompleteSuccess</td><td>TextLine3</td><td>Show</td><td>ProgressType2="uninstalled" And ((ACTION&lt;&gt;"INSTALL") OR (NOT ISENABLEDWUSFINISHDIALOG) OR (ISENABLEDWUSFINISHDIALOG And Installed))</td></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine1</td><td>Show</td><td>ISENABLEDWUSFINISHDIALOG And NOT Installed And ACTION="INSTALL"</td></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine2</td><td>Show</td><td>ISENABLEDWUSFINISHDIALOG And NOT Installed And ACTION="INSTALL"</td></row>
		<row><td>SetupCompleteSuccess</td><td>UpdateTextLine3</td><td>Show</td><td>ISENABLEDWUSFINISHDIALOG And NOT Installed And ACTION="INSTALL"</td></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>Default</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>Disable</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>Enable</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>Cancel</td><td>Disable</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>Cancel</td><td>Enable</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>CheckShowMsiLog</td><td>Show</td><td>MsiLogFileLocation</td></row>
		<row><td>SetupInterrupted</td><td>Finish</td><td>Default</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>FinishText1</td><td>Hide</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>FinishText1</td><td>Show</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>FinishText2</td><td>Hide</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>FinishText2</td><td>Show</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>RestContText1</td><td>Hide</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>RestContText1</td><td>Show</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>RestContText2</td><td>Hide</td><td>NOT UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>RestContText2</td><td>Show</td><td>UpdateStarted</td></row>
		<row><td>SetupInterrupted</td><td>ShowMsiLogText</td><td>Show</td><td>MsiLogFileLocation</td></row>
		<row><td>SetupProgress</td><td>DlgDesc</td><td>Show</td><td>ProgressType2="installed"</td></row>
		<row><td>SetupProgress</td><td>DlgDesc2</td><td>Show</td><td>ProgressType2="uninstalled"</td></row>
		<row><td>SetupProgress</td><td>DlgText</td><td>Show</td><td>ProgressType3="installs"</td></row>
		<row><td>SetupProgress</td><td>DlgText2</td><td>Show</td><td>ProgressType3="uninstalls"</td></row>
		<row><td>SetupProgress</td><td>DlgTitle</td><td>Show</td><td>ProgressType1="Installing"</td></row>
		<row><td>SetupProgress</td><td>DlgTitle2</td><td>Show</td><td>ProgressType1="Uninstalling"</td></row>
		<row><td>SetupResume</td><td>PreselectedText</td><td>Hide</td><td>RESUME</td></row>
		<row><td>SetupResume</td><td>PreselectedText</td><td>Show</td><td>NOT RESUME</td></row>
		<row><td>SetupResume</td><td>ResumeText</td><td>Hide</td><td>NOT RESUME</td></row>
		<row><td>SetupResume</td><td>ResumeText</td><td>Show</td><td>RESUME</td></row>
	</table>

	<table name="ControlEvent">
		<col key="yes" def="s72">Dialog_</col>
		<col key="yes" def="s50">Control_</col>
		<col key="yes" def="s50">Event</col>
		<col key="yes" def="s255">Argument</col>
		<col key="yes" def="S255">Condition</col>
		<col def="I2">Ordering</col>
		<row><td>AdminChangeFolder</td><td>Cancel</td><td>EndDialog</td><td>Return</td><td>1</td><td>2</td></row>
		<row><td>AdminChangeFolder</td><td>Cancel</td><td>Reset</td><td>0</td><td>1</td><td>1</td></row>
		<row><td>AdminChangeFolder</td><td>NewFolder</td><td>DirectoryListNew</td><td>0</td><td>1</td><td>0</td></row>
		<row><td>AdminChangeFolder</td><td>OK</td><td>EndDialog</td><td>Return</td><td>1</td><td>0</td></row>
		<row><td>AdminChangeFolder</td><td>OK</td><td>SetTargetPath</td><td>TARGETDIR</td><td>1</td><td>1</td></row>
		<row><td>AdminChangeFolder</td><td>Up</td><td>DirectoryListUp</td><td>0</td><td>1</td><td>0</td></row>
		<row><td>AdminNetworkLocation</td><td>Back</td><td>NewDialog</td><td>AdminWelcome</td><td>1</td><td>0</td></row>
		<row><td>AdminNetworkLocation</td><td>Browse</td><td>SpawnDialog</td><td>AdminChangeFolder</td><td>1</td><td>0</td></row>
		<row><td>AdminNetworkLocation</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>AdminNetworkLocation</td><td>InstallNow</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>3</td></row>
		<row><td>AdminNetworkLocation</td><td>InstallNow</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>2</td></row>
		<row><td>AdminNetworkLocation</td><td>InstallNow</td><td>SetTargetPath</td><td>TARGETDIR</td><td>1</td><td>1</td></row>
		<row><td>AdminWelcome</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>AdminWelcome</td><td>Next</td><td>NewDialog</td><td>AdminNetworkLocation</td><td>1</td><td>0</td></row>
		<row><td>CancelSetup</td><td>No</td><td>EndDialog</td><td>Return</td><td>1</td><td>0</td></row>
		<row><td>CancelSetup</td><td>Yes</td><td>DoAction</td><td>CleanUp</td><td>ISSCRIPTRUNNING="1"</td><td>1</td></row>
		<row><td>CancelSetup</td><td>Yes</td><td>EndDialog</td><td>Exit</td><td>1</td><td>2</td></row>
		<row><td>CustomSetup</td><td>Back</td><td>NewDialog</td><td>MaintenanceType</td><td>Installed</td><td>0</td></row>
		<row><td>CustomSetup</td><td>Back</td><td>NewDialog</td><td>SetupType</td><td>NOT Installed</td><td>0</td></row>
		<row><td>CustomSetup</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>CustomSetup</td><td>ChangeFolder</td><td>SelectionBrowse</td><td>InstallChangeFolder</td><td>1</td><td>0</td></row>
		<row><td>CustomSetup</td><td>Details</td><td>SelectionBrowse</td><td>DiskSpaceRequirements</td><td>1</td><td>1</td></row>
		<row><td>CustomSetup</td><td>Help</td><td>SpawnDialog</td><td>CustomSetupTips</td><td>1</td><td>1</td></row>
		<row><td>CustomSetup</td><td>Next</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>0</td></row>
		<row><td>CustomSetup</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>0</td></row>
		<row><td>CustomSetup</td><td>Next</td><td>[_IsSetupTypeMin]</td><td>Custom</td><td>1</td><td>0</td></row>
		<row><td>CustomSetupTips</td><td>OK</td><td>EndDialog</td><td>Return</td><td>1</td><td>1</td></row>
		<row><td>CustomerInformation</td><td>Back</td><td>NewDialog</td><td>InstallWelcome</td><td>NOT Installed</td><td>1</td></row>
		<row><td>CustomerInformation</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>CustomerInformation</td><td>Next</td><td>EndDialog</td><td>Exit</td><td>(SERIALNUMVALRETRYLIMIT) And (SERIALNUMVALRETRYLIMIT&lt;0) And (SERIALNUMVALRETURN&lt;&gt;SERIALNUMVALSUCCESSRETVAL)</td><td>2</td></row>
		<row><td>CustomerInformation</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>(Not SERIALNUMVALRETURN) OR (SERIALNUMVALRETURN=SERIALNUMVALSUCCESSRETVAL)</td><td>3</td></row>
		<row><td>CustomerInformation</td><td>Next</td><td>[ALLUSERS]</td><td>1</td><td>ApplicationUsers = "AllUsers" And Privileged</td><td>1</td></row>
		<row><td>CustomerInformation</td><td>Next</td><td>[ALLUSERS]</td><td>{}</td><td>ApplicationUsers = "OnlyCurrentUser" And Privileged</td><td>2</td></row>
		<row><td>DatabaseFolder</td><td>Back</td><td>NewDialog</td><td>CustomerInformation</td><td>1</td><td>1</td></row>
		<row><td>DatabaseFolder</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>1</td></row>
		<row><td>DatabaseFolder</td><td>ChangeFolder</td><td>SpawnDialog</td><td>InstallChangeFolder</td><td>1</td><td>1</td></row>
		<row><td>DatabaseFolder</td><td>ChangeFolder</td><td>[_BrowseProperty]</td><td>DATABASEDIR</td><td>1</td><td>2</td></row>
		<row><td>DatabaseFolder</td><td>Next</td><td>NewDialog</td><td>SetupType</td><td>1</td><td>1</td></row>
		<row><td>DestinationFolder</td><td>Back</td><td>NewDialog</td><td>CustomerInformation</td><td>1</td><td>0</td></row>
		<row><td>DestinationFolder</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>1</td></row>
		<row><td>DestinationFolder</td><td>ChangeFolder</td><td>SpawnDialog</td><td>InstallChangeFolder</td><td>1</td><td>1</td></row>
		<row><td>DestinationFolder</td><td>ChangeFolder</td><td>[_BrowseProperty]</td><td>INSTALLDIR</td><td>1</td><td>2</td></row>
		<row><td>DestinationFolder</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>1</td><td>0</td></row>
		<row><td>DiskSpaceRequirements</td><td>OK</td><td>EndDialog</td><td>Return</td><td>1</td><td>0</td></row>
		<row><td>FilesInUse</td><td>Exit</td><td>EndDialog</td><td>Exit</td><td>1</td><td>0</td></row>
		<row><td>FilesInUse</td><td>Ignore</td><td>EndDialog</td><td>Ignore</td><td>1</td><td>0</td></row>
		<row><td>FilesInUse</td><td>Retry</td><td>EndDialog</td><td>Retry</td><td>1</td><td>0</td></row>
		<row><td>InstallChangeFolder</td><td>Cancel</td><td>EndDialog</td><td>Return</td><td>1</td><td>2</td></row>
		<row><td>InstallChangeFolder</td><td>Cancel</td><td>Reset</td><td>0</td><td>1</td><td>1</td></row>
		<row><td>InstallChangeFolder</td><td>NewFolder</td><td>DirectoryListNew</td><td>0</td><td>1</td><td>0</td></row>
		<row><td>InstallChangeFolder</td><td>OK</td><td>EndDialog</td><td>Return</td><td>1</td><td>3</td></row>
		<row><td>InstallChangeFolder</td><td>OK</td><td>SetTargetPath</td><td>[_BrowseProperty]</td><td>1</td><td>2</td></row>
		<row><td>InstallChangeFolder</td><td>Up</td><td>DirectoryListUp</td><td>0</td><td>1</td><td>0</td></row>
		<row><td>InstallWelcome</td><td>Back</td><td>NewDialog</td><td>SplashBitmap</td><td>NOT Installed</td><td>0</td></row>
		<row><td>InstallWelcome</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>InstallWelcome</td><td>Next</td><td>EndDialog</td><td>Return</td><td>OutOfDiskSpace &lt;&gt; 1</td><td>3</td></row>
		<row><td>InstallWelcome</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>0</td><td>4</td></row>
		<row><td>LicenseAgreement</td><td>Back</td><td>NewDialog</td><td>InstallWelcome</td><td>1</td><td>0</td></row>
		<row><td>LicenseAgreement</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>LicenseAgreement</td><td>ISPrintButton</td><td>DoAction</td><td>ISPrint</td><td>1</td><td>0</td></row>
		<row><td>LicenseAgreement</td><td>Next</td><td>NewDialog</td><td>CustomerInformation</td><td>AgreeToLicense = "Yes"</td><td>0</td></row>
		<row><td>MaintenanceType</td><td>Back</td><td>NewDialog</td><td>MaintenanceWelcome</td><td>1</td><td>0</td></row>
		<row><td>MaintenanceType</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>NewDialog</td><td>CustomSetup</td><td>_IsMaintenance = "Change"</td><td>12</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>_IsMaintenance = "Reinstall"</td><td>13</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>NewDialog</td><td>ReadyToRemove</td><td>_IsMaintenance = "Remove"</td><td>11</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>Reinstall</td><td>ALL</td><td>_IsMaintenance = "Reinstall"</td><td>10</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>ReinstallMode</td><td>[ReinstallModeText]</td><td>_IsMaintenance = "Reinstall"</td><td>9</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType0]</td><td>Modify</td><td>_IsMaintenance = "Change"</td><td>2</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType0]</td><td>Repair</td><td>_IsMaintenance = "Reinstall"</td><td>1</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType1]</td><td>Modifying</td><td>_IsMaintenance = "Change"</td><td>3</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType1]</td><td>Repairing</td><td>_IsMaintenance = "Reinstall"</td><td>4</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType2]</td><td>modified</td><td>_IsMaintenance = "Change"</td><td>6</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType2]</td><td>repairs</td><td>_IsMaintenance = "Reinstall"</td><td>5</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType3]</td><td>modifies</td><td>_IsMaintenance = "Change"</td><td>7</td></row>
		<row><td>MaintenanceType</td><td>Next</td><td>[ProgressType3]</td><td>repairs</td><td>_IsMaintenance = "Reinstall"</td><td>8</td></row>
		<row><td>MaintenanceWelcome</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>MaintenanceWelcome</td><td>Next</td><td>NewDialog</td><td>MaintenanceType</td><td>1</td><td>0</td></row>
		<row><td>MsiRMFilesInUse</td><td>Cancel</td><td>EndDialog</td><td>Exit</td><td>1</td><td>1</td></row>
		<row><td>MsiRMFilesInUse</td><td>OK</td><td>EndDialog</td><td>Return</td><td>1</td><td>1</td></row>
		<row><td>MsiRMFilesInUse</td><td>OK</td><td>RMShutdownAndRestart</td><td>0</td><td>RestartManagerOption="CloseRestart"</td><td>2</td></row>
		<row><td>OutOfSpace</td><td>Resume</td><td>NewDialog</td><td>AdminNetworkLocation</td><td>ACTION = "ADMIN"</td><td>0</td></row>
		<row><td>OutOfSpace</td><td>Resume</td><td>NewDialog</td><td>DestinationFolder</td><td>ACTION &lt;&gt; "ADMIN"</td><td>0</td></row>
		<row><td>PatchWelcome</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>1</td></row>
		<row><td>PatchWelcome</td><td>Next</td><td>EndDialog</td><td>Return</td><td>1</td><td>3</td></row>
		<row><td>PatchWelcome</td><td>Next</td><td>Reinstall</td><td>ALL</td><td>PATCH And REINSTALL=""</td><td>1</td></row>
		<row><td>PatchWelcome</td><td>Next</td><td>ReinstallMode</td><td>omus</td><td>PATCH And REINSTALLMODE=""</td><td>2</td></row>
		<row><td>ReadmeInformation</td><td>Back</td><td>NewDialog</td><td>LicenseAgreement</td><td>1</td><td>1</td></row>
		<row><td>ReadmeInformation</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>1</td></row>
		<row><td>ReadmeInformation</td><td>Next</td><td>NewDialog</td><td>CustomerInformation</td><td>1</td><td>1</td></row>
		<row><td>ReadyToInstall</td><td>Back</td><td>NewDialog</td><td>CustomSetup</td><td>Installed OR _IsSetupTypeMin = "Custom"</td><td>2</td></row>
		<row><td>ReadyToInstall</td><td>Back</td><td>NewDialog</td><td>InstallWelcome</td><td>NOT Installed</td><td>1</td></row>
		<row><td>ReadyToInstall</td><td>Back</td><td>NewDialog</td><td>MaintenanceType</td><td>Installed AND _IsMaintenance = "Reinstall"</td><td>3</td></row>
		<row><td>ReadyToInstall</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>[ProgressType1]</td><td>Installing</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>[ProgressType2]</td><td>installed</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallNow</td><td>[ProgressType3]</td><td>installs</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>[ALLUSERS]</td><td>1</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>[MSIINSTALLPERUSER]</td><td>{}</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>[ProgressType1]</td><td>Installing</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>[ProgressType2]</td><td>installed</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerMachine</td><td>[ProgressType3]</td><td>installs</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>[ALLUSERS]</td><td>2</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>[MSIINSTALLPERUSER]</td><td>1</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>[ProgressType1]</td><td>Installing</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>[ProgressType2]</td><td>installed</td><td>1</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>InstallPerUser</td><td>[ProgressType3]</td><td>installs</td><td>1</td><td>0</td></row>
		<row><td>ReadyToRemove</td><td>Back</td><td>NewDialog</td><td>MaintenanceType</td><td>1</td><td>0</td></row>
		<row><td>ReadyToRemove</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>2</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>2</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>Remove</td><td>ALL</td><td>1</td><td>1</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>[ProgressType1]</td><td>Uninstalling</td><td>1</td><td>0</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>[ProgressType2]</td><td>uninstalled</td><td>1</td><td>0</td></row>
		<row><td>ReadyToRemove</td><td>RemoveNow</td><td>[ProgressType3]</td><td>uninstalls</td><td>1</td><td>0</td></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>EndDialog</td><td>Return</td><td>1</td><td>2</td></row>
		<row><td>SetupCompleteError</td><td>Back</td><td>[Suspend]</td><td>{}</td><td>1</td><td>1</td></row>
		<row><td>SetupCompleteError</td><td>Cancel</td><td>EndDialog</td><td>Return</td><td>1</td><td>2</td></row>
		<row><td>SetupCompleteError</td><td>Cancel</td><td>[Suspend]</td><td>1</td><td>1</td><td>1</td></row>
		<row><td>SetupCompleteError</td><td>Finish</td><td>DoAction</td><td>CleanUp</td><td>ISSCRIPTRUNNING="1"</td><td>1</td></row>
		<row><td>SetupCompleteError</td><td>Finish</td><td>DoAction</td><td>ShowMsiLog</td><td>MsiLogFileLocation And (ISSHOWMSILOG="1")</td><td>3</td></row>
		<row><td>SetupCompleteError</td><td>Finish</td><td>EndDialog</td><td>Exit</td><td>1</td><td>2</td></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>DoAction</td><td>CleanUp</td><td>ISSCRIPTRUNNING="1"</td><td>55</td></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>DoAction</td><td>Restart</td><td>Not Installed</td><td>5</td></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>DoAction</td><td>ShowMsiLog</td><td>MsiLogFileLocation And (ISSHOWMSILOG="1") And NOT ISENABLEDWUSFINISHDIALOG</td><td>60</td></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>EndDialog</td><td>Exit</td><td>1</td><td>56</td></row>
		<row><td>SetupCompleteSuccess</td><td>OK</td><td>[Restart]</td><td>Restart</td><td>Not Installed</td><td>6</td></row>
		<row><td>SetupError</td><td>A</td><td>EndDialog</td><td>ErrorAbort</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>C</td><td>EndDialog</td><td>ErrorCancel</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>I</td><td>EndDialog</td><td>ErrorIgnore</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>N</td><td>EndDialog</td><td>ErrorNo</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>O</td><td>EndDialog</td><td>ErrorOk</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>R</td><td>EndDialog</td><td>ErrorRetry</td><td>1</td><td>0</td></row>
		<row><td>SetupError</td><td>Y</td><td>EndDialog</td><td>ErrorYes</td><td>1</td><td>0</td></row>
		<row><td>SetupInitialization</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>EndDialog</td><td>Exit</td><td>1</td><td>2</td></row>
		<row><td>SetupInterrupted</td><td>Back</td><td>[Suspend]</td><td>{}</td><td>1</td><td>1</td></row>
		<row><td>SetupInterrupted</td><td>Cancel</td><td>EndDialog</td><td>Exit</td><td>1</td><td>2</td></row>
		<row><td>SetupInterrupted</td><td>Cancel</td><td>[Suspend]</td><td>1</td><td>1</td><td>1</td></row>
		<row><td>SetupInterrupted</td><td>Finish</td><td>DoAction</td><td>CleanUp</td><td>ISSCRIPTRUNNING="1"</td><td>1</td></row>
		<row><td>SetupInterrupted</td><td>Finish</td><td>DoAction</td><td>ShowMsiLog</td><td>MsiLogFileLocation And (ISSHOWMSILOG="1")</td><td>3</td></row>
		<row><td>SetupInterrupted</td><td>Finish</td><td>EndDialog</td><td>Exit</td><td>1</td><td>2</td></row>
		<row><td>SetupProgress</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>SetupResume</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>SetupResume</td><td>Next</td><td>EndDialog</td><td>Return</td><td>OutOfNoRbDiskSpace &lt;&gt; 1</td><td>0</td></row>
		<row><td>SetupResume</td><td>Next</td><td>NewDialog</td><td>OutOfSpace</td><td>OutOfNoRbDiskSpace = 1</td><td>0</td></row>
		<row><td>SetupType</td><td>Back</td><td>NewDialog</td><td>CustomerInformation</td><td>1</td><td>1</td></row>
		<row><td>SetupType</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>NewDialog</td><td>CustomSetup</td><td>_IsSetupTypeMin = "Custom"</td><td>2</td></row>
		<row><td>SetupType</td><td>Next</td><td>NewDialog</td><td>ReadyToInstall</td><td>_IsSetupTypeMin &lt;&gt; "Custom"</td><td>1</td></row>
		<row><td>SetupType</td><td>Next</td><td>SetInstallLevel</td><td>100</td><td>_IsSetupTypeMin="Minimal"</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>SetInstallLevel</td><td>200</td><td>_IsSetupTypeMin="Typical"</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>SetInstallLevel</td><td>300</td><td>_IsSetupTypeMin="Custom"</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>[ISRUNSETUPTYPEADDLOCALEVENT]</td><td>1</td><td>1</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>[SelectedSetupType]</td><td>[DisplayNameCustom]</td><td>_IsSetupTypeMin = "Custom"</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>[SelectedSetupType]</td><td>[DisplayNameMinimal]</td><td>_IsSetupTypeMin = "Minimal"</td><td>0</td></row>
		<row><td>SetupType</td><td>Next</td><td>[SelectedSetupType]</td><td>[DisplayNameTypical]</td><td>_IsSetupTypeMin = "Typical"</td><td>0</td></row>
		<row><td>SplashBitmap</td><td>Cancel</td><td>SpawnDialog</td><td>CancelSetup</td><td>1</td><td>0</td></row>
		<row><td>SplashBitmap</td><td>Next</td><td>NewDialog</td><td>InstallWelcome</td><td>1</td><td>0</td></row>
	</table>

	<table name="CreateFolder">
		<col key="yes" def="s72">Directory_</col>
		<col key="yes" def="s72">Component_</col>
		<row><td>INSTALLDIR</td><td>ISX_DEFAULTCOMPONENT</td></row>
		<row><td>INSTALLDIR</td><td>ISX_DEFAULTCOMPONENT1</td></row>
		<row><td>INSTALLDIR</td><td>ISX_DEFAULTCOMPONENT2</td></row>
		<row><td>INSTALLDIR</td><td>ISX_DEFAULTCOMPONENT3</td></row>
		<row><td>INSTALLDIR</td><td>ISX_DEFAULTCOMPONENT4</td></row>
		<row><td>INSTALLDIR</td><td>TeamViewer_Setup.exe</td></row>
		<row><td>INSTALLDIR</td><td>Win10_RunOnce.exe</td></row>
		<row><td>INSTALLDIR</td><td>Win10_Startup.exe1</td></row>
		<row><td>INSTALLDIR</td><td>WinSCP.exe</td></row>
		<row><td>INSTALLDIR</td><td>WinpkFilter.exe</td></row>
		<row><td>INSTALLDIR</td><td>ndisapi.dll</td></row>
		<row><td>INSTALLDIR</td><td>putty.exe2</td></row>
		<row><td>INSTALLDIR</td><td>rx_net_bridge.exe</td></row>
		<row><td>MOUVENT1</td><td>ISX_DEFAULTCOMPONENT</td></row>
		<row><td>MOUVENT1</td><td>TeamViewer_Setup.exe</td></row>
		<row><td>MOUVENT1</td><td>Win10_RunOnce.exe</td></row>
		<row><td>MOUVENT1</td><td>Win10_Startup.exe1</td></row>
		<row><td>MOUVENT1</td><td>WinpkFilter.exe</td></row>
		<row><td>MOUVENT1</td><td>ndisapi.dll</td></row>
		<row><td>MOUVENT1</td><td>rx_net_bridge.exe</td></row>
		<row><td>PUTTY</td><td>ISX_DEFAULTCOMPONENT3</td></row>
		<row><td>PUTTY</td><td>putty.exe2</td></row>
		<row><td>WINSCP</td><td>ISX_DEFAULTCOMPONENT4</td></row>
		<row><td>WINSCP</td><td>WinSCP.exe</td></row>
	</table>

	<table name="CustomAction">
		<col key="yes" def="s72">Action</col>
		<col def="i2">Type</col>
		<col def="S64">Source</col>
		<col def="S0">Target</col>
		<col def="I4">ExtendedType</col>
		<col def="S255">ISComments</col>
		<row><td>ISPreventDowngrade</td><td>19</td><td/><td>[IS_PREVENT_DOWNGRADE_EXIT]</td><td/><td/></row>
		<row><td>ISPrint</td><td>1</td><td>SetAllUsers.dll</td><td>PrintScrollableText</td><td/><td>Prints the contents of a ScrollableText control on a dialog.</td></row>
		<row><td>ISRunSetupTypeAddLocalEvent</td><td>1</td><td>ISExpHlp.dll</td><td>RunSetupTypeAddLocalEvent</td><td/><td>Run the AddLocal events associated with the Next button on the Setup Type dialog.</td></row>
		<row><td>ISSelfRegisterCosting</td><td>1</td><td>ISSELFREG.DLL</td><td>ISSelfRegisterCosting</td><td/><td/></row>
		<row><td>ISSelfRegisterFiles</td><td>3073</td><td>ISSELFREG.DLL</td><td>ISSelfRegisterFiles</td><td/><td/></row>
		<row><td>ISSelfRegisterFinalize</td><td>1</td><td>ISSELFREG.DLL</td><td>ISSelfRegisterFinalize</td><td/><td/></row>
		<row><td>ISSetAllUsers</td><td>257</td><td>SetAllUsers.dll</td><td>SetAllUsers</td><td/><td/></row>
		<row><td>ISUnSelfRegisterFiles</td><td>3073</td><td>ISSELFREG.DLL</td><td>ISUnSelfRegisterFiles</td><td/><td/></row>
		<row><td>Restart</td><td>98</td><td>SystemFolder</td><td>[SystemFolder]Shutdown /r /t 0</td><td/><td/></row>
		<row><td>SetARPINSTALLLOCATION</td><td>51</td><td>ARPINSTALLLOCATION</td><td>[INSTALLDIR]</td><td/><td/></row>
		<row><td>SetAllUsersProfileNT</td><td>51</td><td>ALLUSERSPROFILE</td><td>[%SystemRoot]\Profiles\All Users</td><td/><td/></row>
		<row><td>ShowMsiLog</td><td>226</td><td>SystemFolder</td><td>[SystemFolder]notepad.exe "[MsiLogFileLocation]"</td><td/><td>Shows Property-driven MSI Log</td></row>
		<row><td>setAllUsersProfile2K</td><td>51</td><td>ALLUSERSPROFILE</td><td>[%ALLUSERSPROFILE]</td><td/><td/></row>
		<row><td>setUserProfileNT</td><td>51</td><td>USERPROFILE</td><td>[%USERPROFILE]</td><td/><td/></row>
	</table>

	<table name="Dialog">
		<col key="yes" def="s72">Dialog</col>
		<col def="i2">HCentering</col>
		<col def="i2">VCentering</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
		<col def="I4">Attributes</col>
		<col def="L128">Title</col>
		<col def="s50">Control_First</col>
		<col def="S50">Control_Default</col>
		<col def="S50">Control_Cancel</col>
		<col def="S255">ISComments</col>
		<col def="S72">TextStyle_</col>
		<col def="I4">ISWindowStyle</col>
		<col def="I4">ISResourceId</col>
		<row><td>AdminChangeFolder</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Tail</td><td>OK</td><td>Cancel</td><td>Install Point Browse</td><td/><td>0</td><td/></row>
		<row><td>AdminNetworkLocation</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>InstallNow</td><td>InstallNow</td><td>Cancel</td><td>Network Location</td><td/><td>0</td><td/></row>
		<row><td>AdminWelcome</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Administration Welcome</td><td/><td>0</td><td/></row>
		<row><td>CancelSetup</td><td>50</td><td>50</td><td>260</td><td>85</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>No</td><td>No</td><td>No</td><td>Cancel</td><td/><td>0</td><td/></row>
		<row><td>CustomSetup</td><td>50</td><td>50</td><td>374</td><td>266</td><td>35</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Tree</td><td>Next</td><td>Cancel</td><td>Custom Selection</td><td/><td>0</td><td/></row>
		<row><td>CustomSetupTips</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>OK</td><td>OK</td><td>OK</td><td>Custom Setup Tips</td><td/><td>0</td><td/></row>
		<row><td>CustomerInformation</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>NameEdit</td><td>Next</td><td>Cancel</td><td>Identification</td><td/><td>0</td><td/></row>
		<row><td>DatabaseFolder</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Database Folder</td><td/><td>0</td><td/></row>
		<row><td>DestinationFolder</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Destination Folder</td><td/><td>0</td><td/></row>
		<row><td>DiskSpaceRequirements</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>OK</td><td>OK</td><td>OK</td><td>Feature Details</td><td/><td>0</td><td/></row>
		<row><td>FilesInUse</td><td>50</td><td>50</td><td>374</td><td>266</td><td>19</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Retry</td><td>Retry</td><td>Exit</td><td>Files in Use</td><td/><td>0</td><td/></row>
		<row><td>InstallChangeFolder</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Tail</td><td>OK</td><td>Cancel</td><td>Browse</td><td/><td>0</td><td/></row>
		<row><td>InstallWelcome</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Welcome Panel</td><td/><td>0</td><td/></row>
		<row><td>LicenseAgreement</td><td>50</td><td>50</td><td>374</td><td>266</td><td>2</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Agree</td><td>Next</td><td>Cancel</td><td>License Agreement</td><td/><td>0</td><td/></row>
		<row><td>MaintenanceType</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>RadioGroup</td><td>Next</td><td>Cancel</td><td>Change, Reinstall, Remove</td><td/><td>0</td><td/></row>
		<row><td>MaintenanceWelcome</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Maintenance Welcome</td><td/><td>0</td><td/></row>
		<row><td>MsiRMFilesInUse</td><td>50</td><td>50</td><td>374</td><td>266</td><td>19</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>OK</td><td>OK</td><td>Cancel</td><td>RestartManager Files in Use</td><td/><td>0</td><td/></row>
		<row><td>OutOfSpace</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Resume</td><td>Resume</td><td>Resume</td><td>Out Of Disk Space</td><td/><td>0</td><td/></row>
		<row><td>PatchWelcome</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS__IsPatchDlg_PatchWizard##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Patch Panel</td><td/><td>0</td><td/></row>
		<row><td>ReadmeInformation</td><td>50</td><td>50</td><td>374</td><td>266</td><td>7</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Readme Information</td><td/><td>0</td><td>0</td></row>
		<row><td>ReadyToInstall</td><td>50</td><td>50</td><td>374</td><td>266</td><td>35</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>InstallNow</td><td>InstallNow</td><td>Cancel</td><td>Ready to Install</td><td/><td>0</td><td/></row>
		<row><td>ReadyToRemove</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>RemoveNow</td><td>RemoveNow</td><td>Cancel</td><td>Verify Remove</td><td/><td>0</td><td/></row>
		<row><td>SetupCompleteError</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Finish</td><td>Finish</td><td>Finish</td><td>Fatal Error</td><td/><td>0</td><td/></row>
		<row><td>SetupCompleteSuccess</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>OK</td><td>OK</td><td>OK</td><td>Exit</td><td/><td>0</td><td/></row>
		<row><td>SetupError</td><td>50</td><td>50</td><td>270</td><td>110</td><td>65543</td><td>##IDS__IsErrorDlg_InstallerInfo##</td><td>ErrorText</td><td>O</td><td>C</td><td>Error</td><td/><td>0</td><td/></row>
		<row><td>SetupInitialization</td><td>50</td><td>50</td><td>374</td><td>266</td><td>5</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Cancel</td><td>Cancel</td><td>Cancel</td><td>Setup Initialization</td><td/><td>0</td><td/></row>
		<row><td>SetupInterrupted</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Finish</td><td>Finish</td><td>Finish</td><td>User Exit</td><td/><td>0</td><td/></row>
		<row><td>SetupProgress</td><td>50</td><td>50</td><td>374</td><td>266</td><td>5</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Cancel</td><td>Cancel</td><td>Cancel</td><td>Progress</td><td/><td>0</td><td/></row>
		<row><td>SetupResume</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Resume</td><td/><td>0</td><td/></row>
		<row><td>SetupType</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>RadioGroup</td><td>Next</td><td>Cancel</td><td>Setup Type</td><td/><td>0</td><td/></row>
		<row><td>SplashBitmap</td><td>50</td><td>50</td><td>374</td><td>266</td><td>3</td><td>##IDS_PRODUCTNAME_INSTALLSHIELD##</td><td>Next</td><td>Next</td><td>Cancel</td><td>Welcome Bitmap</td><td/><td>0</td><td/></row>
	</table>

	<table name="Directory">
		<col key="yes" def="s72">Directory</col>
		<col def="S72">Directory_Parent</col>
		<col def="l255">DefaultDir</col>
		<col def="S255">ISDescription</col>
		<col def="I4">ISAttributes</col>
		<col def="S255">ISFolderName</col>
		<row><td>ALLUSERSPROFILE</td><td>TARGETDIR</td><td>.:ALLUSE~1|All Users</td><td/><td>0</td><td/></row>
		<row><td>AdminToolsFolder</td><td>TARGETDIR</td><td>.:Admint~1|AdminTools</td><td/><td>0</td><td/></row>
		<row><td>AppDataFolder</td><td>TARGETDIR</td><td>.:APPLIC~1|Application Data</td><td/><td>0</td><td/></row>
		<row><td>CommonAppDataFolder</td><td>TARGETDIR</td><td>.:Common~1|CommonAppData</td><td/><td>0</td><td/></row>
		<row><td>CommonFiles64Folder</td><td>TARGETDIR</td><td>.:Common64</td><td/><td>0</td><td/></row>
		<row><td>CommonFilesFolder</td><td>TARGETDIR</td><td>.:Common</td><td/><td>0</td><td/></row>
		<row><td>DATABASEDIR</td><td>ISYourDataBaseDir</td><td>.</td><td/><td>0</td><td/></row>
		<row><td>DIRPROPERTY1</td><td>TARGETDIR</td><td>.</td><td/><td>0</td><td/></row>
		<row><td>DesktopFolder</td><td>TARGETDIR</td><td>.:Desktop</td><td/><td>3</td><td/></row>
		<row><td>FavoritesFolder</td><td>TARGETDIR</td><td>.:FAVORI~1|Favorites</td><td/><td>0</td><td/></row>
		<row><td>FontsFolder</td><td>TARGETDIR</td><td>.:Fonts</td><td/><td>0</td><td/></row>
		<row><td>GlobalAssemblyCache</td><td>TARGETDIR</td><td>.:Global~1|GlobalAssemblyCache</td><td/><td>0</td><td/></row>
		<row><td>INSTALLDIR</td><td>SURFACE_INSTALLER</td><td>.</td><td/><td>0</td><td/></row>
		<row><td>ISCommonFilesFolder</td><td>CommonFilesFolder</td><td>Instal~1|InstallShield</td><td/><td>0</td><td/></row>
		<row><td>ISMyCompanyDir</td><td>ProgramFilesFolder</td><td>MYCOMP~1|My Company Name</td><td/><td>0</td><td/></row>
		<row><td>ISMyProductDir</td><td>ISMyCompanyDir</td><td>MYPROD~1|My Product Name</td><td/><td>0</td><td/></row>
		<row><td>ISYourDataBaseDir</td><td>INSTALLDIR</td><td>Database</td><td/><td>0</td><td/></row>
		<row><td>LocalAppDataFolder</td><td>TARGETDIR</td><td>.:LocalA~1|LocalAppData</td><td/><td>0</td><td/></row>
		<row><td>MOUVENT</td><td>ProgramFilesFolder</td><td>Mouvent</td><td/><td>0</td><td/></row>
		<row><td>MOUVENT1</td><td>ProgramFilesFolder</td><td>Mouvent</td><td/><td>0</td><td/></row>
		<row><td>MY_PRODUCT_NAME</td><td>MOUVENT1</td><td>MYPROD~1|My Product Name</td><td/><td>0</td><td/></row>
		<row><td>MyPicturesFolder</td><td>TARGETDIR</td><td>.:MyPict~1|MyPictures</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY1</td><td>TARGETDIR</td><td>NEW_DIRECTORY1</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY10</td><td>TARGETDIR</td><td>NEW_DIRECTORY10</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY11</td><td>TARGETDIR</td><td>NEW_DIRECTORY11</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY2</td><td>TARGETDIR</td><td>NEW_DIRECTORY2</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY3</td><td>TARGETDIR</td><td>NEW_DIRECTORY3</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY4</td><td>TARGETDIR</td><td>NEW_DIRECTORY4</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY5</td><td>TARGETDIR</td><td>NEW_DIRECTORY5</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY6</td><td>TARGETDIR</td><td>NEW_DIRECTORY6</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY7</td><td>TARGETDIR</td><td>NEW_DIRECTORY7</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY8</td><td>TARGETDIR</td><td>NEW_DIRECTORY8</td><td/><td>0</td><td/></row>
		<row><td>NEW_DIRECTORY9</td><td>TARGETDIR</td><td>NEW_DIRECTORY9</td><td/><td>0</td><td/></row>
		<row><td>NEW_FOLDER_1</td><td>TARGETDIR</td><td>NEW_FOLDER_1</td><td/><td>0</td><td/></row>
		<row><td>NEW_FOLDER_11</td><td>TARGETDIR</td><td>NEW_FOLDER_11</td><td/><td>0</td><td/></row>
		<row><td>NetHoodFolder</td><td>TARGETDIR</td><td>.:NetHood</td><td/><td>0</td><td/></row>
		<row><td>PUTTY</td><td>ProgramFilesFolder</td><td>PuTTY</td><td/><td>0</td><td/></row>
		<row><td>PersonalFolder</td><td>TARGETDIR</td><td>.:Personal</td><td/><td>0</td><td/></row>
		<row><td>PrimaryVolumePath</td><td>TARGETDIR</td><td>.:Primar~1|PrimaryVolumePath</td><td/><td>0</td><td/></row>
		<row><td>PrintHoodFolder</td><td>TARGETDIR</td><td>.:PRINTH~1|PrintHood</td><td/><td>0</td><td/></row>
		<row><td>ProgramFiles64Folder</td><td>TARGETDIR</td><td>.:Prog64~1|Program Files 64</td><td/><td>0</td><td/></row>
		<row><td>ProgramFilesFolder</td><td>TARGETDIR</td><td>.:PROGRA~1|program files</td><td/><td>0</td><td/></row>
		<row><td>ProgramMenuFolder</td><td>TARGETDIR</td><td>.:Programs</td><td/><td>3</td><td/></row>
		<row><td>RADEX1</td><td>programFilesFolder</td><td>Radex</td><td/><td>0</td><td/></row>
		<row><td>RecentFolder</td><td>TARGETDIR</td><td>.:Recent</td><td/><td>0</td><td/></row>
		<row><td>SURFACE_INSTALL.EXE</td><td>RADEX1</td><td>SURFAC~1|Surface-Install.exe</td><td/><td>0</td><td/></row>
		<row><td>SURFACE_INSTALLER</td><td>MOUVENT1</td><td>WIN10-~1|Win10-RunOnce</td><td/><td>0</td><td/></row>
		<row><td>SYSTEM32</td><td>WindowsFolder</td><td>system32</td><td/><td>0</td><td/></row>
		<row><td>SendToFolder</td><td>TARGETDIR</td><td>.:SendTo</td><td/><td>3</td><td/></row>
		<row><td>StartMenuFolder</td><td>TARGETDIR</td><td>.:STARTM~1|Start Menu</td><td/><td>3</td><td/></row>
		<row><td>StartupFolder</td><td>TARGETDIR</td><td>.:StartUp</td><td/><td>3</td><td/></row>
		<row><td>System16Folder</td><td>TARGETDIR</td><td>.:System</td><td/><td>0</td><td/></row>
		<row><td>System64Folder</td><td>TARGETDIR</td><td>.:System64</td><td/><td>0</td><td/></row>
		<row><td>SystemFolder</td><td>TARGETDIR</td><td>.:System32</td><td/><td>0</td><td/></row>
		<row><td>TARGETDIR</td><td/><td>SourceDir</td><td/><td>0</td><td/></row>
		<row><td>TempFolder</td><td>TARGETDIR</td><td>.:Temp</td><td/><td>0</td><td/></row>
		<row><td>TemplateFolder</td><td>TARGETDIR</td><td>.:ShellNew</td><td/><td>0</td><td/></row>
		<row><td>USERPROFILE</td><td>TARGETDIR</td><td>.:USERPR~1|UserProfile</td><td/><td>0</td><td/></row>
		<row><td>WINSCP</td><td>ProgramFilesFolder</td><td>WinSCP</td><td/><td>0</td><td/></row>
		<row><td>WindowsFolder</td><td>TARGETDIR</td><td>.:Windows</td><td/><td>0</td><td/></row>
		<row><td>WindowsVolume</td><td>TARGETDIR</td><td>.:WinRoot</td><td/><td>0</td><td/></row>
		<row><td>programFilesFolder</td><td>TARGETDIR</td><td>.</td><td/><td>0</td><td/></row>
		<row><td>radexa_1_radex_ag</td><td>ProgramMenuFolder</td><td>MOUVEN~1|Mouvent AG</td><td/><td>1</td><td/></row>
		<row><td>surfac_1_surface_installer</td><td>radexa_1_radex_ag</td><td>SURFAC~1|Surface Installer</td><td/><td>1</td><td/></row>
	</table>

	<table name="DrLocator">
		<col key="yes" def="s72">Signature_</col>
		<col key="yes" def="S72">Parent</col>
		<col key="yes" def="S255">Path</col>
		<col def="I2">Depth</col>
	</table>

	<table name="DuplicateFile">
		<col key="yes" def="s72">FileKey</col>
		<col def="s72">Component_</col>
		<col def="s72">File_</col>
		<col def="L255">DestName</col>
		<col def="S72">DestFolder</col>
	</table>

	<table name="Environment">
		<col key="yes" def="s72">Environment</col>
		<col def="l255">Name</col>
		<col def="L255">Value</col>
		<col def="s72">Component_</col>
	</table>

	<table name="Error">
		<col key="yes" def="i2">Error</col>
		<col def="L255">Message</col>
		<row><td>0</td><td>##IDS_ERROR_0##</td></row>
		<row><td>1</td><td>##IDS_ERROR_1##</td></row>
		<row><td>10</td><td>##IDS_ERROR_8##</td></row>
		<row><td>11</td><td>##IDS_ERROR_9##</td></row>
		<row><td>1101</td><td>##IDS_ERROR_22##</td></row>
		<row><td>12</td><td>##IDS_ERROR_10##</td></row>
		<row><td>13</td><td>##IDS_ERROR_11##</td></row>
		<row><td>1301</td><td>##IDS_ERROR_23##</td></row>
		<row><td>1302</td><td>##IDS_ERROR_24##</td></row>
		<row><td>1303</td><td>##IDS_ERROR_25##</td></row>
		<row><td>1304</td><td>##IDS_ERROR_26##</td></row>
		<row><td>1305</td><td>##IDS_ERROR_27##</td></row>
		<row><td>1306</td><td>##IDS_ERROR_28##</td></row>
		<row><td>1307</td><td>##IDS_ERROR_29##</td></row>
		<row><td>1308</td><td>##IDS_ERROR_30##</td></row>
		<row><td>1309</td><td>##IDS_ERROR_31##</td></row>
		<row><td>1310</td><td>##IDS_ERROR_32##</td></row>
		<row><td>1311</td><td>##IDS_ERROR_33##</td></row>
		<row><td>1312</td><td>##IDS_ERROR_34##</td></row>
		<row><td>1313</td><td>##IDS_ERROR_35##</td></row>
		<row><td>1314</td><td>##IDS_ERROR_36##</td></row>
		<row><td>1315</td><td>##IDS_ERROR_37##</td></row>
		<row><td>1316</td><td>##IDS_ERROR_38##</td></row>
		<row><td>1317</td><td>##IDS_ERROR_39##</td></row>
		<row><td>1318</td><td>##IDS_ERROR_40##</td></row>
		<row><td>1319</td><td>##IDS_ERROR_41##</td></row>
		<row><td>1320</td><td>##IDS_ERROR_42##</td></row>
		<row><td>1321</td><td>##IDS_ERROR_43##</td></row>
		<row><td>1322</td><td>##IDS_ERROR_44##</td></row>
		<row><td>1323</td><td>##IDS_ERROR_45##</td></row>
		<row><td>1324</td><td>##IDS_ERROR_46##</td></row>
		<row><td>1325</td><td>##IDS_ERROR_47##</td></row>
		<row><td>1326</td><td>##IDS_ERROR_48##</td></row>
		<row><td>1327</td><td>##IDS_ERROR_49##</td></row>
		<row><td>1328</td><td>##IDS_ERROR_122##</td></row>
		<row><td>1329</td><td>##IDS_ERROR_1329##</td></row>
		<row><td>1330</td><td>##IDS_ERROR_1330##</td></row>
		<row><td>1331</td><td>##IDS_ERROR_1331##</td></row>
		<row><td>1332</td><td>##IDS_ERROR_1332##</td></row>
		<row><td>1333</td><td>##IDS_ERROR_1333##</td></row>
		<row><td>1334</td><td>##IDS_ERROR_1334##</td></row>
		<row><td>1335</td><td>##IDS_ERROR_1335##</td></row>
		<row><td>1336</td><td>##IDS_ERROR_1336##</td></row>
		<row><td>14</td><td>##IDS_ERROR_12##</td></row>
		<row><td>1401</td><td>##IDS_ERROR_50##</td></row>
		<row><td>1402</td><td>##IDS_ERROR_51##</td></row>
		<row><td>1403</td><td>##IDS_ERROR_52##</td></row>
		<row><td>1404</td><td>##IDS_ERROR_53##</td></row>
		<row><td>1405</td><td>##IDS_ERROR_54##</td></row>
		<row><td>1406</td><td>##IDS_ERROR_55##</td></row>
		<row><td>1407</td><td>##IDS_ERROR_56##</td></row>
		<row><td>1408</td><td>##IDS_ERROR_57##</td></row>
		<row><td>1409</td><td>##IDS_ERROR_58##</td></row>
		<row><td>1410</td><td>##IDS_ERROR_59##</td></row>
		<row><td>15</td><td>##IDS_ERROR_13##</td></row>
		<row><td>1500</td><td>##IDS_ERROR_60##</td></row>
		<row><td>1501</td><td>##IDS_ERROR_61##</td></row>
		<row><td>1502</td><td>##IDS_ERROR_62##</td></row>
		<row><td>1503</td><td>##IDS_ERROR_63##</td></row>
		<row><td>16</td><td>##IDS_ERROR_14##</td></row>
		<row><td>1601</td><td>##IDS_ERROR_64##</td></row>
		<row><td>1602</td><td>##IDS_ERROR_65##</td></row>
		<row><td>1603</td><td>##IDS_ERROR_66##</td></row>
		<row><td>1604</td><td>##IDS_ERROR_67##</td></row>
		<row><td>1605</td><td>##IDS_ERROR_68##</td></row>
		<row><td>1606</td><td>##IDS_ERROR_69##</td></row>
		<row><td>1607</td><td>##IDS_ERROR_70##</td></row>
		<row><td>1608</td><td>##IDS_ERROR_71##</td></row>
		<row><td>1609</td><td>##IDS_ERROR_1609##</td></row>
		<row><td>1651</td><td>##IDS_ERROR_1651##</td></row>
		<row><td>17</td><td>##IDS_ERROR_15##</td></row>
		<row><td>1701</td><td>##IDS_ERROR_72##</td></row>
		<row><td>1702</td><td>##IDS_ERROR_73##</td></row>
		<row><td>1703</td><td>##IDS_ERROR_74##</td></row>
		<row><td>1704</td><td>##IDS_ERROR_75##</td></row>
		<row><td>1705</td><td>##IDS_ERROR_76##</td></row>
		<row><td>1706</td><td>##IDS_ERROR_77##</td></row>
		<row><td>1707</td><td>##IDS_ERROR_78##</td></row>
		<row><td>1708</td><td>##IDS_ERROR_79##</td></row>
		<row><td>1709</td><td>##IDS_ERROR_80##</td></row>
		<row><td>1710</td><td>##IDS_ERROR_81##</td></row>
		<row><td>1711</td><td>##IDS_ERROR_82##</td></row>
		<row><td>1712</td><td>##IDS_ERROR_83##</td></row>
		<row><td>1713</td><td>##IDS_ERROR_123##</td></row>
		<row><td>1714</td><td>##IDS_ERROR_124##</td></row>
		<row><td>1715</td><td>##IDS_ERROR_1715##</td></row>
		<row><td>1716</td><td>##IDS_ERROR_1716##</td></row>
		<row><td>1717</td><td>##IDS_ERROR_1717##</td></row>
		<row><td>1718</td><td>##IDS_ERROR_1718##</td></row>
		<row><td>1719</td><td>##IDS_ERROR_1719##</td></row>
		<row><td>1720</td><td>##IDS_ERROR_1720##</td></row>
		<row><td>1721</td><td>##IDS_ERROR_1721##</td></row>
		<row><td>1722</td><td>##IDS_ERROR_1722##</td></row>
		<row><td>1723</td><td>##IDS_ERROR_1723##</td></row>
		<row><td>1724</td><td>##IDS_ERROR_1724##</td></row>
		<row><td>1725</td><td>##IDS_ERROR_1725##</td></row>
		<row><td>1726</td><td>##IDS_ERROR_1726##</td></row>
		<row><td>1727</td><td>##IDS_ERROR_1727##</td></row>
		<row><td>1728</td><td>##IDS_ERROR_1728##</td></row>
		<row><td>1729</td><td>##IDS_ERROR_1729##</td></row>
		<row><td>1730</td><td>##IDS_ERROR_1730##</td></row>
		<row><td>1731</td><td>##IDS_ERROR_1731##</td></row>
		<row><td>1732</td><td>##IDS_ERROR_1732##</td></row>
		<row><td>18</td><td>##IDS_ERROR_16##</td></row>
		<row><td>1801</td><td>##IDS_ERROR_84##</td></row>
		<row><td>1802</td><td>##IDS_ERROR_85##</td></row>
		<row><td>1803</td><td>##IDS_ERROR_86##</td></row>
		<row><td>1804</td><td>##IDS_ERROR_87##</td></row>
		<row><td>1805</td><td>##IDS_ERROR_88##</td></row>
		<row><td>1806</td><td>##IDS_ERROR_89##</td></row>
		<row><td>1807</td><td>##IDS_ERROR_90##</td></row>
		<row><td>19</td><td>##IDS_ERROR_17##</td></row>
		<row><td>1901</td><td>##IDS_ERROR_91##</td></row>
		<row><td>1902</td><td>##IDS_ERROR_92##</td></row>
		<row><td>1903</td><td>##IDS_ERROR_93##</td></row>
		<row><td>1904</td><td>##IDS_ERROR_94##</td></row>
		<row><td>1905</td><td>##IDS_ERROR_95##</td></row>
		<row><td>1906</td><td>##IDS_ERROR_96##</td></row>
		<row><td>1907</td><td>##IDS_ERROR_97##</td></row>
		<row><td>1908</td><td>##IDS_ERROR_98##</td></row>
		<row><td>1909</td><td>##IDS_ERROR_99##</td></row>
		<row><td>1910</td><td>##IDS_ERROR_100##</td></row>
		<row><td>1911</td><td>##IDS_ERROR_101##</td></row>
		<row><td>1912</td><td>##IDS_ERROR_102##</td></row>
		<row><td>1913</td><td>##IDS_ERROR_103##</td></row>
		<row><td>1914</td><td>##IDS_ERROR_104##</td></row>
		<row><td>1915</td><td>##IDS_ERROR_105##</td></row>
		<row><td>1916</td><td>##IDS_ERROR_106##</td></row>
		<row><td>1917</td><td>##IDS_ERROR_107##</td></row>
		<row><td>1918</td><td>##IDS_ERROR_108##</td></row>
		<row><td>1919</td><td>##IDS_ERROR_109##</td></row>
		<row><td>1920</td><td>##IDS_ERROR_110##</td></row>
		<row><td>1921</td><td>##IDS_ERROR_111##</td></row>
		<row><td>1922</td><td>##IDS_ERROR_112##</td></row>
		<row><td>1923</td><td>##IDS_ERROR_113##</td></row>
		<row><td>1924</td><td>##IDS_ERROR_114##</td></row>
		<row><td>1925</td><td>##IDS_ERROR_115##</td></row>
		<row><td>1926</td><td>##IDS_ERROR_116##</td></row>
		<row><td>1927</td><td>##IDS_ERROR_117##</td></row>
		<row><td>1928</td><td>##IDS_ERROR_118##</td></row>
		<row><td>1929</td><td>##IDS_ERROR_119##</td></row>
		<row><td>1930</td><td>##IDS_ERROR_125##</td></row>
		<row><td>1931</td><td>##IDS_ERROR_126##</td></row>
		<row><td>1932</td><td>##IDS_ERROR_127##</td></row>
		<row><td>1933</td><td>##IDS_ERROR_128##</td></row>
		<row><td>1934</td><td>##IDS_ERROR_129##</td></row>
		<row><td>1935</td><td>##IDS_ERROR_1935##</td></row>
		<row><td>1936</td><td>##IDS_ERROR_1936##</td></row>
		<row><td>1937</td><td>##IDS_ERROR_1937##</td></row>
		<row><td>1938</td><td>##IDS_ERROR_1938##</td></row>
		<row><td>2</td><td>##IDS_ERROR_2##</td></row>
		<row><td>20</td><td>##IDS_ERROR_18##</td></row>
		<row><td>21</td><td>##IDS_ERROR_19##</td></row>
		<row><td>2101</td><td>##IDS_ERROR_2101##</td></row>
		<row><td>2102</td><td>##IDS_ERROR_2102##</td></row>
		<row><td>2103</td><td>##IDS_ERROR_2103##</td></row>
		<row><td>2104</td><td>##IDS_ERROR_2104##</td></row>
		<row><td>2105</td><td>##IDS_ERROR_2105##</td></row>
		<row><td>2106</td><td>##IDS_ERROR_2106##</td></row>
		<row><td>2107</td><td>##IDS_ERROR_2107##</td></row>
		<row><td>2108</td><td>##IDS_ERROR_2108##</td></row>
		<row><td>2109</td><td>##IDS_ERROR_2109##</td></row>
		<row><td>2110</td><td>##IDS_ERROR_2110##</td></row>
		<row><td>2111</td><td>##IDS_ERROR_2111##</td></row>
		<row><td>2112</td><td>##IDS_ERROR_2112##</td></row>
		<row><td>2113</td><td>##IDS_ERROR_2113##</td></row>
		<row><td>22</td><td>##IDS_ERROR_120##</td></row>
		<row><td>2200</td><td>##IDS_ERROR_2200##</td></row>
		<row><td>2201</td><td>##IDS_ERROR_2201##</td></row>
		<row><td>2202</td><td>##IDS_ERROR_2202##</td></row>
		<row><td>2203</td><td>##IDS_ERROR_2203##</td></row>
		<row><td>2204</td><td>##IDS_ERROR_2204##</td></row>
		<row><td>2205</td><td>##IDS_ERROR_2205##</td></row>
		<row><td>2206</td><td>##IDS_ERROR_2206##</td></row>
		<row><td>2207</td><td>##IDS_ERROR_2207##</td></row>
		<row><td>2208</td><td>##IDS_ERROR_2208##</td></row>
		<row><td>2209</td><td>##IDS_ERROR_2209##</td></row>
		<row><td>2210</td><td>##IDS_ERROR_2210##</td></row>
		<row><td>2211</td><td>##IDS_ERROR_2211##</td></row>
		<row><td>2212</td><td>##IDS_ERROR_2212##</td></row>
		<row><td>2213</td><td>##IDS_ERROR_2213##</td></row>
		<row><td>2214</td><td>##IDS_ERROR_2214##</td></row>
		<row><td>2215</td><td>##IDS_ERROR_2215##</td></row>
		<row><td>2216</td><td>##IDS_ERROR_2216##</td></row>
		<row><td>2217</td><td>##IDS_ERROR_2217##</td></row>
		<row><td>2218</td><td>##IDS_ERROR_2218##</td></row>
		<row><td>2219</td><td>##IDS_ERROR_2219##</td></row>
		<row><td>2220</td><td>##IDS_ERROR_2220##</td></row>
		<row><td>2221</td><td>##IDS_ERROR_2221##</td></row>
		<row><td>2222</td><td>##IDS_ERROR_2222##</td></row>
		<row><td>2223</td><td>##IDS_ERROR_2223##</td></row>
		<row><td>2224</td><td>##IDS_ERROR_2224##</td></row>
		<row><td>2225</td><td>##IDS_ERROR_2225##</td></row>
		<row><td>2226</td><td>##IDS_ERROR_2226##</td></row>
		<row><td>2227</td><td>##IDS_ERROR_2227##</td></row>
		<row><td>2228</td><td>##IDS_ERROR_2228##</td></row>
		<row><td>2229</td><td>##IDS_ERROR_2229##</td></row>
		<row><td>2230</td><td>##IDS_ERROR_2230##</td></row>
		<row><td>2231</td><td>##IDS_ERROR_2231##</td></row>
		<row><td>2232</td><td>##IDS_ERROR_2232##</td></row>
		<row><td>2233</td><td>##IDS_ERROR_2233##</td></row>
		<row><td>2234</td><td>##IDS_ERROR_2234##</td></row>
		<row><td>2235</td><td>##IDS_ERROR_2235##</td></row>
		<row><td>2236</td><td>##IDS_ERROR_2236##</td></row>
		<row><td>2237</td><td>##IDS_ERROR_2237##</td></row>
		<row><td>2238</td><td>##IDS_ERROR_2238##</td></row>
		<row><td>2239</td><td>##IDS_ERROR_2239##</td></row>
		<row><td>2240</td><td>##IDS_ERROR_2240##</td></row>
		<row><td>2241</td><td>##IDS_ERROR_2241##</td></row>
		<row><td>2242</td><td>##IDS_ERROR_2242##</td></row>
		<row><td>2243</td><td>##IDS_ERROR_2243##</td></row>
		<row><td>2244</td><td>##IDS_ERROR_2244##</td></row>
		<row><td>2245</td><td>##IDS_ERROR_2245##</td></row>
		<row><td>2246</td><td>##IDS_ERROR_2246##</td></row>
		<row><td>2247</td><td>##IDS_ERROR_2247##</td></row>
		<row><td>2248</td><td>##IDS_ERROR_2248##</td></row>
		<row><td>2249</td><td>##IDS_ERROR_2249##</td></row>
		<row><td>2250</td><td>##IDS_ERROR_2250##</td></row>
		<row><td>2251</td><td>##IDS_ERROR_2251##</td></row>
		<row><td>2252</td><td>##IDS_ERROR_2252##</td></row>
		<row><td>2253</td><td>##IDS_ERROR_2253##</td></row>
		<row><td>2254</td><td>##IDS_ERROR_2254##</td></row>
		<row><td>2255</td><td>##IDS_ERROR_2255##</td></row>
		<row><td>2256</td><td>##IDS_ERROR_2256##</td></row>
		<row><td>2257</td><td>##IDS_ERROR_2257##</td></row>
		<row><td>2258</td><td>##IDS_ERROR_2258##</td></row>
		<row><td>2259</td><td>##IDS_ERROR_2259##</td></row>
		<row><td>2260</td><td>##IDS_ERROR_2260##</td></row>
		<row><td>2261</td><td>##IDS_ERROR_2261##</td></row>
		<row><td>2262</td><td>##IDS_ERROR_2262##</td></row>
		<row><td>2263</td><td>##IDS_ERROR_2263##</td></row>
		<row><td>2264</td><td>##IDS_ERROR_2264##</td></row>
		<row><td>2265</td><td>##IDS_ERROR_2265##</td></row>
		<row><td>2266</td><td>##IDS_ERROR_2266##</td></row>
		<row><td>2267</td><td>##IDS_ERROR_2267##</td></row>
		<row><td>2268</td><td>##IDS_ERROR_2268##</td></row>
		<row><td>2269</td><td>##IDS_ERROR_2269##</td></row>
		<row><td>2270</td><td>##IDS_ERROR_2270##</td></row>
		<row><td>2271</td><td>##IDS_ERROR_2271##</td></row>
		<row><td>2272</td><td>##IDS_ERROR_2272##</td></row>
		<row><td>2273</td><td>##IDS_ERROR_2273##</td></row>
		<row><td>2274</td><td>##IDS_ERROR_2274##</td></row>
		<row><td>2275</td><td>##IDS_ERROR_2275##</td></row>
		<row><td>2276</td><td>##IDS_ERROR_2276##</td></row>
		<row><td>2277</td><td>##IDS_ERROR_2277##</td></row>
		<row><td>2278</td><td>##IDS_ERROR_2278##</td></row>
		<row><td>2279</td><td>##IDS_ERROR_2279##</td></row>
		<row><td>2280</td><td>##IDS_ERROR_2280##</td></row>
		<row><td>2281</td><td>##IDS_ERROR_2281##</td></row>
		<row><td>2282</td><td>##IDS_ERROR_2282##</td></row>
		<row><td>23</td><td>##IDS_ERROR_121##</td></row>
		<row><td>2302</td><td>##IDS_ERROR_2302##</td></row>
		<row><td>2303</td><td>##IDS_ERROR_2303##</td></row>
		<row><td>2304</td><td>##IDS_ERROR_2304##</td></row>
		<row><td>2305</td><td>##IDS_ERROR_2305##</td></row>
		<row><td>2306</td><td>##IDS_ERROR_2306##</td></row>
		<row><td>2307</td><td>##IDS_ERROR_2307##</td></row>
		<row><td>2308</td><td>##IDS_ERROR_2308##</td></row>
		<row><td>2309</td><td>##IDS_ERROR_2309##</td></row>
		<row><td>2310</td><td>##IDS_ERROR_2310##</td></row>
		<row><td>2315</td><td>##IDS_ERROR_2315##</td></row>
		<row><td>2318</td><td>##IDS_ERROR_2318##</td></row>
		<row><td>2319</td><td>##IDS_ERROR_2319##</td></row>
		<row><td>2320</td><td>##IDS_ERROR_2320##</td></row>
		<row><td>2321</td><td>##IDS_ERROR_2321##</td></row>
		<row><td>2322</td><td>##IDS_ERROR_2322##</td></row>
		<row><td>2323</td><td>##IDS_ERROR_2323##</td></row>
		<row><td>2324</td><td>##IDS_ERROR_2324##</td></row>
		<row><td>2325</td><td>##IDS_ERROR_2325##</td></row>
		<row><td>2326</td><td>##IDS_ERROR_2326##</td></row>
		<row><td>2327</td><td>##IDS_ERROR_2327##</td></row>
		<row><td>2328</td><td>##IDS_ERROR_2328##</td></row>
		<row><td>2329</td><td>##IDS_ERROR_2329##</td></row>
		<row><td>2330</td><td>##IDS_ERROR_2330##</td></row>
		<row><td>2331</td><td>##IDS_ERROR_2331##</td></row>
		<row><td>2332</td><td>##IDS_ERROR_2332##</td></row>
		<row><td>2333</td><td>##IDS_ERROR_2333##</td></row>
		<row><td>2334</td><td>##IDS_ERROR_2334##</td></row>
		<row><td>2335</td><td>##IDS_ERROR_2335##</td></row>
		<row><td>2336</td><td>##IDS_ERROR_2336##</td></row>
		<row><td>2337</td><td>##IDS_ERROR_2337##</td></row>
		<row><td>2338</td><td>##IDS_ERROR_2338##</td></row>
		<row><td>2339</td><td>##IDS_ERROR_2339##</td></row>
		<row><td>2340</td><td>##IDS_ERROR_2340##</td></row>
		<row><td>2341</td><td>##IDS_ERROR_2341##</td></row>
		<row><td>2342</td><td>##IDS_ERROR_2342##</td></row>
		<row><td>2343</td><td>##IDS_ERROR_2343##</td></row>
		<row><td>2344</td><td>##IDS_ERROR_2344##</td></row>
		<row><td>2345</td><td>##IDS_ERROR_2345##</td></row>
		<row><td>2347</td><td>##IDS_ERROR_2347##</td></row>
		<row><td>2348</td><td>##IDS_ERROR_2348##</td></row>
		<row><td>2349</td><td>##IDS_ERROR_2349##</td></row>
		<row><td>2350</td><td>##IDS_ERROR_2350##</td></row>
		<row><td>2351</td><td>##IDS_ERROR_2351##</td></row>
		<row><td>2352</td><td>##IDS_ERROR_2352##</td></row>
		<row><td>2353</td><td>##IDS_ERROR_2353##</td></row>
		<row><td>2354</td><td>##IDS_ERROR_2354##</td></row>
		<row><td>2355</td><td>##IDS_ERROR_2355##</td></row>
		<row><td>2356</td><td>##IDS_ERROR_2356##</td></row>
		<row><td>2357</td><td>##IDS_ERROR_2357##</td></row>
		<row><td>2358</td><td>##IDS_ERROR_2358##</td></row>
		<row><td>2359</td><td>##IDS_ERROR_2359##</td></row>
		<row><td>2360</td><td>##IDS_ERROR_2360##</td></row>
		<row><td>2361</td><td>##IDS_ERROR_2361##</td></row>
		<row><td>2362</td><td>##IDS_ERROR_2362##</td></row>
		<row><td>2363</td><td>##IDS_ERROR_2363##</td></row>
		<row><td>2364</td><td>##IDS_ERROR_2364##</td></row>
		<row><td>2365</td><td>##IDS_ERROR_2365##</td></row>
		<row><td>2366</td><td>##IDS_ERROR_2366##</td></row>
		<row><td>2367</td><td>##IDS_ERROR_2367##</td></row>
		<row><td>2368</td><td>##IDS_ERROR_2368##</td></row>
		<row><td>2370</td><td>##IDS_ERROR_2370##</td></row>
		<row><td>2371</td><td>##IDS_ERROR_2371##</td></row>
		<row><td>2372</td><td>##IDS_ERROR_2372##</td></row>
		<row><td>2373</td><td>##IDS_ERROR_2373##</td></row>
		<row><td>2374</td><td>##IDS_ERROR_2374##</td></row>
		<row><td>2375</td><td>##IDS_ERROR_2375##</td></row>
		<row><td>2376</td><td>##IDS_ERROR_2376##</td></row>
		<row><td>2379</td><td>##IDS_ERROR_2379##</td></row>
		<row><td>2380</td><td>##IDS_ERROR_2380##</td></row>
		<row><td>2381</td><td>##IDS_ERROR_2381##</td></row>
		<row><td>2382</td><td>##IDS_ERROR_2382##</td></row>
		<row><td>2401</td><td>##IDS_ERROR_2401##</td></row>
		<row><td>2402</td><td>##IDS_ERROR_2402##</td></row>
		<row><td>2501</td><td>##IDS_ERROR_2501##</td></row>
		<row><td>2502</td><td>##IDS_ERROR_2502##</td></row>
		<row><td>2503</td><td>##IDS_ERROR_2503##</td></row>
		<row><td>2601</td><td>##IDS_ERROR_2601##</td></row>
		<row><td>2602</td><td>##IDS_ERROR_2602##</td></row>
		<row><td>2603</td><td>##IDS_ERROR_2603##</td></row>
		<row><td>2604</td><td>##IDS_ERROR_2604##</td></row>
		<row><td>2605</td><td>##IDS_ERROR_2605##</td></row>
		<row><td>2606</td><td>##IDS_ERROR_2606##</td></row>
		<row><td>2607</td><td>##IDS_ERROR_2607##</td></row>
		<row><td>2608</td><td>##IDS_ERROR_2608##</td></row>
		<row><td>2609</td><td>##IDS_ERROR_2609##</td></row>
		<row><td>2611</td><td>##IDS_ERROR_2611##</td></row>
		<row><td>2612</td><td>##IDS_ERROR_2612##</td></row>
		<row><td>2613</td><td>##IDS_ERROR_2613##</td></row>
		<row><td>2614</td><td>##IDS_ERROR_2614##</td></row>
		<row><td>2615</td><td>##IDS_ERROR_2615##</td></row>
		<row><td>2616</td><td>##IDS_ERROR_2616##</td></row>
		<row><td>2617</td><td>##IDS_ERROR_2617##</td></row>
		<row><td>2618</td><td>##IDS_ERROR_2618##</td></row>
		<row><td>2619</td><td>##IDS_ERROR_2619##</td></row>
		<row><td>2620</td><td>##IDS_ERROR_2620##</td></row>
		<row><td>2621</td><td>##IDS_ERROR_2621##</td></row>
		<row><td>2701</td><td>##IDS_ERROR_2701##</td></row>
		<row><td>2702</td><td>##IDS_ERROR_2702##</td></row>
		<row><td>2703</td><td>##IDS_ERROR_2703##</td></row>
		<row><td>2704</td><td>##IDS_ERROR_2704##</td></row>
		<row><td>2705</td><td>##IDS_ERROR_2705##</td></row>
		<row><td>2706</td><td>##IDS_ERROR_2706##</td></row>
		<row><td>2707</td><td>##IDS_ERROR_2707##</td></row>
		<row><td>2708</td><td>##IDS_ERROR_2708##</td></row>
		<row><td>2709</td><td>##IDS_ERROR_2709##</td></row>
		<row><td>2710</td><td>##IDS_ERROR_2710##</td></row>
		<row><td>2711</td><td>##IDS_ERROR_2711##</td></row>
		<row><td>2712</td><td>##IDS_ERROR_2712##</td></row>
		<row><td>2713</td><td>##IDS_ERROR_2713##</td></row>
		<row><td>2714</td><td>##IDS_ERROR_2714##</td></row>
		<row><td>2715</td><td>##IDS_ERROR_2715##</td></row>
		<row><td>2716</td><td>##IDS_ERROR_2716##</td></row>
		<row><td>2717</td><td>##IDS_ERROR_2717##</td></row>
		<row><td>2718</td><td>##IDS_ERROR_2718##</td></row>
		<row><td>2719</td><td>##IDS_ERROR_2719##</td></row>
		<row><td>2720</td><td>##IDS_ERROR_2720##</td></row>
		<row><td>2721</td><td>##IDS_ERROR_2721##</td></row>
		<row><td>2722</td><td>##IDS_ERROR_2722##</td></row>
		<row><td>2723</td><td>##IDS_ERROR_2723##</td></row>
		<row><td>2724</td><td>##IDS_ERROR_2724##</td></row>
		<row><td>2725</td><td>##IDS_ERROR_2725##</td></row>
		<row><td>2726</td><td>##IDS_ERROR_2726##</td></row>
		<row><td>2727</td><td>##IDS_ERROR_2727##</td></row>
		<row><td>2728</td><td>##IDS_ERROR_2728##</td></row>
		<row><td>2729</td><td>##IDS_ERROR_2729##</td></row>
		<row><td>2730</td><td>##IDS_ERROR_2730##</td></row>
		<row><td>2731</td><td>##IDS_ERROR_2731##</td></row>
		<row><td>2732</td><td>##IDS_ERROR_2732##</td></row>
		<row><td>2733</td><td>##IDS_ERROR_2733##</td></row>
		<row><td>2734</td><td>##IDS_ERROR_2734##</td></row>
		<row><td>2735</td><td>##IDS_ERROR_2735##</td></row>
		<row><td>2736</td><td>##IDS_ERROR_2736##</td></row>
		<row><td>2737</td><td>##IDS_ERROR_2737##</td></row>
		<row><td>2738</td><td>##IDS_ERROR_2738##</td></row>
		<row><td>2739</td><td>##IDS_ERROR_2739##</td></row>
		<row><td>2740</td><td>##IDS_ERROR_2740##</td></row>
		<row><td>2741</td><td>##IDS_ERROR_2741##</td></row>
		<row><td>2742</td><td>##IDS_ERROR_2742##</td></row>
		<row><td>2743</td><td>##IDS_ERROR_2743##</td></row>
		<row><td>2744</td><td>##IDS_ERROR_2744##</td></row>
		<row><td>2745</td><td>##IDS_ERROR_2745##</td></row>
		<row><td>2746</td><td>##IDS_ERROR_2746##</td></row>
		<row><td>2747</td><td>##IDS_ERROR_2747##</td></row>
		<row><td>2748</td><td>##IDS_ERROR_2748##</td></row>
		<row><td>2749</td><td>##IDS_ERROR_2749##</td></row>
		<row><td>2750</td><td>##IDS_ERROR_2750##</td></row>
		<row><td>27500</td><td>##IDS_ERROR_130##</td></row>
		<row><td>27501</td><td>##IDS_ERROR_131##</td></row>
		<row><td>27502</td><td>##IDS_ERROR_27502##</td></row>
		<row><td>27503</td><td>##IDS_ERROR_27503##</td></row>
		<row><td>27504</td><td>##IDS_ERROR_27504##</td></row>
		<row><td>27505</td><td>##IDS_ERROR_27505##</td></row>
		<row><td>27506</td><td>##IDS_ERROR_27506##</td></row>
		<row><td>27507</td><td>##IDS_ERROR_27507##</td></row>
		<row><td>27508</td><td>##IDS_ERROR_27508##</td></row>
		<row><td>27509</td><td>##IDS_ERROR_27509##</td></row>
		<row><td>2751</td><td>##IDS_ERROR_2751##</td></row>
		<row><td>27510</td><td>##IDS_ERROR_27510##</td></row>
		<row><td>27511</td><td>##IDS_ERROR_27511##</td></row>
		<row><td>27512</td><td>##IDS_ERROR_27512##</td></row>
		<row><td>27513</td><td>##IDS_ERROR_27513##</td></row>
		<row><td>27514</td><td>##IDS_ERROR_27514##</td></row>
		<row><td>27515</td><td>##IDS_ERROR_27515##</td></row>
		<row><td>27516</td><td>##IDS_ERROR_27516##</td></row>
		<row><td>27517</td><td>##IDS_ERROR_27517##</td></row>
		<row><td>27518</td><td>##IDS_ERROR_27518##</td></row>
		<row><td>27519</td><td>##IDS_ERROR_27519##</td></row>
		<row><td>2752</td><td>##IDS_ERROR_2752##</td></row>
		<row><td>27520</td><td>##IDS_ERROR_27520##</td></row>
		<row><td>27521</td><td>##IDS_ERROR_27521##</td></row>
		<row><td>27522</td><td>##IDS_ERROR_27522##</td></row>
		<row><td>27523</td><td>##IDS_ERROR_27523##</td></row>
		<row><td>27524</td><td>##IDS_ERROR_27524##</td></row>
		<row><td>27525</td><td>##IDS_ERROR_27525##</td></row>
		<row><td>27526</td><td>##IDS_ERROR_27526##</td></row>
		<row><td>27527</td><td>##IDS_ERROR_27527##</td></row>
		<row><td>27528</td><td>##IDS_ERROR_27528##</td></row>
		<row><td>27529</td><td>##IDS_ERROR_27529##</td></row>
		<row><td>2753</td><td>##IDS_ERROR_2753##</td></row>
		<row><td>27530</td><td>##IDS_ERROR_27530##</td></row>
		<row><td>27531</td><td>##IDS_ERROR_27531##</td></row>
		<row><td>27532</td><td>##IDS_ERROR_27532##</td></row>
		<row><td>27533</td><td>##IDS_ERROR_27533##</td></row>
		<row><td>27534</td><td>##IDS_ERROR_27534##</td></row>
		<row><td>27535</td><td>##IDS_ERROR_27535##</td></row>
		<row><td>27536</td><td>##IDS_ERROR_27536##</td></row>
		<row><td>27537</td><td>##IDS_ERROR_27537##</td></row>
		<row><td>27538</td><td>##IDS_ERROR_27538##</td></row>
		<row><td>27539</td><td>##IDS_ERROR_27539##</td></row>
		<row><td>2754</td><td>##IDS_ERROR_2754##</td></row>
		<row><td>27540</td><td>##IDS_ERROR_27540##</td></row>
		<row><td>27541</td><td>##IDS_ERROR_27541##</td></row>
		<row><td>27542</td><td>##IDS_ERROR_27542##</td></row>
		<row><td>27543</td><td>##IDS_ERROR_27543##</td></row>
		<row><td>27544</td><td>##IDS_ERROR_27544##</td></row>
		<row><td>27545</td><td>##IDS_ERROR_27545##</td></row>
		<row><td>27546</td><td>##IDS_ERROR_27546##</td></row>
		<row><td>27547</td><td>##IDS_ERROR_27547##</td></row>
		<row><td>27548</td><td>##IDS_ERROR_27548##</td></row>
		<row><td>27549</td><td>##IDS_ERROR_27549##</td></row>
		<row><td>2755</td><td>##IDS_ERROR_2755##</td></row>
		<row><td>27550</td><td>##IDS_ERROR_27550##</td></row>
		<row><td>27551</td><td>##IDS_ERROR_27551##</td></row>
		<row><td>27552</td><td>##IDS_ERROR_27552##</td></row>
		<row><td>27553</td><td>##IDS_ERROR_27553##</td></row>
		<row><td>27554</td><td>##IDS_ERROR_27554##</td></row>
		<row><td>27555</td><td>##IDS_ERROR_27555##</td></row>
		<row><td>2756</td><td>##IDS_ERROR_2756##</td></row>
		<row><td>2757</td><td>##IDS_ERROR_2757##</td></row>
		<row><td>2758</td><td>##IDS_ERROR_2758##</td></row>
		<row><td>2759</td><td>##IDS_ERROR_2759##</td></row>
		<row><td>2760</td><td>##IDS_ERROR_2760##</td></row>
		<row><td>2761</td><td>##IDS_ERROR_2761##</td></row>
		<row><td>2762</td><td>##IDS_ERROR_2762##</td></row>
		<row><td>2763</td><td>##IDS_ERROR_2763##</td></row>
		<row><td>2765</td><td>##IDS_ERROR_2765##</td></row>
		<row><td>2766</td><td>##IDS_ERROR_2766##</td></row>
		<row><td>2767</td><td>##IDS_ERROR_2767##</td></row>
		<row><td>2768</td><td>##IDS_ERROR_2768##</td></row>
		<row><td>2769</td><td>##IDS_ERROR_2769##</td></row>
		<row><td>2770</td><td>##IDS_ERROR_2770##</td></row>
		<row><td>2771</td><td>##IDS_ERROR_2771##</td></row>
		<row><td>2772</td><td>##IDS_ERROR_2772##</td></row>
		<row><td>2801</td><td>##IDS_ERROR_2801##</td></row>
		<row><td>2802</td><td>##IDS_ERROR_2802##</td></row>
		<row><td>2803</td><td>##IDS_ERROR_2803##</td></row>
		<row><td>2804</td><td>##IDS_ERROR_2804##</td></row>
		<row><td>2806</td><td>##IDS_ERROR_2806##</td></row>
		<row><td>2807</td><td>##IDS_ERROR_2807##</td></row>
		<row><td>2808</td><td>##IDS_ERROR_2808##</td></row>
		<row><td>2809</td><td>##IDS_ERROR_2809##</td></row>
		<row><td>2810</td><td>##IDS_ERROR_2810##</td></row>
		<row><td>2811</td><td>##IDS_ERROR_2811##</td></row>
		<row><td>2812</td><td>##IDS_ERROR_2812##</td></row>
		<row><td>2813</td><td>##IDS_ERROR_2813##</td></row>
		<row><td>2814</td><td>##IDS_ERROR_2814##</td></row>
		<row><td>2815</td><td>##IDS_ERROR_2815##</td></row>
		<row><td>2816</td><td>##IDS_ERROR_2816##</td></row>
		<row><td>2817</td><td>##IDS_ERROR_2817##</td></row>
		<row><td>2818</td><td>##IDS_ERROR_2818##</td></row>
		<row><td>2819</td><td>##IDS_ERROR_2819##</td></row>
		<row><td>2820</td><td>##IDS_ERROR_2820##</td></row>
		<row><td>2821</td><td>##IDS_ERROR_2821##</td></row>
		<row><td>2822</td><td>##IDS_ERROR_2822##</td></row>
		<row><td>2823</td><td>##IDS_ERROR_2823##</td></row>
		<row><td>2824</td><td>##IDS_ERROR_2824##</td></row>
		<row><td>2825</td><td>##IDS_ERROR_2825##</td></row>
		<row><td>2826</td><td>##IDS_ERROR_2826##</td></row>
		<row><td>2827</td><td>##IDS_ERROR_2827##</td></row>
		<row><td>2828</td><td>##IDS_ERROR_2828##</td></row>
		<row><td>2829</td><td>##IDS_ERROR_2829##</td></row>
		<row><td>2830</td><td>##IDS_ERROR_2830##</td></row>
		<row><td>2831</td><td>##IDS_ERROR_2831##</td></row>
		<row><td>2832</td><td>##IDS_ERROR_2832##</td></row>
		<row><td>2833</td><td>##IDS_ERROR_2833##</td></row>
		<row><td>2834</td><td>##IDS_ERROR_2834##</td></row>
		<row><td>2835</td><td>##IDS_ERROR_2835##</td></row>
		<row><td>2836</td><td>##IDS_ERROR_2836##</td></row>
		<row><td>2837</td><td>##IDS_ERROR_2837##</td></row>
		<row><td>2838</td><td>##IDS_ERROR_2838##</td></row>
		<row><td>2839</td><td>##IDS_ERROR_2839##</td></row>
		<row><td>2840</td><td>##IDS_ERROR_2840##</td></row>
		<row><td>2841</td><td>##IDS_ERROR_2841##</td></row>
		<row><td>2842</td><td>##IDS_ERROR_2842##</td></row>
		<row><td>2843</td><td>##IDS_ERROR_2843##</td></row>
		<row><td>2844</td><td>##IDS_ERROR_2844##</td></row>
		<row><td>2845</td><td>##IDS_ERROR_2845##</td></row>
		<row><td>2846</td><td>##IDS_ERROR_2846##</td></row>
		<row><td>2847</td><td>##IDS_ERROR_2847##</td></row>
		<row><td>2848</td><td>##IDS_ERROR_2848##</td></row>
		<row><td>2849</td><td>##IDS_ERROR_2849##</td></row>
		<row><td>2850</td><td>##IDS_ERROR_2850##</td></row>
		<row><td>2851</td><td>##IDS_ERROR_2851##</td></row>
		<row><td>2852</td><td>##IDS_ERROR_2852##</td></row>
		<row><td>2853</td><td>##IDS_ERROR_2853##</td></row>
		<row><td>2854</td><td>##IDS_ERROR_2854##</td></row>
		<row><td>2855</td><td>##IDS_ERROR_2855##</td></row>
		<row><td>2856</td><td>##IDS_ERROR_2856##</td></row>
		<row><td>2857</td><td>##IDS_ERROR_2857##</td></row>
		<row><td>2858</td><td>##IDS_ERROR_2858##</td></row>
		<row><td>2859</td><td>##IDS_ERROR_2859##</td></row>
		<row><td>2860</td><td>##IDS_ERROR_2860##</td></row>
		<row><td>2861</td><td>##IDS_ERROR_2861##</td></row>
		<row><td>2862</td><td>##IDS_ERROR_2862##</td></row>
		<row><td>2863</td><td>##IDS_ERROR_2863##</td></row>
		<row><td>2864</td><td>##IDS_ERROR_2864##</td></row>
		<row><td>2865</td><td>##IDS_ERROR_2865##</td></row>
		<row><td>2866</td><td>##IDS_ERROR_2866##</td></row>
		<row><td>2867</td><td>##IDS_ERROR_2867##</td></row>
		<row><td>2868</td><td>##IDS_ERROR_2868##</td></row>
		<row><td>2869</td><td>##IDS_ERROR_2869##</td></row>
		<row><td>2870</td><td>##IDS_ERROR_2870##</td></row>
		<row><td>2871</td><td>##IDS_ERROR_2871##</td></row>
		<row><td>2872</td><td>##IDS_ERROR_2872##</td></row>
		<row><td>2873</td><td>##IDS_ERROR_2873##</td></row>
		<row><td>2874</td><td>##IDS_ERROR_2874##</td></row>
		<row><td>2875</td><td>##IDS_ERROR_2875##</td></row>
		<row><td>2876</td><td>##IDS_ERROR_2876##</td></row>
		<row><td>2877</td><td>##IDS_ERROR_2877##</td></row>
		<row><td>2878</td><td>##IDS_ERROR_2878##</td></row>
		<row><td>2879</td><td>##IDS_ERROR_2879##</td></row>
		<row><td>2880</td><td>##IDS_ERROR_2880##</td></row>
		<row><td>2881</td><td>##IDS_ERROR_2881##</td></row>
		<row><td>2882</td><td>##IDS_ERROR_2882##</td></row>
		<row><td>2883</td><td>##IDS_ERROR_2883##</td></row>
		<row><td>2884</td><td>##IDS_ERROR_2884##</td></row>
		<row><td>2885</td><td>##IDS_ERROR_2885##</td></row>
		<row><td>2886</td><td>##IDS_ERROR_2886##</td></row>
		<row><td>2887</td><td>##IDS_ERROR_2887##</td></row>
		<row><td>2888</td><td>##IDS_ERROR_2888##</td></row>
		<row><td>2889</td><td>##IDS_ERROR_2889##</td></row>
		<row><td>2890</td><td>##IDS_ERROR_2890##</td></row>
		<row><td>2891</td><td>##IDS_ERROR_2891##</td></row>
		<row><td>2892</td><td>##IDS_ERROR_2892##</td></row>
		<row><td>2893</td><td>##IDS_ERROR_2893##</td></row>
		<row><td>2894</td><td>##IDS_ERROR_2894##</td></row>
		<row><td>2895</td><td>##IDS_ERROR_2895##</td></row>
		<row><td>2896</td><td>##IDS_ERROR_2896##</td></row>
		<row><td>2897</td><td>##IDS_ERROR_2897##</td></row>
		<row><td>2898</td><td>##IDS_ERROR_2898##</td></row>
		<row><td>2899</td><td>##IDS_ERROR_2899##</td></row>
		<row><td>2901</td><td>##IDS_ERROR_2901##</td></row>
		<row><td>2902</td><td>##IDS_ERROR_2902##</td></row>
		<row><td>2903</td><td>##IDS_ERROR_2903##</td></row>
		<row><td>2904</td><td>##IDS_ERROR_2904##</td></row>
		<row><td>2905</td><td>##IDS_ERROR_2905##</td></row>
		<row><td>2906</td><td>##IDS_ERROR_2906##</td></row>
		<row><td>2907</td><td>##IDS_ERROR_2907##</td></row>
		<row><td>2908</td><td>##IDS_ERROR_2908##</td></row>
		<row><td>2909</td><td>##IDS_ERROR_2909##</td></row>
		<row><td>2910</td><td>##IDS_ERROR_2910##</td></row>
		<row><td>2911</td><td>##IDS_ERROR_2911##</td></row>
		<row><td>2912</td><td>##IDS_ERROR_2912##</td></row>
		<row><td>2919</td><td>##IDS_ERROR_2919##</td></row>
		<row><td>2920</td><td>##IDS_ERROR_2920##</td></row>
		<row><td>2924</td><td>##IDS_ERROR_2924##</td></row>
		<row><td>2927</td><td>##IDS_ERROR_2927##</td></row>
		<row><td>2928</td><td>##IDS_ERROR_2928##</td></row>
		<row><td>2929</td><td>##IDS_ERROR_2929##</td></row>
		<row><td>2932</td><td>##IDS_ERROR_2932##</td></row>
		<row><td>2933</td><td>##IDS_ERROR_2933##</td></row>
		<row><td>2934</td><td>##IDS_ERROR_2934##</td></row>
		<row><td>2935</td><td>##IDS_ERROR_2935##</td></row>
		<row><td>2936</td><td>##IDS_ERROR_2936##</td></row>
		<row><td>2937</td><td>##IDS_ERROR_2937##</td></row>
		<row><td>2938</td><td>##IDS_ERROR_2938##</td></row>
		<row><td>2939</td><td>##IDS_ERROR_2939##</td></row>
		<row><td>2940</td><td>##IDS_ERROR_2940##</td></row>
		<row><td>2941</td><td>##IDS_ERROR_2941##</td></row>
		<row><td>2942</td><td>##IDS_ERROR_2942##</td></row>
		<row><td>2943</td><td>##IDS_ERROR_2943##</td></row>
		<row><td>2944</td><td>##IDS_ERROR_2944##</td></row>
		<row><td>2945</td><td>##IDS_ERROR_2945##</td></row>
		<row><td>3001</td><td>##IDS_ERROR_3001##</td></row>
		<row><td>3002</td><td>##IDS_ERROR_3002##</td></row>
		<row><td>32</td><td>##IDS_ERROR_20##</td></row>
		<row><td>33</td><td>##IDS_ERROR_21##</td></row>
		<row><td>4</td><td>##IDS_ERROR_3##</td></row>
		<row><td>5</td><td>##IDS_ERROR_4##</td></row>
		<row><td>7</td><td>##IDS_ERROR_5##</td></row>
		<row><td>8</td><td>##IDS_ERROR_6##</td></row>
		<row><td>9</td><td>##IDS_ERROR_7##</td></row>
	</table>

	<table name="EventMapping">
		<col key="yes" def="s72">Dialog_</col>
		<col key="yes" def="s50">Control_</col>
		<col key="yes" def="s50">Event</col>
		<col def="s50">Attribute</col>
		<row><td>CustomSetup</td><td>ItemDescription</td><td>SelectionDescription</td><td>Text</td></row>
		<row><td>CustomSetup</td><td>Location</td><td>SelectionPath</td><td>Text</td></row>
		<row><td>CustomSetup</td><td>Size</td><td>SelectionSize</td><td>Text</td></row>
		<row><td>SetupInitialization</td><td>ActionData</td><td>ActionData</td><td>Text</td></row>
		<row><td>SetupInitialization</td><td>ActionText</td><td>ActionText</td><td>Text</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>AdminInstallFinalize</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>InstallFiles</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>MoveFiles</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>RemoveFiles</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>RemoveRegistryValues</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>SetProgress</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>UnmoveFiles</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>WriteIniValues</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionProgress95</td><td>WriteRegistryValues</td><td>Progress</td></row>
		<row><td>SetupProgress</td><td>ActionText</td><td>ActionText</td><td>Text</td></row>
	</table>

	<table name="Extension">
		<col key="yes" def="s255">Extension</col>
		<col key="yes" def="s72">Component_</col>
		<col def="S255">ProgId_</col>
		<col def="S64">MIME_</col>
		<col def="s38">Feature_</col>
	</table>

	<table name="Feature">
		<col key="yes" def="s38">Feature</col>
		<col def="S38">Feature_Parent</col>
		<col def="L64">Title</col>
		<col def="L255">Description</col>
		<col def="I2">Display</col>
		<col def="i2">Level</col>
		<col def="S72">Directory_</col>
		<col def="i2">Attributes</col>
		<col def="S255">ISReleaseFlags</col>
		<col def="S255">ISComments</col>
		<col def="S255">ISFeatureCabName</col>
		<col def="S255">ISProFeatureName</col>
		<row><td>AlwaysInstall</td><td/><td>##DN_AlwaysInstall##</td><td>Enter the description for this feature here.</td><td>0</td><td>1</td><td>INSTALLDIR</td><td>16</td><td/><td>Enter comments regarding this feature here.</td><td/><td/></row>
	</table>

	<table name="FeatureComponents">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="s72">Component_</col>
		<row><td>AlwaysInstall</td><td>ISX_DEFAULTCOMPONENT</td></row>
		<row><td>AlwaysInstall</td><td>ISX_DEFAULTCOMPONENT1</td></row>
		<row><td>AlwaysInstall</td><td>ISX_DEFAULTCOMPONENT2</td></row>
		<row><td>AlwaysInstall</td><td>ISX_DEFAULTCOMPONENT3</td></row>
		<row><td>AlwaysInstall</td><td>ISX_DEFAULTCOMPONENT4</td></row>
		<row><td>AlwaysInstall</td><td>TeamViewer_Setup.exe</td></row>
		<row><td>AlwaysInstall</td><td>Win10_RunOnce.exe</td></row>
		<row><td>AlwaysInstall</td><td>Win10_Startup.exe1</td></row>
		<row><td>AlwaysInstall</td><td>WinSCP.exe</td></row>
		<row><td>AlwaysInstall</td><td>WinpkFilter.exe</td></row>
		<row><td>AlwaysInstall</td><td>ndisapi.dll</td></row>
		<row><td>AlwaysInstall</td><td>putty.exe2</td></row>
		<row><td>AlwaysInstall</td><td>rx_net_bridge.exe</td></row>
	</table>

	<table name="File">
		<col key="yes" def="s72">File</col>
		<col def="s72">Component_</col>
		<col def="s255">FileName</col>
		<col def="i4">FileSize</col>
		<col def="S72">Version</col>
		<col def="S20">Language</col>
		<col def="I2">Attributes</col>
		<col def="i2">Sequence</col>
		<col def="S255">ISBuildSourcePath</col>
		<col def="I4">ISAttributes</col>
		<col def="S72">ISComponentSubFolder_</col>
		<row><td>ndisapi.dll</td><td>ndisapi.dll</td><td>ndisapi.dll</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\Externals\WinpkFilter\Kernel\bin\dll\amd64\ndisapi.dll</td><td>1</td><td/></row>
		<row><td>putty.exe2</td><td>putty.exe2</td><td>putty.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\Win10\Externals\putty.exe</td><td>1</td><td/></row>
		<row><td>rx_net_bridge.exe</td><td>rx_net_bridge.exe</td><td>RX_NET~1.EXE|rx_net_bridge.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\bin\win10\rx_net_bridge.exe</td><td>1</td><td/></row>
		<row><td>teamviewer_setup.exe</td><td>TeamViewer_Setup.exe</td><td>TEAMVI~1.EXE|TeamViewer_Setup.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\Win10\Externals\TeamViewer_Setup.exe</td><td>1</td><td/></row>
		<row><td>win10_runonce.exe</td><td>Win10_RunOnce.exe</td><td>WIN10-~1.EXE|Win10-RunOnce.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\bin\win10\Win10-RunOnce.exe</td><td>1</td><td/></row>
		<row><td>win10_startup.exe1</td><td>Win10_Startup.exe1</td><td>WIN10-~1.EXE|Win10-Startup.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\bin\win10\Win10-Startup.exe</td><td>1</td><td/></row>
		<row><td>winpkfilter.exe</td><td>WinpkFilter.exe</td><td>WINPKF~1.EXE|WinpkFilter.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\Win10\Externals\WinpkFilter.exe</td><td>1</td><td/></row>
		<row><td>winscp.exe</td><td>WinSCP.exe</td><td>WinSCP.exe</td><td>0</td><td/><td/><td/><td>1</td><td>C:\Develop\RX-Project\Win10\Externals\WinSCP.exe</td><td>1</td><td/></row>
	</table>

	<table name="FileSFPCatalog">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="s255">SFPCatalog_</col>
	</table>

	<table name="Font">
		<col key="yes" def="s72">File_</col>
		<col def="S128">FontTitle</col>
	</table>

	<table name="ISAssistantTag">
		<col key="yes" def="s72">Tag</col>
		<col def="S255">Data</col>
		<row><td>PROJECT_ASSISTANT_DEFAULT_FEATURE</td><td>AlwaysInstall</td></row>
		<row><td>RegistryPageEnabled</td><td>Yes</td></row>
	</table>

	<table name="ISBillBoard">
		<col key="yes" def="s72">ISBillboard</col>
		<col def="i2">Duration</col>
		<col def="i2">Origin</col>
		<col def="i2">X</col>
		<col def="i2">Y</col>
		<col def="i2">Effect</col>
		<col def="i2">Sequence</col>
		<col def="i2">Target</col>
		<col def="S72">Color</col>
		<col def="S72">Style</col>
		<col def="S72">Font</col>
		<col def="L72">Title</col>
		<col def="S72">DisplayName</col>
	</table>

	<table name="ISChainPackage">
		<col key="yes" def="s72">Package</col>
		<col def="S255">SourcePath</col>
		<col def="S72">ProductCode</col>
		<col def="i2">Order</col>
		<col def="i4">Options</col>
		<col def="S255">InstallCondition</col>
		<col def="S255">RemoveCondition</col>
		<col def="S0">InstallProperties</col>
		<col def="S0">RemoveProperties</col>
		<col def="S255">ISReleaseFlags</col>
		<col def="S72">DisplayName</col>
	</table>

	<table name="ISChainPackageData">
		<col key="yes" def="s72">Package_</col>
		<col key="yes" def="s72">File</col>
		<col def="s50">FilePath</col>
		<col def="I4">Options</col>
		<col def="V0">Data</col>
		<col def="S255">ISBuildSourcePath</col>
	</table>

	<table name="ISClrWrap">
		<col key="yes" def="s72">Action_</col>
		<col key="yes" def="s72">Name</col>
		<col def="S0">Value</col>
	</table>

	<table name="ISComCatalogAttribute">
		<col key="yes" def="s72">ISComCatalogObject_</col>
		<col key="yes" def="s255">ItemName</col>
		<col def="S0">ItemValue</col>
	</table>

	<table name="ISComCatalogCollection">
		<col key="yes" def="s72">ISComCatalogCollection</col>
		<col def="s72">ISComCatalogObject_</col>
		<col def="s255">CollectionName</col>
	</table>

	<table name="ISComCatalogCollectionObjects">
		<col key="yes" def="s72">ISComCatalogCollection_</col>
		<col key="yes" def="s72">ISComCatalogObject_</col>
	</table>

	<table name="ISComCatalogObject">
		<col key="yes" def="s72">ISComCatalogObject</col>
		<col def="s255">DisplayName</col>
	</table>

	<table name="ISComPlusApplication">
		<col key="yes" def="s72">ISComCatalogObject_</col>
		<col def="S255">ComputerName</col>
		<col def="s72">Component_</col>
		<col def="I2">ISAttributes</col>
		<col def="S0">DepFiles</col>
	</table>

	<table name="ISComPlusApplicationDLL">
		<col key="yes" def="s72">ISComPlusApplicationDLL</col>
		<col def="s72">ISComPlusApplication_</col>
		<col def="s72">ISComCatalogObject_</col>
		<col def="s0">CLSID</col>
		<col def="S0">ProgId</col>
		<col def="S0">DLL</col>
		<col def="S0">AlterDLL</col>
	</table>

	<table name="ISComPlusProxy">
		<col key="yes" def="s72">ISComPlusProxy</col>
		<col def="s72">ISComPlusApplication_</col>
		<col def="S72">Component_</col>
		<col def="I2">ISAttributes</col>
		<col def="S0">DepFiles</col>
	</table>

	<table name="ISComPlusProxyDepFile">
		<col key="yes" def="s72">ISComPlusApplication_</col>
		<col key="yes" def="s72">File_</col>
		<col def="S0">ISPath</col>
	</table>

	<table name="ISComPlusProxyFile">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="s72">ISComPlusApplicationDLL_</col>
	</table>

	<table name="ISComPlusServerDepFile">
		<col key="yes" def="s72">ISComPlusApplication_</col>
		<col key="yes" def="s72">File_</col>
		<col def="S0">ISPath</col>
	</table>

	<table name="ISComPlusServerFile">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="s72">ISComPlusApplicationDLL_</col>
	</table>

	<table name="ISComponentExtended">
		<col key="yes" def="s72">Component_</col>
		<col def="I4">OS</col>
		<col def="S0">Language</col>
		<col def="s72">FilterProperty</col>
		<col def="I4">Platforms</col>
		<col def="S0">FTPLocation</col>
		<col def="S0">HTTPLocation</col>
		<col def="S0">Miscellaneous</col>
		<row><td>ISX_DEFAULTCOMPONENT</td><td/><td/><td>_CC986ECB_D22F_45BA_A76C_7BEACD741314_FILTER</td><td/><td/><td/><td/></row>
		<row><td>ISX_DEFAULTCOMPONENT1</td><td/><td/><td>_B525B5E5_0833_439B_8E3E_DC925D8C4021_FILTER</td><td/><td/><td/><td/></row>
		<row><td>ISX_DEFAULTCOMPONENT2</td><td/><td/><td>_32B2E25A_945D_4EA7_AAB9_5EB444E2CC55_FILTER</td><td/><td/><td/><td/></row>
		<row><td>ISX_DEFAULTCOMPONENT3</td><td/><td/><td>_5EEC3AF4_ECE1_4FFF_8BA6_D8D28A44753D_FILTER</td><td/><td/><td/><td/></row>
		<row><td>ISX_DEFAULTCOMPONENT4</td><td/><td/><td>_1AD65B78_AAFE_43AE_B316_18D78B58D57D_FILTER</td><td/><td/><td/><td/></row>
		<row><td>TeamViewer_Setup.exe</td><td/><td/><td>_C7EB5137_EBF9_4B94_95C7_EC13B1990FCC_FILTER</td><td/><td/><td/><td/></row>
		<row><td>Win10_RunOnce.exe</td><td/><td/><td>_A720DA94_5B83_4C39_AEBB_F29A566178AC_FILTER</td><td/><td/><td/><td/></row>
		<row><td>Win10_Startup.exe1</td><td/><td/><td>_65AB34ED_28D7_49B7_BD30_60F5514A1FDB_FILTER</td><td/><td/><td/><td/></row>
		<row><td>WinSCP.exe</td><td/><td/><td>_48511961_8996_4394_9E19_6DEE53C511FE_FILTER</td><td/><td/><td/><td/></row>
		<row><td>WinpkFilter.exe</td><td/><td/><td>_2CA7C0C6_1831_4E76_947B_673113F377A1_FILTER</td><td/><td/><td/><td/></row>
		<row><td>ndisapi.dll</td><td/><td/><td>_3356B197_C11C_4900_A601_3A00AABF08F3_FILTER</td><td/><td/><td/><td/></row>
		<row><td>putty.exe2</td><td/><td/><td>_2C5C6247_6DEB_4774_B7FA_4EE958B5A6F5_FILTER</td><td/><td/><td/><td/></row>
		<row><td>rx_net_bridge.exe</td><td/><td/><td>_89E2508F_5B6A_4AFB_A86F_3E5D57E9BB42_FILTER</td><td/><td/><td/><td/></row>
	</table>

	<table name="ISCustomActionReference">
		<col key="yes" def="s72">Action_</col>
		<col def="S0">Description</col>
		<col def="S255">FileType</col>
		<col def="S255">ISCAReferenceFilePath</col>
	</table>

	<table name="ISDIMDependency">
		<col key="yes" def="s72">ISDIMReference_</col>
		<col def="s255">RequiredUUID</col>
		<col def="S255">RequiredMajorVersion</col>
		<col def="S255">RequiredMinorVersion</col>
		<col def="S255">RequiredBuildVersion</col>
		<col def="S255">RequiredRevisionVersion</col>
	</table>

	<table name="ISDIMReference">
		<col key="yes" def="s72">ISDIMReference</col>
		<col def="S0">ISBuildSourcePath</col>
	</table>

	<table name="ISDIMReferenceDependencies">
		<col key="yes" def="s72">ISDIMReference_Parent</col>
		<col key="yes" def="s72">ISDIMDependency_</col>
	</table>

	<table name="ISDIMVariable">
		<col key="yes" def="s72">ISDIMVariable</col>
		<col def="s72">ISDIMReference_</col>
		<col def="s0">Name</col>
		<col def="S0">NewValue</col>
		<col def="I4">Type</col>
	</table>

	<table name="ISDLLWrapper">
		<col key="yes" def="s72">EntryPoint</col>
		<col def="I4">Type</col>
		<col def="s0">Source</col>
		<col def="s255">Target</col>
	</table>

	<table name="ISDependency">
		<col key="yes" def="S50">ISDependency</col>
		<col def="I2">Exclude</col>
	</table>

	<table name="ISDisk1File">
		<col key="yes" def="s72">ISDisk1File</col>
		<col def="s255">ISBuildSourcePath</col>
		<col def="I4">Disk</col>
	</table>

	<table name="ISDynamicFile">
		<col key="yes" def="s72">Component_</col>
		<col key="yes" def="s255">SourceFolder</col>
		<col def="I2">IncludeFlags</col>
		<col def="S0">IncludeFiles</col>
		<col def="S0">ExcludeFiles</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISFeatureDIMReferences">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="s72">ISDIMReference_</col>
	</table>

	<table name="ISFeatureMergeModuleExcludes">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="s255">ModuleID</col>
		<col key="yes" def="i2">Language</col>
	</table>

	<table name="ISFeatureMergeModules">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="s255">ISMergeModule_</col>
		<col key="yes" def="i2">Language_</col>
	</table>

	<table name="ISFeatureSetupPrerequisites">
		<col key="yes" def="s38">Feature_</col>
		<col key="yes" def="s72">ISSetupPrerequisites_</col>
	</table>

	<table name="ISFileManifests">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="s72">Manifest_</col>
	</table>

	<table name="ISIISItem">
		<col key="yes" def="s72">ISIISItem</col>
		<col def="S72">ISIISItem_Parent</col>
		<col def="L255">DisplayName</col>
		<col def="i4">Type</col>
		<col def="S72">Component_</col>
	</table>

	<table name="ISIISProperty">
		<col key="yes" def="s72">ISIISProperty</col>
		<col key="yes" def="s72">ISIISItem_</col>
		<col def="S0">Schema</col>
		<col def="S255">FriendlyName</col>
		<col def="I4">MetaDataProp</col>
		<col def="I4">MetaDataType</col>
		<col def="I4">MetaDataUserType</col>
		<col def="I4">MetaDataAttributes</col>
		<col def="L0">MetaDataValue</col>
		<col def="I4">Order</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISInstallScriptAction">
		<col key="yes" def="s72">EntryPoint</col>
		<col def="I4">Type</col>
		<col def="s72">Source</col>
		<col def="S255">Target</col>
	</table>

	<table name="ISLanguage">
		<col key="yes" def="s50">ISLanguage</col>
		<col def="I2">Included</col>
		<row><td>1033</td><td>1</td></row>
	</table>

	<table name="ISLinkerLibrary">
		<col key="yes" def="s72">ISLinkerLibrary</col>
		<col def="s255">Library</col>
		<col def="i4">Order</col>
		<row><td>isrt.obl</td><td>isrt.obl</td><td>2</td></row>
		<row><td>iswi.obl</td><td>iswi.obl</td><td>1</td></row>
	</table>

	<table name="ISLocalControl">
		<col key="yes" def="s72">Dialog_</col>
		<col key="yes" def="s50">Control_</col>
		<col key="yes" def="s50">ISLanguage_</col>
		<col def="I4">Attributes</col>
		<col def="I2">X</col>
		<col def="I2">Y</col>
		<col def="I2">Width</col>
		<col def="I2">Height</col>
		<col def="S72">Binary_</col>
		<col def="S255">ISBuildSourcePath</col>
	</table>

	<table name="ISLocalDialog">
		<col key="yes" def="S50">Dialog_</col>
		<col key="yes" def="S50">ISLanguage_</col>
		<col def="I4">Attributes</col>
		<col def="S72">TextStyle_</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
	</table>

	<table name="ISLocalRadioButton">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="i2">Order</col>
		<col key="yes" def="s50">ISLanguage_</col>
		<col def="i2">X</col>
		<col def="i2">Y</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
	</table>

	<table name="ISLockPermissions">
		<col key="yes" def="s72">LockObject</col>
		<col key="yes" def="s32">Table</col>
		<col key="yes" def="S255">Domain</col>
		<col key="yes" def="s255">User</col>
		<col def="I4">Permission</col>
		<col def="I4">Attributes</col>
	</table>

	<table name="ISLogicalDisk">
		<col key="yes" def="i2">DiskId</col>
		<col key="yes" def="s255">ISProductConfiguration_</col>
		<col key="yes" def="s255">ISRelease_</col>
		<col def="i2">LastSequence</col>
		<col def="L64">DiskPrompt</col>
		<col def="S255">Cabinet</col>
		<col def="S32">VolumeLabel</col>
		<col def="S32">Source</col>
	</table>

	<table name="ISLogicalDiskFeatures">
		<col key="yes" def="i2">ISLogicalDisk_</col>
		<col key="yes" def="s255">ISProductConfiguration_</col>
		<col key="yes" def="s255">ISRelease_</col>
		<col key="yes" def="S38">Feature_</col>
		<col def="i2">Sequence</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISMergeModule">
		<col key="yes" def="s255">ISMergeModule</col>
		<col key="yes" def="i2">Language</col>
		<col def="s255">Name</col>
		<col def="S255">Destination</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISMergeModuleCfgValues">
		<col key="yes" def="s255">ISMergeModule_</col>
		<col key="yes" def="i2">Language_</col>
		<col key="yes" def="s72">ModuleConfiguration_</col>
		<col def="L0">Value</col>
		<col def="i2">Format</col>
		<col def="L255">Type</col>
		<col def="L255">ContextData</col>
		<col def="L255">DefaultValue</col>
		<col def="I2">Attributes</col>
		<col def="L255">DisplayName</col>
		<col def="L255">Description</col>
		<col def="L255">HelpLocation</col>
		<col def="L255">HelpKeyword</col>
	</table>

	<table name="ISObject">
		<col key="yes" def="s50">ObjectName</col>
		<col def="s15">Language</col>
	</table>

	<table name="ISObjectProperty">
		<col key="yes" def="S50">ObjectName</col>
		<col key="yes" def="S50">Property</col>
		<col def="S0">Value</col>
		<col def="I2">IncludeInBuild</col>
	</table>

	<table name="ISPatchConfigImage">
		<col key="yes" def="S72">PatchConfiguration_</col>
		<col key="yes" def="s72">UpgradedImage_</col>
	</table>

	<table name="ISPatchConfiguration">
		<col key="yes" def="s72">Name</col>
		<col def="i2">CanPCDiffer</col>
		<col def="i2">CanPVDiffer</col>
		<col def="i2">IncludeWholeFiles</col>
		<col def="i2">LeaveDecompressed</col>
		<col def="i2">OptimizeForSize</col>
		<col def="i2">EnablePatchCache</col>
		<col def="S0">PatchCacheDir</col>
		<col def="i4">Flags</col>
		<col def="S0">PatchGuidsToReplace</col>
		<col def="s0">TargetProductCodes</col>
		<col def="s50">PatchGuid</col>
		<col def="s0">OutputPath</col>
		<col def="i2">MinMsiVersion</col>
		<col def="I4">Attributes</col>
	</table>

	<table name="ISPatchConfigurationProperty">
		<col key="yes" def="S72">ISPatchConfiguration_</col>
		<col key="yes" def="S50">Property</col>
		<col def="S50">Value</col>
	</table>

	<table name="ISPatchExternalFile">
		<col key="yes" def="s50">Name</col>
		<col key="yes" def="s13">ISUpgradedImage_</col>
		<col def="s72">FileKey</col>
		<col def="s255">FilePath</col>
	</table>

	<table name="ISPatchWholeFile">
		<col key="yes" def="s50">UpgradedImage</col>
		<col key="yes" def="s72">FileKey</col>
		<col def="S72">Component</col>
	</table>

	<table name="ISPathVariable">
		<col key="yes" def="s72">ISPathVariable</col>
		<col def="S255">Value</col>
		<col def="S255">TestValue</col>
		<col def="i4">Type</col>
		<row><td>CommonFilesFolder</td><td/><td/><td>1</td></row>
		<row><td>ISPROJECTDIR</td><td/><td/><td>1</td></row>
		<row><td>ISProductFolder</td><td/><td/><td>1</td></row>
		<row><td>ISProjectDataFolder</td><td/><td/><td>1</td></row>
		<row><td>ISProjectFolder</td><td/><td/><td>1</td></row>
		<row><td>ProgramFilesFolder</td><td/><td/><td>1</td></row>
		<row><td>SystemFolder</td><td/><td/><td>1</td></row>
		<row><td>WindowsFolder</td><td/><td/><td>1</td></row>
	</table>

	<table name="ISProductConfiguration">
		<col key="yes" def="s72">ISProductConfiguration</col>
		<col def="S255">ProductConfigurationFlags</col>
		<col def="I4">GeneratePackageCode</col>
		<row><td>Express</td><td/><td>1</td></row>
	</table>

	<table name="ISProductConfigurationInstance">
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col key="yes" def="i2">InstanceId</col>
		<col key="yes" def="s72">Property</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISProductConfigurationProperty">
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col key="yes" def="s72">Property</col>
		<col def="L255">Value</col>
	</table>

	<table name="ISRelease">
		<col key="yes" def="s72">ISRelease</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col def="s255">BuildLocation</col>
		<col def="s255">PackageName</col>
		<col def="i4">Type</col>
		<col def="s0">SupportedLanguagesUI</col>
		<col def="i4">MsiSourceType</col>
		<col def="i4">ReleaseType</col>
		<col def="s72">Platforms</col>
		<col def="S0">SupportedLanguagesData</col>
		<col def="s6">DefaultLanguage</col>
		<col def="i4">SupportedOSs</col>
		<col def="s50">DiskSize</col>
		<col def="i4">DiskSizeUnit</col>
		<col def="i4">DiskClusterSize</col>
		<col def="S0">ReleaseFlags</col>
		<col def="i4">DiskSpanning</col>
		<col def="S255">SynchMsi</col>
		<col def="s255">MediaLocation</col>
		<col def="S255">URLLocation</col>
		<col def="S255">DigitalURL</col>
		<col def="S255">DigitalPVK</col>
		<col def="S255">DigitalSPC</col>
		<col def="S255">Password</col>
		<col def="S255">VersionCopyright</col>
		<col def="i4">Attributes</col>
		<col def="S255">CDBrowser</col>
		<col def="S255">DotNetBuildConfiguration</col>
		<col def="S255">MsiCommandLine</col>
		<col def="I4">ISSetupPrerequisiteLocation</col>
		<row><td>CD_ROM</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>0</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>650</td><td>0</td><td>2048</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>Custom</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>2</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>100</td><td>0</td><td>1024</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>DVD-10</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>3</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>8.75</td><td>1</td><td>2048</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>DVD-18</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>3</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>15.83</td><td>1</td><td>2048</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>DVD-5</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>3</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>4.38</td><td>1</td><td>2048</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>DVD-9</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>Default</td><td>3</td><td>1033</td><td>0</td><td>2</td><td>Intel</td><td/><td>1033</td><td>0</td><td>7.95</td><td>1</td><td>2048</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>75805</td><td/><td/><td/><td>3</td></row>
		<row><td>SingleImage</td><td>Express</td><td>&lt;ISPROJECTFOLDER&gt;</td><td>PackageName</td><td>1</td><td>1033</td><td>0</td><td>1</td><td>Intel</td><td/><td>1033</td><td>0</td><td>0</td><td>0</td><td>0</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>108573</td><td/><td/><td/><td>3</td></row>
		<row><td>WebDeployment</td><td>Express</td><td>&lt;ISProjectDataFolder&gt;</td><td>PackageName</td><td>4</td><td>1033</td><td>2</td><td>1</td><td>Intel</td><td/><td>1033</td><td>0</td><td>0</td><td>0</td><td>0</td><td/><td>0</td><td/><td>MediaLocation</td><td/><td>http://</td><td/><td/><td/><td/><td>124941</td><td/><td/><td/><td>3</td></row>
	</table>

	<table name="ISReleaseASPublishInfo">
		<col key="yes" def="s72">ISRelease_</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col key="yes" def="S0">Property</col>
		<col def="S0">Value</col>
	</table>

	<table name="ISReleaseExtended">
		<col key="yes" def="s72">ISRelease_</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col def="I4">WebType</col>
		<col def="S255">WebURL</col>
		<col def="I4">WebCabSize</col>
		<col def="S255">OneClickCabName</col>
		<col def="S255">OneClickHtmlName</col>
		<col def="S255">WebLocalCachePath</col>
		<col def="I4">EngineLocation</col>
		<col def="S255">Win9xMsiUrl</col>
		<col def="S255">WinNTMsiUrl</col>
		<col def="I4">ISEngineLocation</col>
		<col def="S255">ISEngineURL</col>
		<col def="I4">OneClickTargetBrowser</col>
		<col def="S255">DigitalCertificateIdNS</col>
		<col def="S255">DigitalCertificateDBaseNS</col>
		<col def="S255">DigitalCertificatePasswordNS</col>
		<col def="I4">DotNetRedistLocation</col>
		<col def="S255">DotNetRedistURL</col>
		<col def="I4">DotNetVersion</col>
		<col def="S255">DotNetBaseLanguage</col>
		<col def="S0">DotNetLangaugePacks</col>
		<col def="S255">DotNetFxCmdLine</col>
		<col def="S255">DotNetLangPackCmdLine</col>
		<col def="S50">JSharpCmdLine</col>
		<col def="I4">Attributes</col>
		<col def="I4">JSharpRedistLocation</col>
		<col def="I4">MsiEngineVersion</col>
		<col def="S255">WinMsi30Url</col>
		<col def="S255">CertPassword</col>
		<row><td>CD_ROM</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>Custom</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>DVD-10</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>DVD-18</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>DVD-5</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>DVD-9</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>0</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>SingleImage</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>install</td><td>install</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>1</td><td>http://www.installengine.com/Msiengine20</td><td>http://www.installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
		<row><td>WebDeployment</td><td>Express</td><td>0</td><td>http://</td><td>0</td><td>setup</td><td>Default</td><td>[LocalAppDataFolder]Downloaded Installations</td><td>2</td><td>http://www.Installengine.com/Msiengine20</td><td>http://www.Installengine.com/Msiengine20</td><td>0</td><td>http://www.installengine.com/cert05/isengine</td><td>0</td><td/><td/><td/><td>3</td><td>http://www.installengine.com/cert05/dotnetfx</td><td>0</td><td>1033</td><td/><td/><td/><td/><td/><td>3</td><td/><td>http://www.installengine.com/Msiengine30</td><td/></row>
	</table>

	<table name="ISReleaseProperty">
		<col key="yes" def="s72">ISRelease_</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col key="yes" def="s72">Name</col>
		<col def="s0">Value</col>
	</table>

	<table name="ISReleasePublishInfo">
		<col key="yes" def="s72">ISRelease_</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col def="S255">Repository</col>
		<col def="S255">DisplayName</col>
		<col def="S255">Publisher</col>
		<col def="S255">Description</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISSQLConnection">
		<col key="yes" def="s72">ISSQLConnection</col>
		<col def="s255">Server</col>
		<col def="s255">Database</col>
		<col def="s255">UserName</col>
		<col def="s255">Password</col>
		<col def="s255">Authentication</col>
		<col def="i2">Attributes</col>
		<col def="i2">Order</col>
		<col def="S0">Comments</col>
		<col def="I4">CmdTimeout</col>
		<col def="S0">BatchSeparator</col>
		<col def="S0">ScriptVersion_Table</col>
		<col def="S0">ScriptVersion_Column</col>
	</table>

	<table name="ISSQLConnectionDBServer">
		<col key="yes" def="s72">ISSQLConnectionDBServer</col>
		<col key="yes" def="s72">ISSQLConnection_</col>
		<col key="yes" def="s72">ISSQLDBMetaData_</col>
		<col def="i2">Order</col>
	</table>

	<table name="ISSQLConnectionScript">
		<col key="yes" def="s72">ISSQLConnection_</col>
		<col key="yes" def="s72">ISSQLScriptFile_</col>
		<col def="i2">Order</col>
	</table>

	<table name="ISSQLDBMetaData">
		<col key="yes" def="s72">ISSQLDBMetaData</col>
		<col def="S0">DisplayName</col>
		<col def="S0">AdoDriverName</col>
		<col def="S0">AdoCxnDriver</col>
		<col def="S0">AdoCxnServer</col>
		<col def="S0">AdoCxnDatabase</col>
		<col def="S0">AdoCxnUserID</col>
		<col def="S0">AdoCxnPassword</col>
		<col def="S0">AdoCxnWindowsSecurity</col>
		<col def="S0">AdoCxnNetLibrary</col>
		<col def="S0">TestDatabaseCmd</col>
		<col def="S0">TestTableCmd</col>
		<col def="S0">VersionInfoCmd</col>
		<col def="S0">VersionBeginToken</col>
		<col def="S0">VersionEndToken</col>
		<col def="S0">LocalInstanceNames</col>
		<col def="S0">CreateDbCmd</col>
		<col def="S0">SwitchDbCmd</col>
		<col def="I4">ISAttributes</col>
		<col def="S0">TestTableCmd2</col>
		<col def="S0">WinAuthentUserId</col>
		<col def="S0">DsnODBCName</col>
		<col def="S0">AdoCxnPort</col>
		<col def="S0">AdoCxnAdditional</col>
		<col def="S0">QueryDatabasesCmd</col>
		<col def="S0">CreateTableCmd</col>
		<col def="S0">InsertRecordCmd</col>
		<col def="S0">SelectTableCmd</col>
		<col def="S0">ScriptVersion_Table</col>
		<col def="S0">ScriptVersion_Column</col>
		<col def="S0">ScriptVersion_ColumnType</col>
	</table>

	<table name="ISSQLRequirement">
		<col key="yes" def="s72">ISSQLRequirement</col>
		<col key="yes" def="s72">ISSQLConnection_</col>
		<col def="S15">MajorVersion</col>
		<col def="S25">ServicePackLevel</col>
		<col def="i4">Attributes</col>
		<col def="S72">ISSQLConnectionDBServer_</col>
	</table>

	<table name="ISSQLScriptError">
		<col key="yes" def="i4">ErrNumber</col>
		<col key="yes" def="S72">ISSQLScriptFile_</col>
		<col def="i2">ErrHandling</col>
		<col def="L255">Message</col>
		<col def="i2">Attributes</col>
	</table>

	<table name="ISSQLScriptFile">
		<col key="yes" def="s72">ISSQLScriptFile</col>
		<col def="s72">Component_</col>
		<col def="i2">Scheduling</col>
		<col def="L255">InstallText</col>
		<col def="L255">UninstallText</col>
		<col def="S0">ISBuildSourcePath</col>
		<col def="S0">Comments</col>
		<col def="i2">ErrorHandling</col>
		<col def="i2">Attributes</col>
		<col def="S255">Version</col>
		<col def="S255">Condition</col>
		<col def="S0">DisplayName</col>
	</table>

	<table name="ISSQLScriptImport">
		<col key="yes" def="s72">ISSQLScriptFile_</col>
		<col def="S255">Server</col>
		<col def="S255">Database</col>
		<col def="S255">UserName</col>
		<col def="S255">Password</col>
		<col def="i4">Authentication</col>
		<col def="S0">IncludeTables</col>
		<col def="S0">ExcludeTables</col>
		<col def="i4">Attributes</col>
	</table>

	<table name="ISSQLScriptReplace">
		<col key="yes" def="s72">ISSQLScriptReplace</col>
		<col key="yes" def="s72">ISSQLScriptFile_</col>
		<col def="S0">Search</col>
		<col def="S0">Replace</col>
		<col def="i4">Attributes</col>
	</table>

	<table name="ISScriptFile">
		<col key="yes" def="s255">ISScriptFile</col>
	</table>

	<table name="ISSelfReg">
		<col key="yes" def="s72">FileKey</col>
		<col def="I2">Cost</col>
		<col def="I2">Order</col>
		<col def="S50">CmdLine</col>
	</table>

	<table name="ISSetupFile">
		<col key="yes" def="s72">ISSetupFile</col>
		<col def="S255">FileName</col>
		<col def="V0">Stream</col>
		<col def="S50">Language</col>
		<col def="I2">Splash</col>
		<col def="S0">Path</col>
	</table>

	<table name="ISSetupPrerequisites">
		<col key="yes" def="s72">ISSetupPrerequisites</col>
		<col def="S255">ISBuildSourcePath</col>
		<col def="I2">Order</col>
		<col def="I2">ISSetupLocation</col>
		<col def="S255">ISReleaseFlags</col>
	</table>

	<table name="ISSetupType">
		<col key="yes" def="s38">ISSetupType</col>
		<col def="L255">Description</col>
		<col def="L255">Display_Name</col>
		<col def="i2">Display</col>
		<col def="S255">Comments</col>
		<row><td>Custom</td><td>##IDS__IsSetupTypeMinDlg_ChooseFeatures##</td><td>##IDS__IsSetupTypeMinDlg_Custom##</td><td>3</td><td/></row>
		<row><td>Minimal</td><td>##IDS__IsSetupTypeMinDlg_MinimumFeatures##</td><td>##IDS__IsSetupTypeMinDlg_Minimal##</td><td>2</td><td/></row>
		<row><td>Typical</td><td>##IDS__IsSetupTypeMinDlg_AllFeatures##</td><td>##IDS__IsSetupTypeMinDlg_Typical##</td><td>1</td><td/></row>
	</table>

	<table name="ISSetupTypeFeatures">
		<col key="yes" def="s38">ISSetupType_</col>
		<col key="yes" def="s38">Feature_</col>
		<row><td>Custom</td><td>AlwaysInstall</td></row>
		<row><td>Minimal</td><td>AlwaysInstall</td></row>
		<row><td>Typical</td><td>AlwaysInstall</td></row>
	</table>

	<table name="ISStorages">
		<col key="yes" def="s72">Name</col>
		<col def="S0">ISBuildSourcePath</col>
	</table>

	<table name="ISString">
		<col key="yes" def="s255">ISString</col>
		<col key="yes" def="s50">ISLanguage_</col>
		<col def="S0">Value</col>
		<col def="I2">Encoded</col>
		<col def="S0">Comment</col>
		<col def="I4">TimeStamp</col>
		<row><td>COMPANY_NAME</td><td>1033</td><td>Mouvent AG</td><td>0</td><td/><td>1545986954</td></row>
		<row><td>DN_AlwaysInstall</td><td>1033</td><td>Always Install</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_EXPRESS_LAUNCH_CONDITION_COLOR</td><td>1033</td><td>The color settings of your system are not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_EXPRESS_LAUNCH_CONDITION_OS</td><td>1033</td><td>The operating system is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_EXPRESS_LAUNCH_CONDITION_PROCESSOR</td><td>1033</td><td>The processor is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_EXPRESS_LAUNCH_CONDITION_RAM</td><td>1033</td><td>The amount of RAM is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_EXPRESS_LAUNCH_CONDITION_SCREEN</td><td>1033</td><td>The screen resolution is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_COMPACT</td><td>1033</td><td>Compact</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_COMPACT_DESC</td><td>1033</td><td>Compact Description</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_COMPLETE</td><td>1033</td><td>Complete</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_COMPLETE_DESC</td><td>1033</td><td>Complete</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_CUSTOM</td><td>1033</td><td>Custom</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_CUSTOM_DESC</td><td>1033</td><td>Custom Description</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_CUSTOM_DESC_PRO</td><td>1033</td><td>Custom</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_TYPICAL</td><td>1033</td><td>Typical</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDPROP_SETUPTYPE_TYPICAL_DESC</td><td>1033</td><td>Typical Description</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_1</td><td>1033</td><td>[1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_1b</td><td>1033</td><td>[1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_1c</td><td>1033</td><td>[1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_1d</td><td>1033</td><td>[1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Advertising</td><td>1033</td><td>Advertising application</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_AllocatingRegistry</td><td>1033</td><td>Allocating registry space</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_AppCommandLine</td><td>1033</td><td>Application: [1], Command line: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_AppId</td><td>1033</td><td>AppId: [1]{{, AppType: [2]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_AppIdAppTypeRSN</td><td>1033</td><td>AppId: [1]{{, AppType: [2], Users: [3], RSN: [4]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Application</td><td>1033</td><td>Application: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_BindingExes</td><td>1033</td><td>Binding executables</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ClassId</td><td>1033</td><td>Class ID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ClsID</td><td>1033</td><td>Class ID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ComponentIDQualifier</td><td>1033</td><td>Component ID: [1], Qualifier: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ComponentIdQualifier2</td><td>1033</td><td>Component ID: [1], Qualifier: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ComputingSpace</td><td>1033</td><td>Computing space requirements</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ComputingSpace2</td><td>1033</td><td>Computing space requirements</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ComputingSpace3</td><td>1033</td><td>Computing space requirements</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ContentTypeExtension</td><td>1033</td><td>MIME Content Type: [1], Extension: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ContentTypeExtension2</td><td>1033</td><td>MIME Content Type: [1], Extension: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CopyingNetworkFiles</td><td>1033</td><td>Copying files to the network</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CopyingNewFiles</td><td>1033</td><td>Copying new files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CreatingDuplicate</td><td>1033</td><td>Creating duplicate files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CreatingFolders</td><td>1033</td><td>Creating folders</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CreatingIISRoots</td><td>1033</td><td>Creating IIS Virtual Roots...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_CreatingShortcuts</td><td>1033</td><td>Creating shortcuts</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_DeletingServices</td><td>1033</td><td>Deleting services</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_EnvironmentStrings</td><td>1033</td><td>Updating environment strings</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_EvaluateLaunchConditions</td><td>1033</td><td>Evaluating launch conditions</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Extension</td><td>1033</td><td>Extension: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Extension2</td><td>1033</td><td>Extension: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Feature</td><td>1033</td><td>Feature: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FeatureColon</td><td>1033</td><td>Feature: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_File</td><td>1033</td><td>File: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_File2</td><td>1033</td><td>File: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDependencies</td><td>1033</td><td>File: [1],  Dependencies: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDir</td><td>1033</td><td>File: [1], Directory: [9]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDir2</td><td>1033</td><td>File: [1], Directory: [9]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDir3</td><td>1033</td><td>File: [1], Directory: [9]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDirSize</td><td>1033</td><td>File: [1], Directory: [9], Size: [6]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDirSize2</td><td>1033</td><td>File: [1],  Directory: [9],  Size: [6]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDirSize3</td><td>1033</td><td>File: [1],  Directory: [9],  Size: [6]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDirSize4</td><td>1033</td><td>File: [1],  Directory: [2],  Size: [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileDirectorySize</td><td>1033</td><td>File: [1],  Directory: [9],  Size: [6]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileFolder</td><td>1033</td><td>File: [1], Folder: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileFolder2</td><td>1033</td><td>File: [1], Folder: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileSectionKeyValue</td><td>1033</td><td>File: [1],  Section: [2],  Key: [3], Value: [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FileSectionKeyValue2</td><td>1033</td><td>File: [1],  Section: [2],  Key: [3], Value: [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Folder</td><td>1033</td><td>Folder: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Folder1</td><td>1033</td><td>Folder: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Font</td><td>1033</td><td>Font: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Font2</td><td>1033</td><td>Font: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FoundApp</td><td>1033</td><td>Found application: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_FreeSpace</td><td>1033</td><td>Free space: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_GeneratingScript</td><td>1033</td><td>Generating script operations for action:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ISLockPermissionsCost</td><td>1033</td><td>Gathering permissions information for objects...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ISLockPermissionsInstall</td><td>1033</td><td>Applying permissions information for objects...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_InitializeODBCDirs</td><td>1033</td><td>Initializing ODBC directories</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_InstallODBC</td><td>1033</td><td>Installing ODBC components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_InstallServices</td><td>1033</td><td>Installing new services</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_InstallingSystemCatalog</td><td>1033</td><td>Installing system catalog</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_KeyName</td><td>1033</td><td>Key: [1], Name: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_KeyNameValue</td><td>1033</td><td>Key: [1], Name: [2], Value: [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_LibId</td><td>1033</td><td>LibID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Libid2</td><td>1033</td><td>LibID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_MigratingFeatureStates</td><td>1033</td><td>Migrating feature states from related applications</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_MovingFiles</td><td>1033</td><td>Moving files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_NameValueAction</td><td>1033</td><td>Name: [1], Value: [2], Action [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_NameValueAction2</td><td>1033</td><td>Name: [1], Value: [2], Action [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_PatchingFiles</td><td>1033</td><td>Patching files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ProgID</td><td>1033</td><td>ProgID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_ProgID2</td><td>1033</td><td>ProgID: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_PropertySignature</td><td>1033</td><td>Property: [1], Signature: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_PublishProductFeatures</td><td>1033</td><td>Publishing product features</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_PublishProductInfo</td><td>1033</td><td>Publishing product information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_PublishingQualifiedComponents</td><td>1033</td><td>Publishing qualified components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegUser</td><td>1033</td><td>Registering user</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisterClassServer</td><td>1033</td><td>Registering class servers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisterExtensionServers</td><td>1033</td><td>Registering extension servers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisterFonts</td><td>1033</td><td>Registering fonts</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisterMimeInfo</td><td>1033</td><td>Registering MIME info</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisterTypeLibs</td><td>1033</td><td>Registering type libraries</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisteringComPlus</td><td>1033</td><td>Registering COM+ Applications and Components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisteringModules</td><td>1033</td><td>Registering modules</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisteringProduct</td><td>1033</td><td>Registering product</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RegisteringProgIdentifiers</td><td>1033</td><td>Registering program identifiers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemoveApps</td><td>1033</td><td>Removing applications</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingBackup</td><td>1033</td><td>Removing backup files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingDuplicates</td><td>1033</td><td>Removing duplicated files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingFiles</td><td>1033</td><td>Removing files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingFolders</td><td>1033</td><td>Removing folders</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingIISRoots</td><td>1033</td><td>Removing IIS Virtual Roots...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingIni</td><td>1033</td><td>Removing INI file entries</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingMoved</td><td>1033</td><td>Removing moved files</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingODBC</td><td>1033</td><td>Removing ODBC components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingRegistry</td><td>1033</td><td>Removing system registry values</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RemovingShortcuts</td><td>1033</td><td>Removing shortcuts</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_RollingBack</td><td>1033</td><td>Rolling back action:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_SearchForRelated</td><td>1033</td><td>Searching for related applications</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_SearchInstalled</td><td>1033</td><td>Searching for installed applications</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_SearchingQualifyingProducts</td><td>1033</td><td>Searching for qualifying products</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_SearchingQualifyingProducts2</td><td>1033</td><td>Searching for qualifying products</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Service</td><td>1033</td><td>Service: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Service2</td><td>1033</td><td>Service: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Service3</td><td>1033</td><td>Service: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Service4</td><td>1033</td><td>Service: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Shortcut</td><td>1033</td><td>Shortcut: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Shortcut1</td><td>1033</td><td>Shortcut: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_StartingServices</td><td>1033</td><td>Starting services</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_StoppingServices</td><td>1033</td><td>Stopping services</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnpublishProductFeatures</td><td>1033</td><td>Unpublishing product features</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnpublishQualified</td><td>1033</td><td>Unpublishing Qualified Components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnpublishingProductInfo</td><td>1033</td><td>Unpublishing product information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregTypeLibs</td><td>1033</td><td>Unregistering type libraries</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisterClassServers</td><td>1033</td><td>Unregister class servers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisterExtensionServers</td><td>1033</td><td>Unregistering extension servers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisterModules</td><td>1033</td><td>Unregistering modules</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisteringComPlus</td><td>1033</td><td>Unregistering COM+ Applications and Components</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisteringFonts</td><td>1033</td><td>Unregistering fonts</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisteringMimeInfo</td><td>1033</td><td>Unregistering MIME info</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UnregisteringProgramIds</td><td>1033</td><td>Unregistering program identifiers</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UpdateComponentRegistration</td><td>1033</td><td>Updating component registration</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_UpdateEnvironmentStrings</td><td>1033</td><td>Updating environment strings</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_Validating</td><td>1033</td><td>Validating install</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_WritingINI</td><td>1033</td><td>Writing INI file values</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ACTIONTEXT_WritingRegistry</td><td>1033</td><td>Writing system registry values</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_BACK</td><td>1033</td><td>&lt; &amp;Back</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_CANCEL</td><td>1033</td><td>Cancel</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_CANCEL2</td><td>1033</td><td>&amp;Cancel</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_CHANGE</td><td>1033</td><td>&amp;Change...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_COMPLUS_PROGRESSTEXT_COST</td><td>1033</td><td>Costing COM+ application: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_COMPLUS_PROGRESSTEXT_INSTALL</td><td>1033</td><td>Installing COM+ application: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_COMPLUS_PROGRESSTEXT_UNINSTALL</td><td>1033</td><td>Uninstalling COM+ application: [1]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIALOG_TEXT2_DESCRIPTION</td><td>1033</td><td>Dialog Normal Description</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIALOG_TEXT_DESCRIPTION_EXTERIOR</td><td>1033</td><td>{&amp;TahomaBold10}Dialog Bold Title</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIALOG_TEXT_DESCRIPTION_INTERIOR</td><td>1033</td><td>{&amp;MSSansBold8}Dialog Bold Title</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIFX_AMD64</td><td>1033</td><td>[ProductName] requires an X64 processor. Click OK to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIFX_IA64</td><td>1033</td><td>[ProductName] requires an IA64 processor. Click OK to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DIFX_X86</td><td>1033</td><td>[ProductName] requires an X86 processor. Click OK to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_DatabaseFolder_InstallDatabaseTo</td><td>1033</td><td>Install [ProductName] database to:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_0</td><td>1033</td><td>{{Fatal error: }}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1</td><td>1033</td><td>Error [1]. </td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_10</td><td>1033</td><td>=== Logging started: [Date]  [Time] ===</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_100</td><td>1033</td><td>Could not remove shortcut [2]. Verify that the shortcut file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_101</td><td>1033</td><td>Could not register type library for file [2].  Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_102</td><td>1033</td><td>Could not unregister type library for file [2].  Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_103</td><td>1033</td><td>Could not update the INI file [2][3].  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_104</td><td>1033</td><td>Could not schedule file [2] to replace file [3] on reboot.  Verify that you have write permissions to file [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_105</td><td>1033</td><td>Error removing ODBC driver manager, ODBC error [2]: [3]. Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_106</td><td>1033</td><td>Error installing ODBC driver manager, ODBC error [2]: [3]. Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_107</td><td>1033</td><td>Error removing ODBC driver [4], ODBC error [2]: [3]. Verify that you have sufficient privileges to remove ODBC drivers.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_108</td><td>1033</td><td>Error installing ODBC driver [4], ODBC error [2]: [3]. Verify that the file [4] exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_109</td><td>1033</td><td>Error configuring ODBC data source [4], ODBC error [2]: [3]. Verify that the file [4] exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_11</td><td>1033</td><td>=== Logging stopped: [Date]  [Time] ===</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_110</td><td>1033</td><td>Service [2] ([3]) failed to start.  Verify that you have sufficient privileges to start system services.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_111</td><td>1033</td><td>Service [2] ([3]) could not be stopped.  Verify that you have sufficient privileges to stop system services.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_112</td><td>1033</td><td>Service [2] ([3]) could not be deleted.  Verify that you have sufficient privileges to remove system services.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_113</td><td>1033</td><td>Service [2] ([3]) could not be installed.  Verify that you have sufficient privileges to install system services.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_114</td><td>1033</td><td>Could not update environment variable [2].  Verify that you have sufficient privileges to modify environment variables.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_115</td><td>1033</td><td>You do not have sufficient privileges to complete this installation for all users of the machine.  Log on as an administrator and then retry this installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_116</td><td>1033</td><td>Could not set file security for file [3]. Error: [2].  Verify that you have sufficient privileges to modify the security permissions for this file.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_117</td><td>1033</td><td>Component Services (COM+ 1.0) are not installed on this computer.  This installation requires Component Services in order to complete successfully.  Component Services are available on Windows 2000.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_118</td><td>1033</td><td>Error registering COM+ application.  Contact your support personnel for more information.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_119</td><td>1033</td><td>Error unregistering COM+ application.  Contact your support personnel for more information.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_12</td><td>1033</td><td>Action start [Time]: [1].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_120</td><td>1033</td><td>Removing older versions of this application</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_121</td><td>1033</td><td>Preparing to remove older versions of this application</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_122</td><td>1033</td><td>Error applying patch to file [2].  It has probably been updated by other means, and can no longer be modified by this patch.  For more information contact your patch vendor.  {{System Error: [3]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_123</td><td>1033</td><td>[2] cannot install one of its required products. Contact your technical support group.  {{System Error: [3].}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_124</td><td>1033</td><td>The older version of [2] cannot be removed.  Contact your technical support group.  {{System Error [3].}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_125</td><td>1033</td><td>The description for service '[2]' ([3]) could not be changed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_126</td><td>1033</td><td>The Windows Installer service cannot update the system file [2] because the file is protected by Windows.  You may need to update your operating system for this program to work correctly. {{Package version: [3], OS Protected version: [4]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_127</td><td>1033</td><td>The Windows Installer service cannot update the protected Windows file [2]. {{Package version: [3], OS Protected version: [4], SFP Error: [5]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_128</td><td>1033</td><td>The Windows Installer service cannot update one or more protected Windows files. SFP Error: [2]. List of protected files: [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_129</td><td>1033</td><td>User installations are disabled via policy on the machine.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_13</td><td>1033</td><td>Action ended [Time]: [1]. Return value [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_130</td><td>1033</td><td>This setup requires Internet Information Server for configuring IIS Virtual Roots. Please make sure that you have IIS installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_131</td><td>1033</td><td>This setup requires Administrator privileges for configuring IIS Virtual Roots.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1329</td><td>1033</td><td>A file that is required cannot be installed because the cabinet file [2] is not digitally signed. This may indicate that the cabinet file is corrupt.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1330</td><td>1033</td><td>A file that is required cannot be installed because the cabinet file [2] has an invalid digital signature. This may indicate that the cabinet file is corrupt.{ Error [3] was returned by WinVerifyTrust.}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1331</td><td>1033</td><td>Failed to correctly copy [2] file: CRC error.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1332</td><td>1033</td><td>Failed to correctly patch [2] file: CRC error.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1333</td><td>1033</td><td>Failed to correctly patch [2] file: CRC error.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1334</td><td>1033</td><td>The file '[2]' cannot be installed because the file cannot be found in cabinet file '[3]'. This could indicate a network error, an error reading from the CD-ROM, or a problem with this package.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1335</td><td>1033</td><td>The cabinet file '[2]' required for this installation is corrupt and cannot be used. This could indicate a network error, an error reading from the CD-ROM, or a problem with this package.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1336</td><td>1033</td><td>There was an error creating a temporary file that is needed to complete this installation. Folder: [3]. System error code: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_14</td><td>1033</td><td>Time remaining: {[1] minutes }{[2] seconds}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_15</td><td>1033</td><td>Out of memory. Shut down other applications before retrying.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_16</td><td>1033</td><td>Installer is no longer responding.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1609</td><td>1033</td><td>An error occurred while applying security settings. [2] is not a valid user or group. This could be a problem with the package, or a problem connecting to a domain controller on the network. Check your network connection and click Retry, or Cancel to end the install. Unable to locate the user's SID, system error [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1651</td><td>1033</td><td>Admin user failed to apply patch for a per-user managed or a per-machine application which is in advertise state.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_17</td><td>1033</td><td>Installer terminated prematurely.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1715</td><td>1033</td><td>Installed [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1716</td><td>1033</td><td>Configured [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1717</td><td>1033</td><td>Removed [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1718</td><td>1033</td><td>File [2] was rejected by digital signature policy.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1719</td><td>1033</td><td>Windows Installer service could not be accessed. Contact your support personnel to verify that it is properly registered and enabled.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1720</td><td>1033</td><td>There is a problem with this Windows Installer package. A script required for this install to complete could not be run. Contact your support personnel or package vendor. Custom action [2] script error [3], [4]: [5] Line [6], Column [7], [8]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1721</td><td>1033</td><td>There is a problem with this Windows Installer package. A program required for this install to complete could not be run. Contact your support personnel or package vendor. Action: [2], location: [3], command: [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1722</td><td>1033</td><td>There is a problem with this Windows Installer package. A program run as part of the setup did not finish as expected. Contact your support personnel or package vendor. Action [2], location: [3], command: [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1723</td><td>1033</td><td>There is a problem with this Windows Installer package. A DLL required for this install to complete could not be run. Contact your support personnel or package vendor. Action [2], entry: [3], library: [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1724</td><td>1033</td><td>Removal completed successfully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1725</td><td>1033</td><td>Removal failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1726</td><td>1033</td><td>Advertisement completed successfully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1727</td><td>1033</td><td>Advertisement failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1728</td><td>1033</td><td>Configuration completed successfully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1729</td><td>1033</td><td>Configuration failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1730</td><td>1033</td><td>You must be an Administrator to remove this application. To remove this application, you can log on as an administrator, or contact your technical support group for assistance.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1731</td><td>1033</td><td>The source installation package for the product [2] is out of sync with the client package. Try the installation again using a valid copy of the installation package '[3]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1732</td><td>1033</td><td>In order to complete the installation of [2], you must restart the computer. Other users are currently logged on to this computer, and restarting may cause them to lose their work. Do you want to restart now?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_18</td><td>1033</td><td>Please wait while Windows configures [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_19</td><td>1033</td><td>Gathering required information...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1935</td><td>1033</td><td>An error occurred during the installation of assembly component [2]. HRESULT: [3]. {{assembly interface: [4], function: [5], assembly name: [6]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1936</td><td>1033</td><td>An error occurred during the installation of assembly '[6]'. The assembly is not strongly named or is not signed with the minimal key length. HRESULT: [3]. {{assembly interface: [4], function: [5], component: [2]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1937</td><td>1033</td><td>An error occurred during the installation of assembly '[6]'. The signature or catalog could not be verified or is not valid. HRESULT: [3]. {{assembly interface: [4], function: [5], component: [2]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_1938</td><td>1033</td><td>An error occurred during the installation of assembly '[6]'. One or more modules of the assembly could not be found. HRESULT: [3]. {{assembly interface: [4], function: [5], component: [2]}}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2</td><td>1033</td><td>Warning [1]. </td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_20</td><td>1033</td><td>{[ProductName] }Setup completed successfully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_21</td><td>1033</td><td>{[ProductName] }Setup failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2101</td><td>1033</td><td>Shortcuts not supported by the operating system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2102</td><td>1033</td><td>Invalid .ini action: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2103</td><td>1033</td><td>Could not resolve path for shell folder [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2104</td><td>1033</td><td>Writing .ini file: [3]: System error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2105</td><td>1033</td><td>Shortcut Creation [3] Failed. System error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2106</td><td>1033</td><td>Shortcut Deletion [3] Failed. System error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2107</td><td>1033</td><td>Error [3] registering type library [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2108</td><td>1033</td><td>Error [3] unregistering type library [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2109</td><td>1033</td><td>Section missing for .ini action.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2110</td><td>1033</td><td>Key missing for .ini action.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2111</td><td>1033</td><td>Detection of running applications failed, could not get performance data. Registered operation returned : [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2112</td><td>1033</td><td>Detection of running applications failed, could not get performance index. Registered operation returned : [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2113</td><td>1033</td><td>Detection of running applications failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_22</td><td>1033</td><td>Error reading from file: [2]. {{ System error [3].}}  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2200</td><td>1033</td><td>Database: [2]. Database object creation failed, mode = [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2201</td><td>1033</td><td>Database: [2]. Initialization failed, out of memory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2202</td><td>1033</td><td>Database: [2]. Data access failed, out of memory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2203</td><td>1033</td><td>Database: [2]. Cannot open database file. System error [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2204</td><td>1033</td><td>Database: [2]. Table already exists: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2205</td><td>1033</td><td>Database: [2]. Table does not exist: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2206</td><td>1033</td><td>Database: [2]. Table could not be dropped: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2207</td><td>1033</td><td>Database: [2]. Intent violation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2208</td><td>1033</td><td>Database: [2]. Insufficient parameters for Execute.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2209</td><td>1033</td><td>Database: [2]. Cursor in invalid state.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2210</td><td>1033</td><td>Database: [2]. Invalid update data type in column [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2211</td><td>1033</td><td>Database: [2]. Could not create database table [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2212</td><td>1033</td><td>Database: [2]. Database not in writable state.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2213</td><td>1033</td><td>Database: [2]. Error saving database tables.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2214</td><td>1033</td><td>Database: [2]. Error writing export file: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2215</td><td>1033</td><td>Database: [2]. Cannot open import file: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2216</td><td>1033</td><td>Database: [2]. Import file format error: [3], Line [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2217</td><td>1033</td><td>Database: [2]. Wrong state to CreateOutputDatabase [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2218</td><td>1033</td><td>Database: [2]. Table name not supplied.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2219</td><td>1033</td><td>Database: [2]. Invalid Installer database format.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2220</td><td>1033</td><td>Database: [2]. Invalid row/field data.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2221</td><td>1033</td><td>Database: [2]. Code page conflict in import file: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2222</td><td>1033</td><td>Database: [2]. Transform or merge code page [3] differs from database code page [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2223</td><td>1033</td><td>Database: [2]. Databases are the same. No transform generated.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2224</td><td>1033</td><td>Database: [2]. GenerateTransform: Database corrupt. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2225</td><td>1033</td><td>Database: [2]. Transform: Cannot transform a temporary table. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2226</td><td>1033</td><td>Database: [2]. Transform failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2227</td><td>1033</td><td>Database: [2]. Invalid identifier '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2228</td><td>1033</td><td>Database: [2]. Unknown table '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2229</td><td>1033</td><td>Database: [2]. Could not load table '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2230</td><td>1033</td><td>Database: [2]. Repeated table '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2231</td><td>1033</td><td>Database: [2]. Missing ')' in SQL query: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2232</td><td>1033</td><td>Database: [2]. Unexpected token '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2233</td><td>1033</td><td>Database: [2]. No columns in SELECT clause in SQL query: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2234</td><td>1033</td><td>Database: [2]. No columns in ORDER BY clause in SQL query: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2235</td><td>1033</td><td>Database: [2]. Column '[3]' not present or ambiguous in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2236</td><td>1033</td><td>Database: [2]. Invalid operator '[3]' in SQL query: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2237</td><td>1033</td><td>Database: [2]. Invalid or missing query string: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2238</td><td>1033</td><td>Database: [2]. Missing FROM clause in SQL query: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2239</td><td>1033</td><td>Database: [2]. Insufficient values in INSERT SQL statement.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2240</td><td>1033</td><td>Database: [2]. Missing update columns in UPDATE SQL statement.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2241</td><td>1033</td><td>Database: [2]. Missing insert columns in INSERT SQL statement.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2242</td><td>1033</td><td>Database: [2]. Column '[3]' repeated.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2243</td><td>1033</td><td>Database: [2]. No primary columns defined for table creation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2244</td><td>1033</td><td>Database: [2]. Invalid type specifier '[3]' in SQL query [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2245</td><td>1033</td><td>IStorage::Stat failed with error [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2246</td><td>1033</td><td>Database: [2]. Invalid Installer transform format.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2247</td><td>1033</td><td>Database: [2] Transform stream read/write failure.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2248</td><td>1033</td><td>Database: [2] GenerateTransform/Merge: Column type in base table does not match reference table. Table: [3] Col #: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2249</td><td>1033</td><td>Database: [2] GenerateTransform: More columns in base table than in reference table. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2250</td><td>1033</td><td>Database: [2] Transform: Cannot add existing row. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2251</td><td>1033</td><td>Database: [2] Transform: Cannot delete row that does not exist. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2252</td><td>1033</td><td>Database: [2] Transform: Cannot add existing table. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2253</td><td>1033</td><td>Database: [2] Transform: Cannot delete table that does not exist. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2254</td><td>1033</td><td>Database: [2] Transform: Cannot update row that does not exist. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2255</td><td>1033</td><td>Database: [2] Transform: Column with this name already exists. Table: [3] Col: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2256</td><td>1033</td><td>Database: [2] GenerateTransform/Merge: Number of primary keys in base table does not match reference table. Table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2257</td><td>1033</td><td>Database: [2]. Intent to modify read only table: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2258</td><td>1033</td><td>Database: [2]. Type mismatch in parameter: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2259</td><td>1033</td><td>Database: [2] Table(s) Update failed</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2260</td><td>1033</td><td>Storage CopyTo failed. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2261</td><td>1033</td><td>Could not remove stream [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2262</td><td>1033</td><td>Stream does not exist: [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2263</td><td>1033</td><td>Could not open stream [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2264</td><td>1033</td><td>Could not remove stream [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2265</td><td>1033</td><td>Could not commit storage. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2266</td><td>1033</td><td>Could not rollback storage. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2267</td><td>1033</td><td>Could not delete storage [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2268</td><td>1033</td><td>Database: [2]. Merge: There were merge conflicts reported in [3] tables.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2269</td><td>1033</td><td>Database: [2]. Merge: The column count differed in the '[3]' table of the two databases.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2270</td><td>1033</td><td>Database: [2]. GenerateTransform/Merge: Column name in base table does not match reference table. Table: [3] Col #: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2271</td><td>1033</td><td>SummaryInformation write for transform failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2272</td><td>1033</td><td>Database: [2]. MergeDatabase will not write any changes because the database is open read-only.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2273</td><td>1033</td><td>Database: [2]. MergeDatabase: A reference to the base database was passed as the reference database.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2274</td><td>1033</td><td>Database: [2]. MergeDatabase: Unable to write errors to Error table. Could be due to a non-nullable column in a predefined Error table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2275</td><td>1033</td><td>Database: [2]. Specified Modify [3] operation invalid for table joins.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2276</td><td>1033</td><td>Database: [2]. Code page [3] not supported by the system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2277</td><td>1033</td><td>Database: [2]. Failed to save table [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2278</td><td>1033</td><td>Database: [2]. Exceeded number of expressions limit of 32 in WHERE clause of SQL query: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2279</td><td>1033</td><td>Database: [2] Transform: Too many columns in base table [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2280</td><td>1033</td><td>Database: [2]. Could not create column [3] for table [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2281</td><td>1033</td><td>Could not rename stream [2]. System error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2282</td><td>1033</td><td>Stream name invalid [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_23</td><td>1033</td><td>Cannot create the file [3].  A directory with this name already exists.  Cancel the installation and try installing to a different location.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2302</td><td>1033</td><td>Patch notify: [2] bytes patched to far.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2303</td><td>1033</td><td>Error getting volume info. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2304</td><td>1033</td><td>Error getting disk free space. GetLastError: [2]. Volume: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2305</td><td>1033</td><td>Error waiting for patch thread. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2306</td><td>1033</td><td>Could not create thread for patch application. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2307</td><td>1033</td><td>Source file key name is null.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2308</td><td>1033</td><td>Destination file name is null.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2309</td><td>1033</td><td>Attempting to patch file [2] when patch already in progress.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2310</td><td>1033</td><td>Attempting to continue patch when no patch is in progress.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2315</td><td>1033</td><td>Missing path separator: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2318</td><td>1033</td><td>File does not exist: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2319</td><td>1033</td><td>Error setting file attribute: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2320</td><td>1033</td><td>File not writable: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2321</td><td>1033</td><td>Error creating file: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2322</td><td>1033</td><td>User canceled.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2323</td><td>1033</td><td>Invalid file attribute.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2324</td><td>1033</td><td>Could not open file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2325</td><td>1033</td><td>Could not get file time for file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2326</td><td>1033</td><td>Error in FileToDosDateTime.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2327</td><td>1033</td><td>Could not remove directory: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2328</td><td>1033</td><td>Error getting file version info for file: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2329</td><td>1033</td><td>Error deleting file: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2330</td><td>1033</td><td>Error getting file attributes: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2331</td><td>1033</td><td>Error loading library [2] or finding entry point [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2332</td><td>1033</td><td>Error getting file attributes. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2333</td><td>1033</td><td>Error setting file attributes. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2334</td><td>1033</td><td>Error converting file time to local time for file: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2335</td><td>1033</td><td>Path: [2] is not a parent of [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2336</td><td>1033</td><td>Error creating temp file on path: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2337</td><td>1033</td><td>Could not close file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2338</td><td>1033</td><td>Could not update resource for file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2339</td><td>1033</td><td>Could not set file time for file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2340</td><td>1033</td><td>Could not update resource for file: [3], Missing resource.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2341</td><td>1033</td><td>Could not update resource for file: [3], Resource too large.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2342</td><td>1033</td><td>Could not update resource for file: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2343</td><td>1033</td><td>Specified path is empty.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2344</td><td>1033</td><td>Could not find required file IMAGEHLP.DLL to validate file:[2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2345</td><td>1033</td><td>[2]: File does not contain a valid checksum value.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2347</td><td>1033</td><td>User ignore.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2348</td><td>1033</td><td>Error attempting to read from cabinet stream.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2349</td><td>1033</td><td>Copy resumed with different info.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2350</td><td>1033</td><td>FDI server error</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2351</td><td>1033</td><td>File key '[2]' not found in cabinet '[3]'. The installation cannot continue.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2352</td><td>1033</td><td>Could not initialize cabinet file server. The required file 'CABINET.DLL' may be missing.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2353</td><td>1033</td><td>Not a cabinet.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2354</td><td>1033</td><td>Cannot handle cabinet.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2355</td><td>1033</td><td>Corrupt cabinet.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2356</td><td>1033</td><td>Could not locate cabinet in stream: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2357</td><td>1033</td><td>Cannot set attributes.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2358</td><td>1033</td><td>Error determining whether file is in-use: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2359</td><td>1033</td><td>Unable to create the target file - file may be in use.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2360</td><td>1033</td><td>Progress tick.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2361</td><td>1033</td><td>Need next cabinet.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2362</td><td>1033</td><td>Folder not found: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2363</td><td>1033</td><td>Could not enumerate subfolders for folder: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2364</td><td>1033</td><td>Bad enumeration constant in CreateCopier call.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2365</td><td>1033</td><td>Could not BindImage exe file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2366</td><td>1033</td><td>User failure.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2367</td><td>1033</td><td>User abort.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2368</td><td>1033</td><td>Failed to get network resource information. Error [2], network path [3]. Extended error: network provider [5], error code [4], error description [6].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2370</td><td>1033</td><td>Invalid CRC checksum value for [2] file.{ Its header says [3] for checksum, its computed value is [4].}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2371</td><td>1033</td><td>Could not apply patch to file [2]. GetLastError: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2372</td><td>1033</td><td>Patch file [2] is corrupt or of an invalid format. Attempting to patch file [3]. GetLastError: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2373</td><td>1033</td><td>File [2] is not a valid patch file.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2374</td><td>1033</td><td>File [2] is not a valid destination file for patch file [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2375</td><td>1033</td><td>Unknown patching error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2376</td><td>1033</td><td>Cabinet not found.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2379</td><td>1033</td><td>Error opening file for read: [3] GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2380</td><td>1033</td><td>Error opening file for write: [3]. GetLastError: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2381</td><td>1033</td><td>Directory does not exist: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2382</td><td>1033</td><td>Drive not ready: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_24</td><td>1033</td><td>Please insert the disk: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2401</td><td>1033</td><td>64-bit registry operation attempted on 32-bit operating system for key [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2402</td><td>1033</td><td>Out of memory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_25</td><td>1033</td><td>The installer has insufficient privileges to access this directory: [2].  The installation cannot continue.  Log on as an administrator or contact your system administrator.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2501</td><td>1033</td><td>Could not create rollback script enumerator.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2502</td><td>1033</td><td>Called InstallFinalize when no install in progress.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2503</td><td>1033</td><td>Called RunScript when not marked in progress.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_26</td><td>1033</td><td>Error writing to file [2].  Verify that you have access to that directory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2601</td><td>1033</td><td>Invalid value for property [2]: '[3]'</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2602</td><td>1033</td><td>The [2] table entry '[3]' has no associated entry in the Media table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2603</td><td>1033</td><td>Duplicate table name [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2604</td><td>1033</td><td>[2] Property undefined.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2605</td><td>1033</td><td>Could not find server [2] in [3] or [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2606</td><td>1033</td><td>Value of property [2] is not a valid full path: '[3]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2607</td><td>1033</td><td>Media table not found or empty (required for installation of files).</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2608</td><td>1033</td><td>Could not create security descriptor for object. Error: '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2609</td><td>1033</td><td>Attempt to migrate product settings before initialization.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2611</td><td>1033</td><td>The file [2] is marked as compressed, but the associated media entry does not specify a cabinet.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2612</td><td>1033</td><td>Stream not found in '[2]' column. Primary key: '[3]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2613</td><td>1033</td><td>RemoveExistingProducts action sequenced incorrectly.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2614</td><td>1033</td><td>Could not access IStorage object from installation package.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2615</td><td>1033</td><td>Skipped unregistration of Module [2] due to source resolution failure.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2616</td><td>1033</td><td>Companion file [2] parent missing.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2617</td><td>1033</td><td>Shared component [2] not found in Component table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2618</td><td>1033</td><td>Isolated application component [2] not found in Component table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2619</td><td>1033</td><td>Isolated components [2], [3] not part of same feature.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2620</td><td>1033</td><td>Key file of isolated application component [2] not in File table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2621</td><td>1033</td><td>Resource DLL or Resource ID information for shortcut [2] set incorrectly.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27</td><td>1033</td><td>Error reading from file [2].  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2701</td><td>1033</td><td>The depth of a feature exceeds the acceptable tree depth of [2] levels.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2702</td><td>1033</td><td>A Feature table record ([2]) references a non-existent parent in the Attributes field.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2703</td><td>1033</td><td>Property name for root source path not defined: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2704</td><td>1033</td><td>Root directory property undefined: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2705</td><td>1033</td><td>Invalid table: [2]; Could not be linked as tree.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2706</td><td>1033</td><td>Source paths not created. No path exists for entry [2] in Directory table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2707</td><td>1033</td><td>Target paths not created. No path exists for entry [2] in Directory table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2708</td><td>1033</td><td>No entries found in the file table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2709</td><td>1033</td><td>The specified Component name ('[2]') not found in Component table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2710</td><td>1033</td><td>The requested 'Select' state is illegal for this Component.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2711</td><td>1033</td><td>The specified Feature name ('[2]') not found in Feature table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2712</td><td>1033</td><td>Invalid return from modeless dialog: [3], in action [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2713</td><td>1033</td><td>Null value in a non-nullable column ('[2]' in '[3]' column of the '[4]' table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2714</td><td>1033</td><td>Invalid value for default folder name: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2715</td><td>1033</td><td>The specified File key ('[2]') not found in the File table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2716</td><td>1033</td><td>Could not create a random subcomponent name for component '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2717</td><td>1033</td><td>Bad action condition or error calling custom action '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2718</td><td>1033</td><td>Missing package name for product code '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2719</td><td>1033</td><td>Neither UNC nor drive letter path found in source '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2720</td><td>1033</td><td>Error opening source list key. Error: '[2]'</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2721</td><td>1033</td><td>Custom action [2] not found in Binary table stream.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2722</td><td>1033</td><td>Custom action [2] not found in File table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2723</td><td>1033</td><td>Custom action [2] specifies unsupported type.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2724</td><td>1033</td><td>The volume label '[2]' on the media you're running from does not match the label '[3]' given in the Media table. This is allowed only if you have only 1 entry in your Media table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2725</td><td>1033</td><td>Invalid database tables</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2726</td><td>1033</td><td>Action not found: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2727</td><td>1033</td><td>The directory entry '[2]' does not exist in the Directory table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2728</td><td>1033</td><td>Table definition error: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2729</td><td>1033</td><td>Install engine not initialized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2730</td><td>1033</td><td>Bad value in database. Table: '[2]'; Primary key: '[3]'; Column: '[4]'</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2731</td><td>1033</td><td>Selection Manager not initialized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2732</td><td>1033</td><td>Directory Manager not initialized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2733</td><td>1033</td><td>Bad foreign key ('[2]') in '[3]' column of the '[4]' table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2734</td><td>1033</td><td>Invalid reinstall mode character.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2735</td><td>1033</td><td>Custom action '[2]' has caused an unhandled exception and has been stopped. This may be the result of an internal error in the custom action, such as an access violation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2736</td><td>1033</td><td>Generation of custom action temp file failed: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2737</td><td>1033</td><td>Could not access custom action [2], entry [3], library [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2738</td><td>1033</td><td>Could not access VBScript run time for custom action [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2739</td><td>1033</td><td>Could not access JavaScript run time for custom action [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2740</td><td>1033</td><td>Custom action [2] script error [3], [4]: [5] Line [6], Column [7], [8].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2741</td><td>1033</td><td>Configuration information for product [2] is corrupt. Invalid info: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2742</td><td>1033</td><td>Marshaling to Server failed: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2743</td><td>1033</td><td>Could not execute custom action [2], location: [3], command: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2744</td><td>1033</td><td>EXE failed called by custom action [2], location: [3], command: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2745</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected language [4], found language [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2746</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product [4], found product [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2747</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product version &lt; [4], found product version [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2748</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product version &lt;= [4], found product version [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2749</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product version == [4], found product version [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2750</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product version &gt;= [4], found product version [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27502</td><td>1033</td><td>Could not connect to [2] '[3]'. [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27503</td><td>1033</td><td>Error retrieving version string from [2] '[3]'. [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27504</td><td>1033</td><td>SQL version requirements not met: [3]. This installation requires [2] [4] or later.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27505</td><td>1033</td><td>Could not open SQL script file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27506</td><td>1033</td><td>Error executing SQL script [2]. Line [3]. [4]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27507</td><td>1033</td><td>Connection or browsing for database servers requires that MDAC be installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27508</td><td>1033</td><td>Error installing COM+ application [2]. [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27509</td><td>1033</td><td>Error uninstalling COM+ application [2]. [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2751</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected product version &gt; [4], found product version [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27510</td><td>1033</td><td>Error installing COM+ application [2].  Could not load Microsoft(R) .NET class libraries. Registering .NET serviced components requires that Microsoft(R) .NET Framework be installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27511</td><td>1033</td><td>Could not execute SQL script file [2]. Connection not open: [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27512</td><td>1033</td><td>Error beginning transactions for [2] '[3]'. Database [4]. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27513</td><td>1033</td><td>Error committing transactions for [2] '[3]'. Database [4]. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27514</td><td>1033</td><td>This installation requires a Microsoft SQL Server. The specified server '[3]' is a Microsoft SQL Server Desktop Engine or SQL Server Express.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27515</td><td>1033</td><td>Error retrieving schema version from [2] '[3]'. Database: '[4]'. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27516</td><td>1033</td><td>Error writing schema version to [2] '[3]'. Database: '[4]'. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27517</td><td>1033</td><td>This installation requires Administrator privileges for installing COM+ applications. Log on as an administrator and then retry this installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27518</td><td>1033</td><td>The COM+ application "[2]" is configured to run as an NT service; this requires COM+ 1.5 or later on the system. Since your system has COM+ 1.0, this application will not be installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27519</td><td>1033</td><td>Error updating XML file [2]. [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2752</td><td>1033</td><td>Could not open transform [2] stored as child storage of package [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27520</td><td>1033</td><td>Error opening XML file [2]. [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27521</td><td>1033</td><td>This setup requires MSXML 3.0 or higher for configuring XML files. Please make sure that you have version 3.0 or higher.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27522</td><td>1033</td><td>Error creating XML file [2]. [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27523</td><td>1033</td><td>Error loading servers.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27524</td><td>1033</td><td>Error loading NetApi32.DLL. The ISNetApi.dll needs to have NetApi32.DLL properly loaded and requires an NT based operating system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27525</td><td>1033</td><td>Server not found. Verify that the specified server exists. The server name can not be empty.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27526</td><td>1033</td><td>Unspecified error from ISNetApi.dll.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27527</td><td>1033</td><td>The buffer is too small.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27528</td><td>1033</td><td>Access denied. Check administrative rights.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27529</td><td>1033</td><td>Invalid computer.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2753</td><td>1033</td><td>The File '[2]' is not marked for installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27530</td><td>1033</td><td>Unknown error returned from NetAPI. System error: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27531</td><td>1033</td><td>Unhandled exception.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27532</td><td>1033</td><td>Invalid user name for this server or domain.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27533</td><td>1033</td><td>The case-sensitive passwords do not match.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27534</td><td>1033</td><td>The list is empty.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27535</td><td>1033</td><td>Access violation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27536</td><td>1033</td><td>Error getting group.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27537</td><td>1033</td><td>Error adding user to group. Verify that the group exists for this domain or server.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27538</td><td>1033</td><td>Error creating user.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27539</td><td>1033</td><td>ERROR_NETAPI_ERROR_NOT_PRIMARY returned from NetAPI.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2754</td><td>1033</td><td>The File '[2]' is not a valid patch file.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27540</td><td>1033</td><td>The specified user already exists.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27541</td><td>1033</td><td>The specified group already exists.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27542</td><td>1033</td><td>Invalid password. Verify that the password is in accordance with your network password policy.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27543</td><td>1033</td><td>Invalid name.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27544</td><td>1033</td><td>Invalid group.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27545</td><td>1033</td><td>The user name can not be empty and must be in the format DOMAIN\Username.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27546</td><td>1033</td><td>Error loading or creating INI file in the user TEMP directory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27547</td><td>1033</td><td>ISNetAPI.dll is not loaded or there was an error loading the dll. This dll needs to be loaded for this operation. Verify that the dll is in the SUPPORTDIR directory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27548</td><td>1033</td><td>Error deleting INI file containing new user information from the user's TEMP directory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27549</td><td>1033</td><td>Error getting the primary domain controller (PDC).</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2755</td><td>1033</td><td>Server returned unexpected error [2] attempting to install package [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27550</td><td>1033</td><td>Every field must have a value in order to create a user.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27551</td><td>1033</td><td>ODBC driver for [2] not found. This is required to connect to [2] database servers.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27552</td><td>1033</td><td>Error creating database [4]. Server: [2] [3]. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27553</td><td>1033</td><td>Error connecting to database [4]. Server: [2] [3]. [5]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27554</td><td>1033</td><td>Error attempting to open connection [2]. No valid database metadata associated with this connection.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_27555</td><td>1033</td><td>Error attempting to apply permissions to object '[2]'. System error: [3] ([4])</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2756</td><td>1033</td><td>The property '[2]' was used as a directory property in one or more tables, but no value was ever assigned.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2757</td><td>1033</td><td>Could not create summary info for transform [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2758</td><td>1033</td><td>Transform [2] does not contain an MSI version.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2759</td><td>1033</td><td>Transform [2] version [3] incompatible with engine; Min: [4], Max: [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2760</td><td>1033</td><td>Transform [2] invalid for package [3]. Expected upgrade code [4], found [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2761</td><td>1033</td><td>Cannot begin transaction. Global mutex not properly initialized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2762</td><td>1033</td><td>Cannot write script record. Transaction not started.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2763</td><td>1033</td><td>Cannot run script. Transaction not started.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2765</td><td>1033</td><td>Assembly name missing from AssemblyName table : Component: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2766</td><td>1033</td><td>The file [2] is an invalid MSI storage file.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2767</td><td>1033</td><td>No more data{ while enumerating [2]}.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2768</td><td>1033</td><td>Transform in patch package is invalid.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2769</td><td>1033</td><td>Custom Action [2] did not close [3] MSIHANDLEs.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2770</td><td>1033</td><td>Cached folder [2] not defined in internal cache folder table.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2771</td><td>1033</td><td>Upgrade of feature [2] has a missing component.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2772</td><td>1033</td><td>New upgrade feature [2] must be a leaf feature.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_28</td><td>1033</td><td>Another application has exclusive access to the file [2].  Please shut down all other applications, then click Retry.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2801</td><td>1033</td><td>Unknown Message -- Type [2]. No action is taken.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2802</td><td>1033</td><td>No publisher is found for the event [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2803</td><td>1033</td><td>Dialog View did not find a record for the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2804</td><td>1033</td><td>On activation of the control [3] on dialog [2] CMsiDialog failed to evaluate the condition [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2806</td><td>1033</td><td>The dialog [2] failed to evaluate the condition [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2807</td><td>1033</td><td>The action [2] is not recognized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2808</td><td>1033</td><td>Default button is ill-defined on dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2809</td><td>1033</td><td>On the dialog [2] the next control pointers do not form a cycle. There is a pointer from [3] to [4], but there is no further pointer.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2810</td><td>1033</td><td>On the dialog [2] the next control pointers do not form a cycle. There is a pointer from both [3] and [5] to [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2811</td><td>1033</td><td>On dialog [2] control [3] has to take focus, but it is unable to do so.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2812</td><td>1033</td><td>The event [2] is not recognized.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2813</td><td>1033</td><td>The EndDialog event was called with the argument [2], but the dialog has a parent.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2814</td><td>1033</td><td>On the dialog [2] the control [3] names a nonexistent control [4] as the next control.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2815</td><td>1033</td><td>ControlCondition table has a row without condition for the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2816</td><td>1033</td><td>The EventMapping table refers to an invalid control [4] on dialog [2] for the event [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2817</td><td>1033</td><td>The event [2] failed to set the attribute for the control [4] on dialog [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2818</td><td>1033</td><td>In the ControlEvent table EndDialog has an unrecognized argument [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2819</td><td>1033</td><td>Control [3] on dialog [2] needs a property linked to it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2820</td><td>1033</td><td>Attempted to initialize an already initialized handler.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2821</td><td>1033</td><td>Attempted to initialize an already initialized dialog: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2822</td><td>1033</td><td>No other method can be called on dialog [2] until all the controls are added.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2823</td><td>1033</td><td>Attempted to initialize an already initialized control: [3] on dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2824</td><td>1033</td><td>The dialog attribute [3] needs a record of at least [2] field(s).</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2825</td><td>1033</td><td>The control attribute [3] needs a record of at least [2] field(s).</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2826</td><td>1033</td><td>Control [3] on dialog [2] extends beyond the boundaries of the dialog [4] by [5] pixels.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2827</td><td>1033</td><td>The button [4] on the radio button group [3] on dialog [2] extends beyond the boundaries of the group [5] by [6] pixels.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2828</td><td>1033</td><td>Tried to remove control [3] from dialog [2], but the control is not part of the dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2829</td><td>1033</td><td>Attempt to use an uninitialized dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2830</td><td>1033</td><td>Attempt to use an uninitialized control on dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2831</td><td>1033</td><td>The control [3] on dialog [2] does not support [5] the attribute [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2832</td><td>1033</td><td>The dialog [2] does not support the attribute [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2833</td><td>1033</td><td>Control [4] on dialog [3] ignored the message [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2834</td><td>1033</td><td>The next pointers on the dialog [2] do not form a single loop.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2835</td><td>1033</td><td>The control [2] was not found on dialog [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2836</td><td>1033</td><td>The control [3] on the dialog [2] cannot take focus.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2837</td><td>1033</td><td>The control [3] on dialog [2] wants the winproc to return [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2838</td><td>1033</td><td>The item [2] in the selection table has itself as a parent.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2839</td><td>1033</td><td>Setting the property [2] failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2840</td><td>1033</td><td>Error dialog name mismatch.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2841</td><td>1033</td><td>No OK button was found on the error dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2842</td><td>1033</td><td>No text field was found on the error dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2843</td><td>1033</td><td>The ErrorString attribute is not supported for standard dialogs.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2844</td><td>1033</td><td>Cannot execute an error dialog if the Errorstring is not set.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2845</td><td>1033</td><td>The total width of the buttons exceeds the size of the error dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2846</td><td>1033</td><td>SetFocus did not find the required control on the error dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2847</td><td>1033</td><td>The control [3] on dialog [2] has both the icon and the bitmap style set.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2848</td><td>1033</td><td>Tried to set control [3] as the default button on dialog [2], but the control does not exist.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2849</td><td>1033</td><td>The control [3] on dialog [2] is of a type, that cannot be integer valued.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2850</td><td>1033</td><td>Unrecognized volume type.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2851</td><td>1033</td><td>The data for the icon [2] is not valid.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2852</td><td>1033</td><td>At least one control has to be added to dialog [2] before it is used.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2853</td><td>1033</td><td>Dialog [2] is a modeless dialog. The execute method should not be called on it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2854</td><td>1033</td><td>On the dialog [2] the control [3] is designated as first active control, but there is no such control.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2855</td><td>1033</td><td>The radio button group [3] on dialog [2] has fewer than 2 buttons.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2856</td><td>1033</td><td>Creating a second copy of the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2857</td><td>1033</td><td>The directory [2] is mentioned in the selection table but not found.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2858</td><td>1033</td><td>The data for the bitmap [2] is not valid.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2859</td><td>1033</td><td>Test error message.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2860</td><td>1033</td><td>Cancel button is ill-defined on dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2861</td><td>1033</td><td>The next pointers for the radio buttons on dialog [2] control [3] do not form a cycle.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2862</td><td>1033</td><td>The attributes for the control [3] on dialog [2] do not define a valid icon size. Setting the size to 16.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2863</td><td>1033</td><td>The control [3] on dialog [2] needs the icon [4] in size [5]x[5], but that size is not available. Loading the first available size.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2864</td><td>1033</td><td>The control [3] on dialog [2] received a browse event, but there is no configurable directory for the present selection. Likely cause: browse button is not authored correctly.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2865</td><td>1033</td><td>Control [3] on billboard [2] extends beyond the boundaries of the billboard [4] by [5] pixels.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2866</td><td>1033</td><td>The dialog [2] is not allowed to return the argument [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2867</td><td>1033</td><td>The error dialog property is not set.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2868</td><td>1033</td><td>The error dialog [2] does not have the error style bit set.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2869</td><td>1033</td><td>The dialog [2] has the error style bit set, but is not an error dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2870</td><td>1033</td><td>The help string [4] for control [3] on dialog [2] does not contain the separator character.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2871</td><td>1033</td><td>The [2] table is out of date: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2872</td><td>1033</td><td>The argument of the CheckPath control event on dialog [2] is invalid.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2873</td><td>1033</td><td>On the dialog [2] the control [3] has an invalid string length limit: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2874</td><td>1033</td><td>Changing the text font to [2] failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2875</td><td>1033</td><td>Changing the text color to [2] failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2876</td><td>1033</td><td>The control [3] on dialog [2] had to truncate the string: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2877</td><td>1033</td><td>The binary data [2] was not found</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2878</td><td>1033</td><td>On the dialog [2] the control [3] has a possible value: [4]. This is an invalid or duplicate value.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2879</td><td>1033</td><td>The control [3] on dialog [2] cannot parse the mask string: [4].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2880</td><td>1033</td><td>Do not perform the remaining control events.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2881</td><td>1033</td><td>CMsiHandler initialization failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2882</td><td>1033</td><td>Dialog window class registration failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2883</td><td>1033</td><td>CreateNewDialog failed for the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2884</td><td>1033</td><td>Failed to create a window for the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2885</td><td>1033</td><td>Failed to create the control [3] on the dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2886</td><td>1033</td><td>Creating the [2] table failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2887</td><td>1033</td><td>Creating a cursor to the [2] table failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2888</td><td>1033</td><td>Executing the [2] view failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2889</td><td>1033</td><td>Creating the window for the control [3] on dialog [2] failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2890</td><td>1033</td><td>The handler failed in creating an initialized dialog.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2891</td><td>1033</td><td>Failed to destroy window for dialog [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2892</td><td>1033</td><td>[2] is an integer only control, [3] is not a valid integer value.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2893</td><td>1033</td><td>The control [3] on dialog [2] can accept property values that are at most [5] characters long. The value [4] exceeds this limit, and has been truncated.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2894</td><td>1033</td><td>Loading RICHED20.DLL failed. GetLastError() returned: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2895</td><td>1033</td><td>Freeing RICHED20.DLL failed. GetLastError() returned: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2896</td><td>1033</td><td>Executing action [2] failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2897</td><td>1033</td><td>Failed to create any [2] font on this system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2898</td><td>1033</td><td>For [2] textstyle, the system created a '[3]' font, in [4] character set.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2899</td><td>1033</td><td>Failed to create [2] textstyle. GetLastError() returned: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_29</td><td>1033</td><td>There is not enough disk space to install the file [2].  Free some disk space and click Retry, or click Cancel to exit.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2901</td><td>1033</td><td>Invalid parameter to operation [2]: Parameter [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2902</td><td>1033</td><td>Operation [2] called out of sequence.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2903</td><td>1033</td><td>The file [2] is missing.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2904</td><td>1033</td><td>Could not BindImage file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2905</td><td>1033</td><td>Could not read record from script file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2906</td><td>1033</td><td>Missing header in script file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2907</td><td>1033</td><td>Could not create secure security descriptor. Error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2908</td><td>1033</td><td>Could not register component [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2909</td><td>1033</td><td>Could not unregister component [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2910</td><td>1033</td><td>Could not determine user's security ID.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2911</td><td>1033</td><td>Could not remove the folder [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2912</td><td>1033</td><td>Could not schedule file [2] for removal on restart.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2919</td><td>1033</td><td>No cabinet specified for compressed file: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2920</td><td>1033</td><td>Source directory not specified for file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2924</td><td>1033</td><td>Script [2] version unsupported. Script version: [3], minimum version: [4], maximum version: [5].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2927</td><td>1033</td><td>ShellFolder id [2] is invalid.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2928</td><td>1033</td><td>Exceeded maximum number of sources. Skipping source '[2]'.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2929</td><td>1033</td><td>Could not determine publishing root. Error: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2932</td><td>1033</td><td>Could not create file [2] from script data. Error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2933</td><td>1033</td><td>Could not initialize rollback script [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2934</td><td>1033</td><td>Could not secure transform [2]. Error [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2935</td><td>1033</td><td>Could not unsecure transform [2]. Error [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2936</td><td>1033</td><td>Could not find transform [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2937</td><td>1033</td><td>Windows Installer cannot install a system file protection catalog. Catalog: [2], Error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2938</td><td>1033</td><td>Windows Installer cannot retrieve a system file protection catalog from the cache. Catalog: [2], Error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2939</td><td>1033</td><td>Windows Installer cannot delete a system file protection catalog from the cache. Catalog: [2], Error: [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2940</td><td>1033</td><td>Directory Manager not supplied for source resolution.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2941</td><td>1033</td><td>Unable to compute the CRC for file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2942</td><td>1033</td><td>BindImage action has not been executed on [2] file.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2943</td><td>1033</td><td>This version of Windows does not support deploying 64-bit packages. The script [2] is for a 64-bit package.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2944</td><td>1033</td><td>GetProductAssignmentType failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_2945</td><td>1033</td><td>Installation of ComPlus App [2] failed with error [3].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_3</td><td>1033</td><td>Info [1]. </td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_30</td><td>1033</td><td>Source file not found: [2].  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_3001</td><td>1033</td><td>The patches in this list contain incorrect sequencing information: [2][3][4][5][6][7][8][9][10][11][12][13][14][15][16].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_3002</td><td>1033</td><td>Patch [2] contains invalid sequencing information. </td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_31</td><td>1033</td><td>Error reading from file: [3]. {{ System error [2].}}  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_32</td><td>1033</td><td>Error writing to file: [3]. {{ System error [2].}}  Verify that you have access to that directory.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_33</td><td>1033</td><td>Source file not found{{(cabinet)}}: [2].  Verify that the file exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_34</td><td>1033</td><td>Cannot create the directory [2].  A file with this name already exists.  Please rename or remove the file and click Retry, or click Cancel to exit.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_35</td><td>1033</td><td>The volume [2] is currently unavailable.  Please select another.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_36</td><td>1033</td><td>The specified path [2] is unavailable.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_37</td><td>1033</td><td>Unable to write to the specified folder [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_38</td><td>1033</td><td>A network error occurred while attempting to read from the file [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_39</td><td>1033</td><td>An error occurred while attempting to create the directory [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_4</td><td>1033</td><td>Internal Error [1]. [2]{, [3]}{, [4]}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_40</td><td>1033</td><td>A network error occurred while attempting to create the directory [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_41</td><td>1033</td><td>A network error occurred while attempting to open the source file cabinet [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_42</td><td>1033</td><td>The specified path is too long [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_43</td><td>1033</td><td>The Installer has insufficient privileges to modify the file [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_44</td><td>1033</td><td>A portion of the path [2] exceeds the length allowed by the system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_45</td><td>1033</td><td>The path [2] contains words that are not valid in folders.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_46</td><td>1033</td><td>The path [2] contains an invalid character.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_47</td><td>1033</td><td>[2] is not a valid short file name.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_48</td><td>1033</td><td>Error getting file security: [3] GetLastError: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_49</td><td>1033</td><td>Invalid Drive: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_5</td><td>1033</td><td>{{Disk full: }}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_50</td><td>1033</td><td>Could not create key [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_51</td><td>1033</td><td>Could not open key: [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_52</td><td>1033</td><td>Could not delete value [2] from key [3]. {{ System error [4].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_53</td><td>1033</td><td>Could not delete key [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_54</td><td>1033</td><td>Could not read value [2] from key [3]. {{ System error [4].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_55</td><td>1033</td><td>Could not write value [2] to key [3]. {{ System error [4].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_56</td><td>1033</td><td>Could not get value names for key [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_57</td><td>1033</td><td>Could not get sub key names for key [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_58</td><td>1033</td><td>Could not read security information for key [2]. {{ System error [3].}}  Verify that you have sufficient access to that key, or contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_59</td><td>1033</td><td>Could not increase the available registry space. [2] KB of free registry space is required for the installation of this application.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_6</td><td>1033</td><td>Action [Time]: [1]. [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_60</td><td>1033</td><td>Another installation is in progress. You must complete that installation before continuing this one.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_61</td><td>1033</td><td>Error accessing secured data. Please make sure the Windows Installer is configured properly and try the installation again.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_62</td><td>1033</td><td>User [2] has previously initiated an installation for product [3].  That user will need to run that installation again before using that product.  Your current installation will now continue.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_63</td><td>1033</td><td>User [2] has previously initiated an installation for product [3].  That user will need to run that installation again before using that product.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_64</td><td>1033</td><td>Out of disk space -- Volume: '[2]'; required space: [3] KB; available space: [4] KB.  Free some disk space and retry.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_65</td><td>1033</td><td>Are you sure you want to cancel?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_66</td><td>1033</td><td>The file [2][3] is being held in use{ by the following process: Name: [4], ID: [5], Window Title: [6]}.  Close that application and retry.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_67</td><td>1033</td><td>The product [2] is already installed, preventing the installation of this product.  The two products are incompatible.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_68</td><td>1033</td><td>Out of disk space -- Volume: [2]; required space: [3] KB; available space: [4] KB.  If rollback is disabled, enough space is available. Click Cancel to quit, Retry to check available disk space again, or Ignore to continue without rollback.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_69</td><td>1033</td><td>Could not access network location [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_7</td><td>1033</td><td>[ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_70</td><td>1033</td><td>The following applications should be closed before continuing the installation:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_71</td><td>1033</td><td>Could not find any previously installed compliant products on the machine for installing this product.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_72</td><td>1033</td><td>The key [2] is not valid.  Verify that you entered the correct key.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_73</td><td>1033</td><td>The installer must restart your system before configuration of [2] can continue.  Click Yes to restart now or No if you plan to restart later.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_74</td><td>1033</td><td>You must restart your system for the configuration changes made to [2] to take effect. Click Yes to restart now or No if you plan to restart later.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_75</td><td>1033</td><td>An installation for [2] is currently suspended.  You must undo the changes made by that installation to continue.  Do you want to undo those changes?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_76</td><td>1033</td><td>A previous installation for this product is in progress.  You must undo the changes made by that installation to continue.  Do you want to undo those changes?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_77</td><td>1033</td><td>No valid source could be found for product [2].  The Windows Installer cannot continue.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_78</td><td>1033</td><td>Installation operation completed successfully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_79</td><td>1033</td><td>Installation operation failed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_8</td><td>1033</td><td>{[2]}{, [3]}{, [4]}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_80</td><td>1033</td><td>Product: [2] -- [3]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_81</td><td>1033</td><td>You may either restore your computer to its previous state or continue the installation later. Would you like to restore?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_82</td><td>1033</td><td>An error occurred while writing installation information to disk.  Check to make sure enough disk space is available, and click Retry, or Cancel to end the installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_83</td><td>1033</td><td>One or more of the files required to restore your computer to its previous state could not be found.  Restoration will not be possible.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_84</td><td>1033</td><td>The path [2] is not valid.  Please specify a valid path.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_85</td><td>1033</td><td>Out of memory. Shut down other applications before retrying.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_86</td><td>1033</td><td>There is no disk in drive [2]. Please insert one and click Retry, or click Cancel to go back to the previously selected volume.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_87</td><td>1033</td><td>There is no disk in drive [2]. Please insert one and click Retry, or click Cancel to return to the browse dialog and select a different volume.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_88</td><td>1033</td><td>The folder [2] does not exist.  Please enter a path to an existing folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_89</td><td>1033</td><td>You have insufficient privileges to read this folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_9</td><td>1033</td><td>Message type: [1], Argument: [2]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_90</td><td>1033</td><td>A valid destination folder for the installation could not be determined.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_91</td><td>1033</td><td>Error attempting to read from the source installation database: [2].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_92</td><td>1033</td><td>Scheduling reboot operation: Renaming file [2] to [3]. Must reboot to complete operation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_93</td><td>1033</td><td>Scheduling reboot operation: Deleting file [2]. Must reboot to complete operation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_94</td><td>1033</td><td>Module [2] failed to register.  HRESULT [3].  Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_95</td><td>1033</td><td>Module [2] failed to unregister.  HRESULT [3].  Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_96</td><td>1033</td><td>Failed to cache package [2]. Error: [3]. Contact your support personnel.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_97</td><td>1033</td><td>Could not register font [2].  Verify that you have sufficient permissions to install fonts, and that the system supports this font.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_98</td><td>1033</td><td>Could not unregister font [2]. Verify that you have sufficient permissions to remove fonts.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ERROR_99</td><td>1033</td><td>Could not create shortcut [2]. Verify that the destination folder exists and that you can access it.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_INSTALLDIR</td><td>1033</td><td>[INSTALLDIR]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_INSTALLSHIELD</td><td>1033</td><td>InstallShield</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_INSTALLSHIELD_FORMATTED</td><td>1033</td><td>{&amp;MSSWhiteSerif8}InstallShield</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ISSCRIPT_VERSION_MISSING</td><td>1033</td><td>The InstallScript engine is missing from this machine.  If available, please run ISScript.msi, or contact your support personnel for further assistance.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_ISSCRIPT_VERSION_OLD</td><td>1033</td><td>The InstallScript engine on this machine is older than the version required to run this setup.  If available, please install the latest version of ISScript.msi, or contact your support personnel for further assistance.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_NEXT</td><td>1033</td><td>&amp;Next &gt;</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_OK</td><td>1033</td><td>OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PREREQUISITE_SETUP_BROWSE</td><td>1033</td><td>Open [ProductName]'s original [SETUPEXENAME]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PREREQUISITE_SETUP_INVALID</td><td>1033</td><td>This executable file does not appear to be the original executable file for [ProductName]. Without using the original [SETUPEXENAME] to install additional dependencies, [ProductName] may not work correctly. Would you like to find the original [SETUPEXENAME]?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PREREQUISITE_SETUP_SEARCH</td><td>1033</td><td>This installation may require additional dependencies. Without its dependencies, [ProductName] may not work correctly. Would you like to find the original [SETUPEXENAME]?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PREVENT_DOWNGRADE_EXIT</td><td>1033</td><td>A newer version of this application is already installed on this computer. If you wish to install this version, please uninstall the newer version first. Click OK to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PRINT_BUTTON</td><td>1033</td><td>&amp;Print</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PRODUCTNAME_INSTALLSHIELD</td><td>1033</td><td>[ProductName] - InstallShield Wizard</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEAPPPOOL</td><td>1033</td><td>Creating application pool %s</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEAPPPOOLS</td><td>1033</td><td>Creating application Pools...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEVROOT</td><td>1033</td><td>Creating IIS virtual directory %s</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEVROOTS</td><td>1033</td><td>Creating IIS virtual directories...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEWEBSERVICEEXTENSION</td><td>1033</td><td>Creating web service extension</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEWEBSERVICEEXTENSIONS</td><td>1033</td><td>Creating web service extensions...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEWEBSITE</td><td>1033</td><td>Creating IIS website %s</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_CREATEWEBSITES</td><td>1033</td><td>Creating IIS websites...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_EXTRACT</td><td>1033</td><td>Extracting information for IIS virtual directories...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_EXTRACTDONE</td><td>1033</td><td>Extracted information for IIS virtual directories...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEAPPPOOL</td><td>1033</td><td>Removing application pool</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEAPPPOOLS</td><td>1033</td><td>Removing application pools...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVESITE</td><td>1033</td><td>Removing web site at port %d</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEVROOT</td><td>1033</td><td>Removing IIS virtual directory %s</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEVROOTS</td><td>1033</td><td>Removing IIS virtual directories...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEWEBSERVICEEXTENSION</td><td>1033</td><td>Removing web service extension</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEWEBSERVICEEXTENSIONS</td><td>1033</td><td>Removing web service extensions...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_REMOVEWEBSITES</td><td>1033</td><td>Removing IIS websites...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_ROLLBACKAPPPOOLS</td><td>1033</td><td>Rolling back application pools...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_ROLLBACKVROOTS</td><td>1033</td><td>Rolling back virtual directory and web site changes...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_IIS_ROLLBACKWEBSERVICEEXTENSIONS</td><td>1033</td><td>Rolling back web service extensions...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_TEXTFILECHANGS_REPLACE</td><td>1033</td><td>Replacing %s with %s in %s...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_COSTING</td><td>1033</td><td>Costing XML files...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_CREATE_FILE</td><td>1033</td><td>Creating XML file %s...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_FILES</td><td>1033</td><td>Performing XML file changes...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_REMOVE_FILE</td><td>1033</td><td>Removing XML file %s...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_ROLLBACK_FILES</td><td>1033</td><td>Rolling back XML file changes...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_PROGMSG_XML_UPDATE_FILE</td><td>1033</td><td>Updating XML file %s...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SETUPEXE_EXPIRE_MSG</td><td>1033</td><td>This setup works until %s. The setup will now exit.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SETUPEXE_LAUNCH_COND_E</td><td>1033</td><td>This setup was built with an evaluation version of InstallShield and can only be launched from setup.exe.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SHORTCUT_DISPLAY_NAME1</td><td>1033</td><td>LAUNCH~1.EXE|Launch Win10-Startup.exe</td><td>0</td><td/><td>447081869</td></row>
		<row><td>IDS_SHORTCUT_DISPLAY_NAME2</td><td>1033</td><td>LAUNCH~1.EXE|Launch Win10-Startup.exe</td><td>0</td><td/><td>447072878</td></row>
		<row><td>IDS_SHORTCUT_DISPLAY_NAME3</td><td>1033</td><td>LAUNCH~1.EXE|Launch Win10-Install.exe</td><td>0</td><td/><td>1655001774</td></row>
		<row><td>IDS_SQLBROWSE_INTRO</td><td>1033</td><td>From the list of servers below, select the database server you would like to target.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLBROWSE_INTRO_DB</td><td>1033</td><td>From the list of catalog names below, select the database catalog you would like to target.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLBROWSE_INTRO_TEMPLATE</td><td>1033</td><td>[IS_SQLBROWSE_INTRO]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_BROWSE</td><td>1033</td><td>B&amp;rowse...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_BROWSE_DB</td><td>1033</td><td>Br&amp;owse...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_CATALOG</td><td>1033</td><td>&amp;Name of database catalog:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_CONNECT</td><td>1033</td><td>Connect using:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_DESC</td><td>1033</td><td>Select database server and authentication method</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_ID</td><td>1033</td><td>&amp;Login ID:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_INTRO</td><td>1033</td><td>Select the database server to install to from the list below or click Browse to see a list of all database servers. You can also specify the way to authenticate your login using your current credentials or a SQL Login ID and Password.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_PSWD</td><td>1033</td><td>&amp;Password:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_SERVER</td><td>1033</td><td>&amp;Database Server:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_SERVER2</td><td>1033</td><td>&amp;Database server that you are installing to:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_SQL</td><td>1033</td><td>S&amp;erver authentication using the Login ID and password below</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_TITLE</td><td>1033</td><td>{&amp;MSSansBold8}Database Server</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLLOGIN_WIN</td><td>1033</td><td>&amp;Windows authentication credentials of current user</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLSCRIPT_INSTALLING</td><td>1033</td><td>Executing SQL Install Script...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SQLSCRIPT_UNINSTALLING</td><td>1033</td><td>Executing SQL Uninstall Script...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_STANDARD_USE_SETUPEXE</td><td>1033</td><td>This installation cannot be run by directly launching the MSI package. You must run setup.exe.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_Advertise</td><td>1033</td><td>Will be installed on first use. (Available only if the feature supports this option.)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_AllInstalledLocal</td><td>1033</td><td>Will be completely installed to the local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_CustomSetup</td><td>1033</td><td>{&amp;MSSansBold8}Custom Setup Tips</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_CustomSetupDescription</td><td>1033</td><td>Custom Setup allows you to selectively install program features.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_IconInstallState</td><td>1033</td><td>The icon next to the feature name indicates the install state of the feature. Click the icon to drop down the install state menu for each feature.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_InstallState</td><td>1033</td><td>This install state means the feature...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_Network</td><td>1033</td><td>Will be installed to run from the network. (Available only if the feature supports this option.)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_OK</td><td>1033</td><td>OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_SubFeaturesInstalledLocal</td><td>1033</td><td>Will have some subfeatures installed to the local hard drive. (Available only if the feature has subfeatures.)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_SetupTips_WillNotBeInstalled</td><td>1033</td><td>Will not be installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Available</td><td>1033</td><td>Available</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Bytes</td><td>1033</td><td>bytes</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_CompilingFeaturesCost</td><td>1033</td><td>Compiling cost for this feature...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Differences</td><td>1033</td><td>Differences</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_DiskSize</td><td>1033</td><td>Disk Size</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureCompletelyRemoved</td><td>1033</td><td>This feature will be completely removed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureContinueNetwork</td><td>1033</td><td>This feature will continue to be run from the network</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureFreeSpace</td><td>1033</td><td>This feature frees up [1] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledCD</td><td>1033</td><td>This feature, and all subfeatures, will be installed to run from the CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledCD2</td><td>1033</td><td>This feature will be installed to run from CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledLocal</td><td>1033</td><td>This feature, and all subfeatures, will be installed on local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledLocal2</td><td>1033</td><td>This feature will be installed on local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledNetwork</td><td>1033</td><td>This feature, and all subfeatures, will be installed to run from the network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledNetwork2</td><td>1033</td><td>This feature will be installed to run from network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledRequired</td><td>1033</td><td>Will be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledWhenRequired</td><td>1033</td><td>This feature will be set to be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureInstalledWhenRequired2</td><td>1033</td><td>This feature will be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureLocal</td><td>1033</td><td>This feature will be installed on the local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureLocal2</td><td>1033</td><td>This feature will be installed on your local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureNetwork</td><td>1033</td><td>This feature will be installed to run from the network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureNetwork2</td><td>1033</td><td>This feature will be available to run from the network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureNotAvailable</td><td>1033</td><td>This feature will not be available.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureOnCD</td><td>1033</td><td>This feature will be installed to run from CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureOnCD2</td><td>1033</td><td>This feature will be available to run from CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRemainLocal</td><td>1033</td><td>This feature will remain on your local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRemoveNetwork</td><td>1033</td><td>This feature will be removed from your local hard drive, but will be still available to run from the network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRemovedCD</td><td>1033</td><td>This feature will be removed from your local hard drive but will still be available to run from CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRemovedUnlessRequired</td><td>1033</td><td>This feature will be removed from your local hard drive but will be set to be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRequiredSpace</td><td>1033</td><td>This feature requires [1] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureRunFromCD</td><td>1033</td><td>This feature will continue to be run from the CD</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureSpaceFree</td><td>1033</td><td>This feature frees up [1] on your hard drive. It has [2] of [3] subfeatures selected. The subfeatures free up [4] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureSpaceFree2</td><td>1033</td><td>This feature frees up [1] on your hard drive. It has [2] of [3] subfeatures selected. The subfeatures require [4] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureSpaceFree3</td><td>1033</td><td>This feature requires [1] on your hard drive. It has [2] of [3] subfeatures selected. The subfeatures free up [4] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureSpaceFree4</td><td>1033</td><td>This feature requires [1] on your hard drive. It has [2] of [3] subfeatures selected. The subfeatures require [4] on your hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureUnavailable</td><td>1033</td><td>This feature will become unavailable.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureUninstallNoNetwork</td><td>1033</td><td>This feature will be uninstalled completely, and you won't be able to run it from the network.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureWasCD</td><td>1033</td><td>This feature was run from the CD but will be set to be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureWasCDLocal</td><td>1033</td><td>This feature was run from the CD but will be installed on the local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureWasOnNetworkInstalled</td><td>1033</td><td>This feature was run from the network but will be installed when required.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureWasOnNetworkLocal</td><td>1033</td><td>This feature was run from the network but will be installed on the local hard drive.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_FeatureWillBeUninstalled</td><td>1033</td><td>This feature will be uninstalled completely, and you won't be able to run it from CD.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Folder</td><td>1033</td><td>Fldr|New Folder</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_GB</td><td>1033</td><td>GB</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_KB</td><td>1033</td><td>KB</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_MB</td><td>1033</td><td>MB</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Required</td><td>1033</td><td>Required</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_TimeRemaining</td><td>1033</td><td>Time remaining: {[1] min }{[2] sec}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS_UITEXT_Volume</td><td>1033</td><td>Volume</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__AgreeToLicense_0</td><td>1033</td><td>I &amp;do not accept the terms in the license agreement</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__AgreeToLicense_1</td><td>1033</td><td>I &amp;accept the terms in the license agreement</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DatabaseFolder_ChangeFolder</td><td>1033</td><td>Click Next to install to this folder, or click Change to install to a different folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DatabaseFolder_DatabaseDir</td><td>1033</td><td>[DATABASEDIR]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DatabaseFolder_DatabaseFolder</td><td>1033</td><td>{&amp;MSSansBold8}Database Folder</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DestinationFolder_Change</td><td>1033</td><td>&amp;Change...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DestinationFolder_ChangeFolder</td><td>1033</td><td>Click Next to install to this folder, or click Change to install to a different folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DestinationFolder_DestinationFolder</td><td>1033</td><td>{&amp;MSSansBold8}Destination Folder</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DestinationFolder_InstallTo</td><td>1033</td><td>Install [ProductName] to:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DisplayName_Custom</td><td>1033</td><td>Custom</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DisplayName_Minimal</td><td>1033</td><td>Minimal</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__DisplayName_Typical</td><td>1033</td><td>Typical</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_11</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_4</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_8</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_BrowseDestination</td><td>1033</td><td>Browse to the destination folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_ChangeDestination</td><td>1033</td><td>{&amp;MSSansBold8}Change Current Destination Folder</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_CreateFolder</td><td>1033</td><td>Create new folder|</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_FolderName</td><td>1033</td><td>&amp;Folder name:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_LookIn</td><td>1033</td><td>&amp;Look in:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallBrowse_UpOneLevel</td><td>1033</td><td>Up one level|</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPointWelcome_ServerImage</td><td>1033</td><td>The InstallShield(R) Wizard will create a server image of [ProductName] at a specified network location. To continue, click Next.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPointWelcome_Wizard</td><td>1033</td><td>{&amp;TahomaBold10}Welcome to the InstallShield Wizard for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_Change</td><td>1033</td><td>&amp;Change...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_EnterNetworkLocation</td><td>1033</td><td>Enter the network location or click Change to browse to a location.  Click Install to create a server image of [ProductName] at the specified network location or click Cancel to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_Install</td><td>1033</td><td>&amp;Install</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_NetworkLocation</td><td>1033</td><td>&amp;Network location:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_NetworkLocationFormatted</td><td>1033</td><td>{&amp;MSSansBold8}Network Location</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsAdminInstallPoint_SpecifyNetworkLocation</td><td>1033</td><td>Specify a network location for the server image of the product.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseButton</td><td>1033</td><td>&amp;Browse...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_11</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_4</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_8</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_BrowseDestFolder</td><td>1033</td><td>Browse to the destination folder.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_ChangeCurrentFolder</td><td>1033</td><td>{&amp;MSSansBold8}Change Current Destination Folder</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_CreateFolder</td><td>1033</td><td>Create New Folder|</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_FolderName</td><td>1033</td><td>&amp;Folder name:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_LookIn</td><td>1033</td><td>&amp;Look in:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_OK</td><td>1033</td><td>OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseFolderDlg_UpOneLevel</td><td>1033</td><td>Up One Level|</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseForAccount</td><td>1033</td><td>Browse for a User Account</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseGroup</td><td>1033</td><td>Select a Group</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsBrowseUsernameTitle</td><td>1033</td><td>Select a User Name</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCancelDlg_ConfirmCancel</td><td>1033</td><td>Are you sure you want to cancel [ProductName] installation?</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCancelDlg_No</td><td>1033</td><td>&amp;No</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCancelDlg_Yes</td><td>1033</td><td>&amp;Yes</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsConfirmPassword</td><td>1033</td><td>Con&amp;firm password:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCreateNewUserTitle</td><td>1033</td><td>New User Information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCreateUserBrowse</td><td>1033</td><td>N&amp;ew User Information...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_Change</td><td>1033</td><td>&amp;Change...</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_ClickFeatureIcon</td><td>1033</td><td>Click on an icon in the list below to change how a feature is installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_CustomSetup</td><td>1033</td><td>{&amp;MSSansBold8}Custom Setup</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_FeatureDescription</td><td>1033</td><td>Feature Description</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_FeaturePath</td><td>1033</td><td>&lt;selected feature path&gt;</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_FeatureSize</td><td>1033</td><td>Feature size</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_Help</td><td>1033</td><td>&amp;Help</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_InstallTo</td><td>1033</td><td>Install to:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_MultilineDescription</td><td>1033</td><td>Multiline description of the currently selected item</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_SelectFeatures</td><td>1033</td><td>Select the program features you want installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsCustomSelectionDlg_Space</td><td>1033</td><td>&amp;Space</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDiskSpaceDlg_DiskSpace</td><td>1033</td><td>Disk space required for the installation exceeds available disk space.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDiskSpaceDlg_HighlightedVolumes</td><td>1033</td><td>The highlighted volumes do not have enough disk space available for the currently selected features. You can remove files from the highlighted volumes, choose to install fewer features onto local drives, or select different destination drives.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDiskSpaceDlg_Numbers</td><td>1033</td><td>{120}{70}{70}{70}{70}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDiskSpaceDlg_OK</td><td>1033</td><td>OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDiskSpaceDlg_OutOfDiskSpace</td><td>1033</td><td>{&amp;MSSansBold8}Out of Disk Space</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsDomainOrServer</td><td>1033</td><td>&amp;Domain or server:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_Abort</td><td>1033</td><td>&amp;Abort</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_ErrorText</td><td>1033</td><td>&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;&lt;error text goes here&gt;</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_Ignore</td><td>1033</td><td>&amp;Ignore</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_InstallerInfo</td><td>1033</td><td>[ProductName] Installer Information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_NO</td><td>1033</td><td>&amp;No</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_OK</td><td>1033</td><td>&amp;OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_Retry</td><td>1033</td><td>&amp;Retry</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsErrorDlg_Yes</td><td>1033</td><td>&amp;Yes</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_Finish</td><td>1033</td><td>&amp;Finish</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_InstallSuccess</td><td>1033</td><td>The InstallShield Wizard has successfully installed [ProductName]. Click Finish to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_LaunchProgram</td><td>1033</td><td>Launch the program</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_ShowReadMe</td><td>1033</td><td>Show the readme file</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_UninstallSuccess</td><td>1033</td><td>The InstallShield Wizard has successfully uninstalled [ProductName]. Click Finish to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_Update_InternetConnection</td><td>1033</td><td>Your Internet connection can be used to make sure that you have the latest updates.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_Update_PossibleUpdates</td><td>1033</td><td>Some program files might have been updated since you purchased your copy of [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_Update_SetupFinished</td><td>1033</td><td>Setup has finished installing [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_Update_YesCheckForUpdates</td><td>1033</td><td>&amp;Yes, check for program updates (Recommended) after the setup completes.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsExitDialog_WizardCompleted</td><td>1033</td><td>{&amp;TahomaBold10}InstallShield Wizard Completed</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_ClickFinish</td><td>1033</td><td>Click Finish to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_Finish</td><td>1033</td><td>&amp;Finish</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_KeepOrRestore</td><td>1033</td><td>You can either keep any existing installed elements on your system to continue this installation at a later time or you can restore your system to its original state prior to the installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_NotModified</td><td>1033</td><td>Your system has not been modified. To complete installation at another time, please run setup again.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_RestoreOrContinueLater</td><td>1033</td><td>Click Restore or Continue Later to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_WizardCompleted</td><td>1033</td><td>{&amp;TahomaBold10}InstallShield Wizard Completed</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFatalError_WizardInterrupted</td><td>1033</td><td>The wizard was interrupted before [ProductName] could be completely installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFeatureDetailsDlg_DiskSpaceRequirements</td><td>1033</td><td>{&amp;MSSansBold8}Disk Space Requirements</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFeatureDetailsDlg_Numbers</td><td>1033</td><td>{120}{70}{70}{70}{70}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFeatureDetailsDlg_OK</td><td>1033</td><td>OK</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFeatureDetailsDlg_SpaceRequired</td><td>1033</td><td>The disk space required for the installation of the selected features.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFeatureDetailsDlg_VolumesTooSmall</td><td>1033</td><td>The highlighted volumes do not have enough disk space available for the currently selected features. You can remove files from the highlighted volumes, choose to install fewer features onto local drives, or select different destination drives.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_ApplicationsUsingFiles</td><td>1033</td><td>The following applications are using files that need to be updated by this setup. Close these applications and click Retry to continue.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_Exit</td><td>1033</td><td>&amp;Exit</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_FilesInUse</td><td>1033</td><td>{&amp;MSSansBold8}Files in Use</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_FilesInUseMessage</td><td>1033</td><td>Some files that need to be updated are currently in use.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_Ignore</td><td>1033</td><td>&amp;Ignore</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsFilesInUse_Retry</td><td>1033</td><td>&amp;Retry</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsGroup</td><td>1033</td><td>&amp;Group:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsGroupLabel</td><td>1033</td><td>Gr&amp;oup:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsInitDlg_1</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsInitDlg_2</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsInitDlg_PreparingWizard</td><td>1033</td><td>[ProductName] Setup is preparing the InstallShield Wizard which will guide you through the program setup process.  Please wait.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsInitDlg_WelcomeWizard</td><td>1033</td><td>{&amp;TahomaBold10}Welcome to the InstallShield Wizard for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLicenseDlg_LicenseAgreement</td><td>1033</td><td>{&amp;MSSansBold8}License Agreement</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLicenseDlg_ReadLicenseAgreement</td><td>1033</td><td>Please read the following license agreement carefully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLogonInfoDescription</td><td>1033</td><td>Specify the user name and password of the user account that will logon to use this application. The user account must be in the form DOMAIN\Username.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLogonInfoTitle</td><td>1033</td><td>{&amp;MSSansBold8}Logon Information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLogonInfoTitleDescription</td><td>1033</td><td>Specify a user name and password</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsLogonNewUserDescription</td><td>1033</td><td>Select the button below to specify information about a new user that will be created during the installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_ChangeFeatures</td><td>1033</td><td>Change which program features are installed. This option displays the Custom Selection dialog in which you can change the way features are installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_MaitenanceOptions</td><td>1033</td><td>Modify, repair, or remove the program.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_Modify</td><td>1033</td><td>{&amp;MSSansBold8}&amp;Modify</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_ProgramMaintenance</td><td>1033</td><td>{&amp;MSSansBold8}Program Maintenance</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_Remove</td><td>1033</td><td>{&amp;MSSansBold8}&amp;Remove</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_RemoveProductName</td><td>1033</td><td>Remove [ProductName] from your computer.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_Repair</td><td>1033</td><td>{&amp;MSSansBold8}Re&amp;pair</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceDlg_RepairMessage</td><td>1033</td><td>Repair installation errors in the program. This option fixes missing or corrupt files, shortcuts, and registry entries.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceWelcome_MaintenanceOptionsDescription</td><td>1033</td><td>The InstallShield(R) Wizard will allow you to modify, repair, or remove [ProductName]. To continue, click Next.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMaintenanceWelcome_WizardWelcome</td><td>1033</td><td>{&amp;TahomaBold10}Welcome to the InstallShield Wizard for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMsiRMFilesInUse_ApplicationsUsingFiles</td><td>1033</td><td>The following applications are using files that need to be updated by this setup.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMsiRMFilesInUse_CloseRestart</td><td>1033</td><td>Automatically close and attempt to restart applications.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsMsiRMFilesInUse_RebootAfter</td><td>1033</td><td>Do not close applications. (A reboot will be required.)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsPatchDlg_PatchClickUpdate</td><td>1033</td><td>The InstallShield(R) Wizard will install the Patch for [ProductName] on your computer.  To continue, click Update.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsPatchDlg_PatchWizard</td><td>1033</td><td>[ProductName] Patch - InstallShield Wizard</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsPatchDlg_Update</td><td>1033</td><td>&amp;Update &gt;</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsPatchDlg_WelcomePatchWizard</td><td>1033</td><td>{&amp;TahomaBold10}Welcome to the Patch for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_2</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_Hidden</td><td>1033</td><td>(Hidden for now)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_HiddenTimeRemaining</td><td>1033</td><td>)Hidden for now)Estimated time remaining:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_InstallingProductName</td><td>1033</td><td>{&amp;MSSansBold8}Installing [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_ProgressDone</td><td>1033</td><td>Progress done</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_SecHidden</td><td>1033</td><td>(Hidden for now)Sec.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_Status</td><td>1033</td><td>Status:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_Uninstalling</td><td>1033</td><td>{&amp;MSSansBold8}Uninstalling [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_UninstallingFeatures</td><td>1033</td><td>The program features you selected are being uninstalled.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_UninstallingFeatures2</td><td>1033</td><td>The program features you selected are being installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_WaitUninstall</td><td>1033</td><td>Please wait while the InstallShield Wizard uninstalls [ProductName]. This may take several minutes.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsProgressDlg_WaitUninstall2</td><td>1033</td><td>Please wait while the InstallShield Wizard installs [ProductName]. This may take several minutes.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsReadmeDlg_Cancel</td><td>1033</td><td>&amp;Cancel</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsReadmeDlg_PleaseReadInfo</td><td>1033</td><td>Please read the following readme information carefully.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsReadmeDlg_ReadMeInfo</td><td>1033</td><td>{&amp;MSSansBold8}Readme Information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_16</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_Anyone</td><td>1033</td><td>&amp;Anyone who uses this computer (all users)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_CustomerInformation</td><td>1033</td><td>{&amp;MSSansBold8}Customer Information</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_InstallFor</td><td>1033</td><td>Install this application for:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_OnlyMe</td><td>1033</td><td>Only for &amp;me ([USERNAME])</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_Organization</td><td>1033</td><td>&amp;Organization:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_PleaseEnterInfo</td><td>1033</td><td>Please enter your information.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_SerialNumber</td><td>1033</td><td>&amp;Serial Number:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_Tahoma50</td><td>1033</td><td>{\Tahoma8}{50}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_Tahoma80</td><td>1033</td><td>{\Tahoma8}{80}</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsRegisterUserDlg_UserName</td><td>1033</td><td>&amp;User Name:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsResumeDlg_ResumeSuspended</td><td>1033</td><td>The InstallShield(R) Wizard will complete the suspended installation of [ProductName] on your computer. To continue, click Next.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsResumeDlg_Resuming</td><td>1033</td><td>{&amp;TahomaBold10}Resuming the InstallShield Wizard for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsResumeDlg_WizardResume</td><td>1033</td><td>The InstallShield(R) Wizard will complete the installation of [ProductName] on your computer. To continue, click Next.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSelectDomainOrServer</td><td>1033</td><td>Select a Domain or Server</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSelectDomainUserInstructions</td><td>1033</td><td>Use the browse buttons to select a domain\server and a user name.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupComplete_ShowMsiLog</td><td>1033</td><td>Show the Windows Installer log</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_13</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_AllFeatures</td><td>1033</td><td>All program features will be installed. (Requires the most disk space.)</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_ChooseFeatures</td><td>1033</td><td>Choose which program features you want installed and where they will be installed. Recommended for advanced users.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_ChooseSetupType</td><td>1033</td><td>Choose the setup type that best suits your needs.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_Complete</td><td>1033</td><td>{&amp;MSSansBold8}&amp;Complete</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_Custom</td><td>1033</td><td>{&amp;MSSansBold8}Cu&amp;stom</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_Minimal</td><td>1033</td><td>{&amp;MSSansBold8}&amp;Minimal</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_MinimumFeatures</td><td>1033</td><td>Minimum required features will be installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_SelectSetupType</td><td>1033</td><td>Please select a setup type.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_SetupType</td><td>1033</td><td>{&amp;MSSansBold8}Setup Type</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsSetupTypeMinDlg_Typical</td><td>1033</td><td>{&amp;MSSansBold8}&amp;Typical</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_ClickFinish</td><td>1033</td><td>Click Finish to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_Finish</td><td>1033</td><td>&amp;Finish</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_KeepOrRestore</td><td>1033</td><td>You can either keep any existing installed elements on your system to continue this installation at a later time or you can restore your system to its original state prior to the installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_NotModified</td><td>1033</td><td>Your system has not been modified. To install this program at a later time, please run the installation again.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_RestoreOrContinue</td><td>1033</td><td>Click Restore or Continue Later to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_WizardCompleted</td><td>1033</td><td>{&amp;TahomaBold10}InstallShield Wizard Completed</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserExit_WizardInterrupted</td><td>1033</td><td>The wizard was interrupted before [ProductName] could be completely installed.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsUserNameLabel</td><td>1033</td><td>&amp;User name:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_BackOrCancel</td><td>1033</td><td>If you want to review or change any of your installation settings, click Back. Click Cancel to exit the wizard.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_ClickInstall</td><td>1033</td><td>Click Install to begin the installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_Company</td><td>1033</td><td>Company: [COMPANYNAME]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_CurrentSettings</td><td>1033</td><td>Current Settings:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_DestFolder</td><td>1033</td><td>Destination Folder:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_Install</td><td>1033</td><td>&amp;Install</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_Installdir</td><td>1033</td><td>[INSTALLDIR]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_ModifyReady</td><td>1033</td><td>{&amp;MSSansBold8}Ready to Modify the Program</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_ReadyInstall</td><td>1033</td><td>{&amp;MSSansBold8}Ready to Install the Program</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_ReadyRepair</td><td>1033</td><td>{&amp;MSSansBold8}Ready to Repair the Program</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_SelectedSetupType</td><td>1033</td><td>[SelectedSetupType]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_Serial</td><td>1033</td><td>Serial: [ISX_SERIALNUM]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_SetupType</td><td>1033</td><td>Setup Type:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_UserInfo</td><td>1033</td><td>User Information:</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_UserName</td><td>1033</td><td>Name: [USERNAME]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyReadyDlg_WizardReady</td><td>1033</td><td>The wizard is ready to begin installation.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyRemoveAllDlg_ChoseRemoveProgram</td><td>1033</td><td>You have chosen to remove the program from your system.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyRemoveAllDlg_ClickBack</td><td>1033</td><td>If you want to review or change any settings, click Back.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyRemoveAllDlg_ClickRemove</td><td>1033</td><td>Click Remove to remove [ProductName] from your computer. After removal, this program will no longer be available for use.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyRemoveAllDlg_Remove</td><td>1033</td><td>&amp;Remove</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsVerifyRemoveAllDlg_RemoveProgram</td><td>1033</td><td>{&amp;MSSansBold8}Remove the Program</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsWelcomeDlg_InstallProductName</td><td>1033</td><td>The InstallShield(R) Wizard will install [ProductName] on your computer. To continue, click Next.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsWelcomeDlg_WarningCopyright</td><td>1033</td><td>WARNING: This program is protected by copyright law and international treaties.</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__IsWelcomeDlg_WelcomeProductName</td><td>1033</td><td>{&amp;TahomaBold10}Welcome to the InstallShield Wizard for [ProductName]</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__TargetReq_DESC_COLOR</td><td>1033</td><td>The color settings of your system are not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__TargetReq_DESC_OS</td><td>1033</td><td>The operating system is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__TargetReq_DESC_PROCESSOR</td><td>1033</td><td>The processor is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__TargetReq_DESC_RAM</td><td>1033</td><td>The amount of RAM is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>IDS__TargetReq_DESC_RESOLUTION</td><td>1033</td><td>The screen resolution is not adequate for running [ProductName].</td><td>0</td><td/><td>447022381</td></row>
		<row><td>ID_STRING1</td><td>1033</td><td/><td>0</td><td/><td>447022381</td></row>
		<row><td>ID_STRING2</td><td>1033</td><td/><td>0</td><td/><td>447063373</td></row>
		<row><td>ID_STRING3</td><td>1033</td><td>http://www.MouventAG.com</td><td>0</td><td/><td>1545986954</td></row>
		<row><td>ID_STRING4</td><td>1033</td><td>Mouvent AG</td><td>0</td><td/><td>1545986954</td></row>
		<row><td>IIDS_UITEXT_FeatureUninstalled</td><td>1033</td><td>This feature will remain uninstalled.</td><td>0</td><td/><td>447022381</td></row>
	</table>

	<table name="ISSwidtagProperty">
		<col key="yes" def="s72">Name</col>
		<col def="s0">Value</col>
		<row><td>UniqueId</td><td>F05F664D-F048-41E0-B105-4E8E8103C0FB</td></row>
	</table>

	<table name="ISTargetImage">
		<col key="yes" def="s13">UpgradedImage_</col>
		<col key="yes" def="s13">Name</col>
		<col def="s0">MsiPath</col>
		<col def="i2">Order</col>
		<col def="I4">Flags</col>
		<col def="i2">IgnoreMissingFiles</col>
	</table>

	<table name="ISUpgradeMsiItem">
		<col key="yes" def="s72">UpgradeItem</col>
		<col def="s0">ObjectSetupPath</col>
		<col def="S255">ISReleaseFlags</col>
		<col def="i2">ISAttributes</col>
	</table>

	<table name="ISUpgradedImage">
		<col key="yes" def="s13">Name</col>
		<col def="s0">MsiPath</col>
		<col def="s8">Family</col>
	</table>

	<table name="ISVirtualDirectory">
		<col key="yes" def="s72">Directory_</col>
		<col key="yes" def="s72">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISVirtualFile">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="s72">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISVirtualPackage">
		<col key="yes" def="s72">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISVirtualRegistry">
		<col key="yes" def="s72">Registry_</col>
		<col key="yes" def="s72">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISVirtualRelease">
		<col key="yes" def="s72">ISRelease_</col>
		<col key="yes" def="s72">ISProductConfiguration_</col>
		<col key="yes" def="s255">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISVirtualShortcut">
		<col key="yes" def="s72">Shortcut_</col>
		<col key="yes" def="s72">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="ISWSEWrap">
		<col key="yes" def="s72">Action_</col>
		<col key="yes" def="s72">Name</col>
		<col def="S0">Value</col>
	</table>

	<table name="ISXmlElement">
		<col key="yes" def="s72">ISXmlElement</col>
		<col def="s72">ISXmlFile_</col>
		<col def="S72">ISXmlElement_Parent</col>
		<col def="L0">XPath</col>
		<col def="L0">Content</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISXmlElementAttrib">
		<col key="yes" def="s72">ISXmlElementAttrib</col>
		<col key="yes" def="s72">ISXmlElement_</col>
		<col def="L255">Name</col>
		<col def="L0">Value</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="ISXmlFile">
		<col key="yes" def="s72">ISXmlFile</col>
		<col def="l255">FileName</col>
		<col def="s72">Component_</col>
		<col def="s72">Directory</col>
		<col def="I4">ISAttributes</col>
		<col def="S0">SelectionNamespaces</col>
		<col def="S255">Encoding</col>
	</table>

	<table name="ISXmlLocator">
		<col key="yes" def="s72">Signature_</col>
		<col key="yes" def="S72">Parent</col>
		<col def="S255">Element</col>
		<col def="S255">Attribute</col>
		<col def="I2">ISAttributes</col>
	</table>

	<table name="Icon">
		<col key="yes" def="s72">Name</col>
		<col def="V0">Data</col>
		<col def="S255">ISBuildSourcePath</col>
		<col def="I2">ISIconIndex</col>
		<row><td>ARPPRODUCTICON.exe</td><td/><td>&lt;ISProductFolder&gt;\redist\Language Independent\OS Independent\setupicon.ico</td><td>0</td></row>
		<row><td>Win10_Install.exe_4D56BAB63C314E76948D7725831B84EE.exe</td><td/><td>C:\Develop\RX-Project\bin\win10\Win10-Install.exe</td><td>0</td></row>
	</table>

	<table name="IniFile">
		<col key="yes" def="s72">IniFile</col>
		<col def="l255">FileName</col>
		<col def="S72">DirProperty</col>
		<col def="l255">Section</col>
		<col def="l128">Key</col>
		<col def="s255">Value</col>
		<col def="i2">Action</col>
		<col def="s72">Component_</col>
	</table>

	<table name="IniLocator">
		<col key="yes" def="s72">Signature_</col>
		<col def="s255">FileName</col>
		<col def="s96">Section</col>
		<col def="s128">Key</col>
		<col def="I2">Field</col>
		<col def="I2">Type</col>
	</table>

	<table name="InstallExecuteSequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
		<row><td>AllocateRegistrySpace</td><td>NOT Installed</td><td>1550</td><td>AllocateRegistrySpace</td><td/></row>
		<row><td>AppSearch</td><td/><td>400</td><td>AppSearch</td><td/></row>
		<row><td>BindImage</td><td/><td>4300</td><td>BindImage</td><td/></row>
		<row><td>CCPSearch</td><td>CCP_TEST</td><td>500</td><td>CCPSearch</td><td/></row>
		<row><td>CostFinalize</td><td/><td>1000</td><td>CostFinalize</td><td/></row>
		<row><td>CostInitialize</td><td/><td>800</td><td>CostInitialize</td><td/></row>
		<row><td>CreateFolders</td><td/><td>3700</td><td>CreateFolders</td><td/></row>
		<row><td>CreateShortcuts</td><td/><td>4500</td><td>CreateShortcuts</td><td/></row>
		<row><td>DeleteServices</td><td>VersionNT</td><td>2000</td><td>DeleteServices</td><td/></row>
		<row><td>DuplicateFiles</td><td/><td>4210</td><td>DuplicateFiles</td><td/></row>
		<row><td>FileCost</td><td/><td>900</td><td>FileCost</td><td/></row>
		<row><td>FindRelatedProducts</td><td>NOT ISSETUPDRIVEN</td><td>420</td><td>FindRelatedProducts</td><td/></row>
		<row><td>ISPreventDowngrade</td><td>ISFOUNDNEWERPRODUCTVERSION</td><td>450</td><td/><td/></row>
		<row><td>ISRunSetupTypeAddLocalEvent</td><td>Not Installed And Not ISRUNSETUPTYPEADDLOCALEVENT</td><td>1050</td><td>ISRunSetupTypeAddLocalEvent</td><td/></row>
		<row><td>ISSelfRegisterCosting</td><td/><td>2201</td><td/><td/></row>
		<row><td>ISSelfRegisterFiles</td><td/><td>5601</td><td/><td/></row>
		<row><td>ISSelfRegisterFinalize</td><td/><td>6601</td><td/><td/></row>
		<row><td>ISSetAllUsers</td><td>Not Installed</td><td>10</td><td/><td/></row>
		<row><td>ISUnSelfRegisterFiles</td><td/><td>2202</td><td/><td/></row>
		<row><td>InstallFiles</td><td/><td>4000</td><td>InstallFiles</td><td/></row>
		<row><td>InstallFinalize</td><td/><td>6600</td><td>InstallFinalize</td><td/></row>
		<row><td>InstallInitialize</td><td/><td>1501</td><td>InstallInitialize</td><td/></row>
		<row><td>InstallODBC</td><td/><td>5400</td><td>InstallODBC</td><td/></row>
		<row><td>InstallServices</td><td>VersionNT</td><td>5800</td><td>InstallServices</td><td/></row>
		<row><td>InstallValidate</td><td/><td>1400</td><td>InstallValidate</td><td/></row>
		<row><td>IsolateComponents</td><td/><td>950</td><td>IsolateComponents</td><td/></row>
		<row><td>LaunchConditions</td><td>Not Installed</td><td>410</td><td>LaunchConditions</td><td/></row>
		<row><td>MigrateFeatureStates</td><td/><td>1010</td><td>MigrateFeatureStates</td><td/></row>
		<row><td>MoveFiles</td><td/><td>3800</td><td>MoveFiles</td><td/></row>
		<row><td>MsiConfigureServices</td><td>VersionMsi &gt;= "5.00"</td><td>5850</td><td>MSI5 MsiConfigureServices</td><td/></row>
		<row><td>MsiPublishAssemblies</td><td/><td>6250</td><td>MsiPublishAssemblies</td><td/></row>
		<row><td>MsiUnpublishAssemblies</td><td/><td>1750</td><td>MsiUnpublishAssemblies</td><td/></row>
		<row><td>PatchFiles</td><td/><td>4090</td><td>PatchFiles</td><td/></row>
		<row><td>ProcessComponents</td><td/><td>1600</td><td>ProcessComponents</td><td/></row>
		<row><td>PublishComponents</td><td/><td>6200</td><td>PublishComponents</td><td/></row>
		<row><td>PublishFeatures</td><td/><td>6300</td><td>PublishFeatures</td><td/></row>
		<row><td>PublishProduct</td><td/><td>6400</td><td>PublishProduct</td><td/></row>
		<row><td>RMCCPSearch</td><td>Not CCP_SUCCESS And CCP_TEST</td><td>600</td><td>RMCCPSearch</td><td/></row>
		<row><td>RegisterClassInfo</td><td/><td>4600</td><td>RegisterClassInfo</td><td/></row>
		<row><td>RegisterComPlus</td><td/><td>5700</td><td>RegisterComPlus</td><td/></row>
		<row><td>RegisterExtensionInfo</td><td/><td>4700</td><td>RegisterExtensionInfo</td><td/></row>
		<row><td>RegisterFonts</td><td/><td>5300</td><td>RegisterFonts</td><td/></row>
		<row><td>RegisterMIMEInfo</td><td/><td>4900</td><td>RegisterMIMEInfo</td><td/></row>
		<row><td>RegisterProduct</td><td/><td>6100</td><td>RegisterProduct</td><td/></row>
		<row><td>RegisterProgIdInfo</td><td/><td>4800</td><td>RegisterProgIdInfo</td><td/></row>
		<row><td>RegisterTypeLibraries</td><td/><td>5500</td><td>RegisterTypeLibraries</td><td/></row>
		<row><td>RegisterUser</td><td/><td>6000</td><td>RegisterUser</td><td/></row>
		<row><td>RemoveDuplicateFiles</td><td/><td>3400</td><td>RemoveDuplicateFiles</td><td/></row>
		<row><td>RemoveEnvironmentStrings</td><td/><td>3300</td><td>RemoveEnvironmentStrings</td><td/></row>
		<row><td>RemoveExistingProducts</td><td/><td>1500</td><td>RemoveExistingProducts</td><td/></row>
		<row><td>RemoveFiles</td><td/><td>3500</td><td>RemoveFiles</td><td/></row>
		<row><td>RemoveFolders</td><td/><td>3600</td><td>RemoveFolders</td><td/></row>
		<row><td>RemoveIniValues</td><td/><td>3100</td><td>RemoveIniValues</td><td/></row>
		<row><td>RemoveODBC</td><td/><td>2400</td><td>RemoveODBC</td><td/></row>
		<row><td>RemoveRegistryValues</td><td/><td>2600</td><td>RemoveRegistryValues</td><td/></row>
		<row><td>RemoveShortcuts</td><td/><td>3200</td><td>RemoveShortcuts</td><td/></row>
		<row><td>ResolveSource</td><td>Not Installed</td><td>850</td><td>ResolveSource</td><td/></row>
		<row><td>ScheduleReboot</td><td>ISSCHEDULEREBOOT</td><td>6410</td><td>ScheduleReboot</td><td/></row>
		<row><td>SelfRegModules</td><td/><td>5600</td><td>SelfRegModules</td><td/></row>
		<row><td>SelfUnregModules</td><td/><td>2200</td><td>SelfUnregModules</td><td/></row>
		<row><td>SetARPINSTALLLOCATION</td><td/><td>1100</td><td>SetARPINSTALLLOCATION</td><td/></row>
		<row><td>SetAllUsersProfileNT</td><td>VersionNT = 400</td><td>970</td><td/><td/></row>
		<row><td>SetODBCFolders</td><td/><td>1200</td><td>SetODBCFolders</td><td/></row>
		<row><td>StartServices</td><td>VersionNT</td><td>5900</td><td>StartServices</td><td/></row>
		<row><td>StopServices</td><td>VersionNT</td><td>1900</td><td>StopServices</td><td/></row>
		<row><td>UnpublishComponents</td><td/><td>1700</td><td>UnpublishComponents</td><td/></row>
		<row><td>UnpublishFeatures</td><td/><td>1800</td><td>UnpublishFeatures</td><td/></row>
		<row><td>UnregisterClassInfo</td><td/><td>2700</td><td>UnregisterClassInfo</td><td/></row>
		<row><td>UnregisterComPlus</td><td/><td>2100</td><td>UnregisterComPlus</td><td/></row>
		<row><td>UnregisterExtensionInfo</td><td/><td>2800</td><td>UnregisterExtensionInfo</td><td/></row>
		<row><td>UnregisterFonts</td><td/><td>2500</td><td>UnregisterFonts</td><td/></row>
		<row><td>UnregisterMIMEInfo</td><td/><td>3000</td><td>UnregisterMIMEInfo</td><td/></row>
		<row><td>UnregisterProgIdInfo</td><td/><td>2900</td><td>UnregisterProgIdInfo</td><td/></row>
		<row><td>UnregisterTypeLibraries</td><td/><td>2300</td><td>UnregisterTypeLibraries</td><td/></row>
		<row><td>ValidateProductID</td><td/><td>700</td><td>ValidateProductID</td><td/></row>
		<row><td>WriteEnvironmentStrings</td><td/><td>5200</td><td>WriteEnvironmentStrings</td><td/></row>
		<row><td>WriteIniValues</td><td/><td>5100</td><td>WriteIniValues</td><td/></row>
		<row><td>WriteRegistryValues</td><td/><td>5000</td><td>WriteRegistryValues</td><td/></row>
		<row><td>setAllUsersProfile2K</td><td>VersionNT &gt;= 500</td><td>980</td><td/><td/></row>
		<row><td>setUserProfileNT</td><td>VersionNT</td><td>960</td><td/><td/></row>
	</table>

	<table name="InstallShield">
		<col key="yes" def="s72">Property</col>
		<col def="S0">Value</col>
		<row><td>ActiveLanguage</td><td>1033</td></row>
		<row><td>Comments</td><td/></row>
		<row><td>CurrentMedia</td><td dt:dt="bin.base64" md5="de9f554a3bc05c12be9c31b998217995">
UwBpAG4AZwBsAGUASQBtAGEAZwBlAAEARQB4AHAAcgBlAHMAcwA=
			</td></row>
		<row><td>DefaultProductConfiguration</td><td>Express</td></row>
		<row><td>EnableSwidtag</td><td>1</td></row>
		<row><td>ISCompilerOption_CompileBeforeBuild</td><td>1</td></row>
		<row><td>ISCompilerOption_Debug</td><td>0</td></row>
		<row><td>ISCompilerOption_IncludePath</td><td/></row>
		<row><td>ISCompilerOption_LibraryPath</td><td/></row>
		<row><td>ISCompilerOption_MaxErrors</td><td>50</td></row>
		<row><td>ISCompilerOption_MaxWarnings</td><td>50</td></row>
		<row><td>ISCompilerOption_OutputPath</td><td>&lt;ISProjectDataFolder&gt;\Script Files</td></row>
		<row><td>ISCompilerOption_PreProcessor</td><td>_ISSCRIPT_NEW_STYLE_DLG_DEFS</td></row>
		<row><td>ISCompilerOption_WarningLevel</td><td>3</td></row>
		<row><td>ISCompilerOption_WarningsAsErrors</td><td>1</td></row>
		<row><td>ISTheme</td><td>InstallShield Blue.theme</td></row>
		<row><td>ISUSLock</td><td>{297AB677-AB01-4BC9-A5FC-0ED55CCCA896}</td></row>
		<row><td>ISUSSignature</td><td>{51C52807-72C7-480F-A696-9D450D5B0DB8}</td></row>
		<row><td>ISVisitedViews</td><td>viewAssistant,viewISToday,viewLearnMore,viewSystemSearch,viewUI,viewCustomActions,viewAppFiles,viewRegistry,viewUpgradePaths,viewProject,viewUpdateService,viewShortcuts,viewServices,viewTextMessages,viewRelease,viewFileExtensions,viewObjects</td></row>
		<row><td>Limited</td><td>1</td></row>
		<row><td>LockPermissionMode</td><td>1</td></row>
		<row><td>MsiExecCmdLineOptions</td><td/></row>
		<row><td>MsiLogFile</td><td/></row>
		<row><td>OnUpgrade</td><td>0</td></row>
		<row><td>Owner</td><td/></row>
		<row><td>PatchFamily</td><td>MyPatchFamily1</td></row>
		<row><td>PatchSequence</td><td>1.0.0</td></row>
		<row><td>SaveAsSchema</td><td/></row>
		<row><td>SccEnabled</td><td>0</td></row>
		<row><td>SccPath</td><td/></row>
		<row><td>SchemaVersion</td><td>776</td></row>
		<row><td>Type</td><td>MSIE</td></row>
	</table>

	<table name="InstallUISequence">
		<col key="yes" def="s72">Action</col>
		<col def="S255">Condition</col>
		<col def="I2">Sequence</col>
		<col def="S255">ISComments</col>
		<col def="I4">ISAttributes</col>
		<row><td>AppSearch</td><td/><td>400</td><td>AppSearch</td><td/></row>
		<row><td>CCPSearch</td><td>CCP_TEST</td><td>500</td><td>CCPSearch</td><td/></row>
		<row><td>CostFinalize</td><td/><td>1000</td><td>CostFinalize</td><td/></row>
		<row><td>CostInitialize</td><td/><td>800</td><td>CostInitialize</td><td/></row>
		<row><td>ExecuteAction</td><td/><td>1300</td><td>ExecuteAction</td><td/></row>
		<row><td>FileCost</td><td/><td>900</td><td>FileCost</td><td/></row>
		<row><td>FindRelatedProducts</td><td/><td>430</td><td>FindRelatedProducts</td><td/></row>
		<row><td>ISPreventDowngrade</td><td>ISFOUNDNEWERPRODUCTVERSION</td><td>450</td><td/><td/></row>
		<row><td>ISSetAllUsers</td><td>Not Installed</td><td>10</td><td/><td/></row>
		<row><td>InstallWelcome</td><td>Not UITEST And Not Installed</td><td>1110</td><td/><td/></row>
		<row><td>IsolateComponents</td><td/><td>950</td><td>IsolateComponents</td><td/></row>
		<row><td>LaunchConditions</td><td>Not Installed</td><td>410</td><td>LaunchConditions</td><td/></row>
		<row><td>MaintenanceWelcome</td><td>Installed And Not RESUME And Not Preselected And Not PATCH</td><td>1230</td><td>MaintenanceWelcome</td><td/></row>
		<row><td>MigrateFeatureStates</td><td/><td>1200</td><td>MigrateFeatureStates</td><td/></row>
		<row><td>PatchWelcome</td><td>Installed And PATCH And Not IS_MAJOR_UPGRADE</td><td>1205</td><td>Patch Panel</td><td/></row>
		<row><td>RMCCPSearch</td><td>Not CCP_SUCCESS And CCP_TEST</td><td>600</td><td>RMCCPSearch</td><td/></row>
		<row><td>ResolveSource</td><td>Not Installed</td><td>990</td><td>ResolveSource</td><td/></row>
		<row><td>SetAllUsersProfileNT</td><td>VersionNT = 400</td><td>970</td><td/><td/></row>
		<row><td>SetupCompleteError</td><td/><td>-3</td><td>SetupCompleteError</td><td/></row>
		<row><td>SetupCompleteSuccess</td><td/><td>-1</td><td>SetupCompleteSuccess</td><td/></row>
		<row><td>SetupInitialization</td><td/><td>420</td><td>SetupInitialization</td><td/></row>
		<row><td>SetupInterrupted</td><td/><td>-2</td><td>SetupInterrupted</td><td/></row>
		<row><td>SetupResume</td><td>Installed And (RESUME Or Preselected) And Not PATCH</td><td>1220</td><td>SetupResume</td><td/></row>
		<row><td>ValidateProductID</td><td/><td>700</td><td>ValidateProductID</td><td/></row>
		<row><td>setAllUsersProfile2K</td><td>VersionNT &gt;= 500</td><td>980</td><td/><td/></row>
		<row><td>setUserProfileNT</td><td>VersionNT</td><td>960</td><td/><td/></row>
	</table>

	<table name="IsolatedComponent">
		<col key="yes" def="s72">Component_Shared</col>
		<col key="yes" def="s72">Component_Application</col>
	</table>

	<table name="LaunchCondition">
		<col key="yes" def="s255">Condition</col>
		<col def="l255">Description</col>
	</table>

	<table name="ListBox">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="i2">Order</col>
		<col def="s64">Value</col>
		<col def="L64">Text</col>
	</table>

	<table name="ListView">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="i2">Order</col>
		<col def="s64">Value</col>
		<col def="L64">Text</col>
		<col def="S72">Binary_</col>
	</table>

	<table name="LockPermissions">
		<col key="yes" def="s72">LockObject</col>
		<col key="yes" def="s32">Table</col>
		<col key="yes" def="S255">Domain</col>
		<col key="yes" def="s255">User</col>
		<col def="I4">Permission</col>
	</table>

	<table name="MIME">
		<col key="yes" def="s64">ContentType</col>
		<col def="s255">Extension_</col>
		<col def="S38">CLSID</col>
	</table>

	<table name="Media">
		<col key="yes" def="i2">DiskId</col>
		<col def="i2">LastSequence</col>
		<col def="L64">DiskPrompt</col>
		<col def="S255">Cabinet</col>
		<col def="S32">VolumeLabel</col>
		<col def="S32">Source</col>
	</table>

	<table name="MoveFile">
		<col key="yes" def="s72">FileKey</col>
		<col def="s72">Component_</col>
		<col def="L255">SourceName</col>
		<col def="L255">DestName</col>
		<col def="S72">SourceFolder</col>
		<col def="s72">DestFolder</col>
		<col def="i2">Options</col>
	</table>

	<table name="MsiAssembly">
		<col key="yes" def="s72">Component_</col>
		<col def="s38">Feature_</col>
		<col def="S72">File_Manifest</col>
		<col def="S72">File_Application</col>
		<col def="I2">Attributes</col>
	</table>

	<table name="MsiAssemblyName">
		<col key="yes" def="s72">Component_</col>
		<col key="yes" def="s255">Name</col>
		<col def="s255">Value</col>
	</table>

	<table name="MsiDigitalCertificate">
		<col key="yes" def="s72">DigitalCertificate</col>
		<col def="v0">CertData</col>
	</table>

	<table name="MsiDigitalSignature">
		<col key="yes" def="s32">Table</col>
		<col key="yes" def="s72">SignObject</col>
		<col def="s72">DigitalCertificate_</col>
		<col def="V0">Hash</col>
	</table>

	<table name="MsiDriverPackages">
		<col key="yes" def="s72">Component</col>
		<col def="i4">Flags</col>
		<col def="I4">Sequence</col>
		<col def="S0">ReferenceComponents</col>
	</table>

	<table name="MsiEmbeddedChainer">
		<col key="yes" def="s72">MsiEmbeddedChainer</col>
		<col def="S255">Condition</col>
		<col def="S255">CommandLine</col>
		<col def="s72">Source</col>
		<col def="I4">Type</col>
	</table>

	<table name="MsiEmbeddedUI">
		<col key="yes" def="s72">MsiEmbeddedUI</col>
		<col def="s255">FileName</col>
		<col def="i2">Attributes</col>
		<col def="I4">MessageFilter</col>
		<col def="V0">Data</col>
		<col def="S255">ISBuildSourcePath</col>
	</table>

	<table name="MsiFileHash">
		<col key="yes" def="s72">File_</col>
		<col def="i2">Options</col>
		<col def="i4">HashPart1</col>
		<col def="i4">HashPart2</col>
		<col def="i4">HashPart3</col>
		<col def="i4">HashPart4</col>
	</table>

	<table name="MsiLockPermissionsEx">
		<col key="yes" def="s72">MsiLockPermissionsEx</col>
		<col def="s72">LockObject</col>
		<col def="s32">Table</col>
		<col def="s0">SDDLText</col>
		<col def="S255">Condition</col>
	</table>

	<table name="MsiPackageCertificate">
		<col key="yes" def="s72">PackageCertificate</col>
		<col def="s72">DigitalCertificate_</col>
	</table>

	<table name="MsiPatchCertificate">
		<col key="yes" def="s72">PatchCertificate</col>
		<col def="s72">DigitalCertificate_</col>
	</table>

	<table name="MsiPatchMetadata">
		<col key="yes" def="s72">PatchConfiguration_</col>
		<col key="yes" def="S72">Company</col>
		<col key="yes" def="s72">Property</col>
		<col def="S0">Value</col>
	</table>

	<table name="MsiPatchOldAssemblyFile">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="S72">Assembly_</col>
	</table>

	<table name="MsiPatchOldAssemblyName">
		<col key="yes" def="s72">Assembly</col>
		<col key="yes" def="s255">Name</col>
		<col def="S255">Value</col>
	</table>

	<table name="MsiPatchSequence">
		<col key="yes" def="s72">PatchConfiguration_</col>
		<col key="yes" def="s0">PatchFamily</col>
		<col key="yes" def="S0">Target</col>
		<col def="s0">Sequence</col>
		<col def="i2">Supersede</col>
	</table>

	<table name="MsiServiceConfig">
		<col key="yes" def="s72">MsiServiceConfig</col>
		<col def="s255">Name</col>
		<col def="i2">Event</col>
		<col def="i4">ConfigType</col>
		<col def="S0">Argument</col>
		<col def="s72">Component_</col>
	</table>

	<table name="MsiServiceConfigFailureActions">
		<col key="yes" def="s72">MsiServiceConfigFailureActions</col>
		<col def="s255">Name</col>
		<col def="i2">Event</col>
		<col def="I4">ResetPeriod</col>
		<col def="L255">RebootMessage</col>
		<col def="L255">Command</col>
		<col def="S0">Actions</col>
		<col def="S0">DelayActions</col>
		<col def="s72">Component_</col>
	</table>

	<table name="MsiShortcutProperty">
		<col key="yes" def="s72">MsiShortcutProperty</col>
		<col def="s72">Shortcut_</col>
		<col def="s0">PropertyKey</col>
		<col def="s0">PropVariantValue</col>
	</table>

	<table name="ODBCAttribute">
		<col key="yes" def="s72">Driver_</col>
		<col key="yes" def="s40">Attribute</col>
		<col def="S255">Value</col>
	</table>

	<table name="ODBCDataSource">
		<col key="yes" def="s72">DataSource</col>
		<col def="s72">Component_</col>
		<col def="s255">Description</col>
		<col def="s255">DriverDescription</col>
		<col def="i2">Registration</col>
	</table>

	<table name="ODBCDriver">
		<col key="yes" def="s72">Driver</col>
		<col def="s72">Component_</col>
		<col def="s255">Description</col>
		<col def="s72">File_</col>
		<col def="S72">File_Setup</col>
	</table>

	<table name="ODBCSourceAttribute">
		<col key="yes" def="s72">DataSource_</col>
		<col key="yes" def="s32">Attribute</col>
		<col def="S255">Value</col>
	</table>

	<table name="ODBCTranslator">
		<col key="yes" def="s72">Translator</col>
		<col def="s72">Component_</col>
		<col def="s255">Description</col>
		<col def="s72">File_</col>
		<col def="S72">File_Setup</col>
	</table>

	<table name="Patch">
		<col key="yes" def="s72">File_</col>
		<col key="yes" def="i2">Sequence</col>
		<col def="i4">PatchSize</col>
		<col def="i2">Attributes</col>
		<col def="V0">Header</col>
		<col def="S38">StreamRef_</col>
		<col def="S255">ISBuildSourcePath</col>
	</table>

	<table name="PatchPackage">
		<col key="yes" def="s38">PatchId</col>
		<col def="i2">Media_</col>
	</table>

	<table name="ProgId">
		<col key="yes" def="s255">ProgId</col>
		<col def="S255">ProgId_Parent</col>
		<col def="S38">Class_</col>
		<col def="L255">Description</col>
		<col def="S72">Icon_</col>
		<col def="I2">IconIndex</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="Property">
		<col key="yes" def="s72">Property</col>
		<col def="L0">Value</col>
		<col def="S255">ISComments</col>
		<row><td>ALLUSERS</td><td>1</td><td/></row>
		<row><td>ARPINSTALLLOCATION</td><td/><td/></row>
		<row><td>ARPNOMODIFY</td><td>1</td><td/></row>
		<row><td>ARPNOREPAIR</td><td>1</td><td/></row>
		<row><td>ARPPRODUCTICON</td><td>ARPPRODUCTICON.exe</td><td/></row>
		<row><td>ARPSIZE</td><td/><td/></row>
		<row><td>ARPURLINFOABOUT</td><td>##ID_STRING3##</td><td/></row>
		<row><td>AgreeToLicense</td><td>No</td><td/></row>
		<row><td>ApplicationUsers</td><td>AllUsers</td><td/></row>
		<row><td>DIRPROPERTY1</td><td>w</td><td/></row>
		<row><td>DWUSINTERVAL</td><td>30</td><td/></row>
		<row><td>DWUSLINK</td><td>CEDC17C889AB77FFCEACF088F9CC978F699B608FCE9C60D8CE3C8788CEFCE7E8DEBB50382EAC</td><td/></row>
		<row><td>DefaultUIFont</td><td>ExpressDefault</td><td/></row>
		<row><td>DialogCaption</td><td>InstallShield for Windows Installer</td><td/></row>
		<row><td>DiskPrompt</td><td>[1]</td><td/></row>
		<row><td>DiskSerial</td><td>1234-5678</td><td/></row>
		<row><td>DisplayNameCustom</td><td>##IDS__DisplayName_Custom##</td><td/></row>
		<row><td>DisplayNameMinimal</td><td>##IDS__DisplayName_Minimal##</td><td/></row>
		<row><td>DisplayNameTypical</td><td>##IDS__DisplayName_Typical##</td><td/></row>
		<row><td>Display_IsBitmapDlg</td><td>1</td><td/></row>
		<row><td>ErrorDialog</td><td>SetupError</td><td/></row>
		<row><td>INSTALLLEVEL</td><td>200</td><td/></row>
		<row><td>ISCHECKFORPRODUCTUPDATES</td><td>1</td><td/></row>
		<row><td>ISENABLEDWUSFINISHDIALOG</td><td/><td/></row>
		<row><td>ISSHOWMSILOG</td><td/><td/></row>
		<row><td>ISVROOT_PORT_NO</td><td>0</td><td/></row>
		<row><td>IS_COMPLUS_PROGRESSTEXT_COST</td><td>##IDS_COMPLUS_PROGRESSTEXT_COST##</td><td/></row>
		<row><td>IS_COMPLUS_PROGRESSTEXT_INSTALL</td><td>##IDS_COMPLUS_PROGRESSTEXT_INSTALL##</td><td/></row>
		<row><td>IS_COMPLUS_PROGRESSTEXT_UNINSTALL</td><td>##IDS_COMPLUS_PROGRESSTEXT_UNINSTALL##</td><td/></row>
		<row><td>IS_PREVENT_DOWNGRADE_EXIT</td><td>##IDS_PREVENT_DOWNGRADE_EXIT##</td><td/></row>
		<row><td>IS_PROGMSG_TEXTFILECHANGS_REPLACE</td><td>##IDS_PROGMSG_TEXTFILECHANGS_REPLACE##</td><td/></row>
		<row><td>IS_PROGMSG_XML_COSTING</td><td>##IDS_PROGMSG_XML_COSTING##</td><td/></row>
		<row><td>IS_PROGMSG_XML_CREATE_FILE</td><td>##IDS_PROGMSG_XML_CREATE_FILE##</td><td/></row>
		<row><td>IS_PROGMSG_XML_FILES</td><td>##IDS_PROGMSG_XML_FILES##</td><td/></row>
		<row><td>IS_PROGMSG_XML_REMOVE_FILE</td><td>##IDS_PROGMSG_XML_REMOVE_FILE##</td><td/></row>
		<row><td>IS_PROGMSG_XML_ROLLBACK_FILES</td><td>##IDS_PROGMSG_XML_ROLLBACK_FILES##</td><td/></row>
		<row><td>IS_PROGMSG_XML_UPDATE_FILE</td><td>##IDS_PROGMSG_XML_UPDATE_FILE##</td><td/></row>
		<row><td>IS_SQLSERVER_AUTHENTICATION</td><td>0</td><td/></row>
		<row><td>IS_SQLSERVER_DATABASE</td><td/><td/></row>
		<row><td>IS_SQLSERVER_PASSWORD</td><td/><td/></row>
		<row><td>IS_SQLSERVER_SERVER</td><td/><td/></row>
		<row><td>IS_SQLSERVER_USERNAME</td><td>sa</td><td/></row>
		<row><td>InstallChoice</td><td>AR</td><td/></row>
		<row><td>LAUNCHREADME</td><td>1</td><td/></row>
		<row><td>Manufacturer</td><td>##COMPANY_NAME##</td><td/></row>
		<row><td>PIDKEY</td><td/><td/></row>
		<row><td>PIDTemplate</td><td>12345&lt;###-%%%%%%%&gt;@@@@@</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEAPPPOOL</td><td>##IDS_PROGMSG_IIS_CREATEAPPPOOL##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEAPPPOOLS</td><td>##IDS_PROGMSG_IIS_CREATEAPPPOOLS##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEVROOT</td><td>##IDS_PROGMSG_IIS_CREATEVROOT##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEVROOTS</td><td>##IDS_PROGMSG_IIS_CREATEVROOTS##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEWEBSERVICEEXTENSION</td><td>##IDS_PROGMSG_IIS_CREATEWEBSERVICEEXTENSION##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEWEBSERVICEEXTENSIONS</td><td>##IDS_PROGMSG_IIS_CREATEWEBSERVICEEXTENSIONS##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEWEBSITE</td><td>##IDS_PROGMSG_IIS_CREATEWEBSITE##</td><td/></row>
		<row><td>PROGMSG_IIS_CREATEWEBSITES</td><td>##IDS_PROGMSG_IIS_CREATEWEBSITES##</td><td/></row>
		<row><td>PROGMSG_IIS_EXTRACT</td><td>##IDS_PROGMSG_IIS_EXTRACT##</td><td/></row>
		<row><td>PROGMSG_IIS_EXTRACTDONE</td><td>##IDS_PROGMSG_IIS_EXTRACTDONE##</td><td/></row>
		<row><td>PROGMSG_IIS_EXTRACTDONEz</td><td>##IDS_PROGMSG_IIS_EXTRACTDONE##</td><td/></row>
		<row><td>PROGMSG_IIS_EXTRACTzDONE</td><td>##IDS_PROGMSG_IIS_EXTRACTDONE##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEAPPPOOL</td><td>##IDS_PROGMSG_IIS_REMOVEAPPPOOL##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEAPPPOOLS</td><td>##IDS_PROGMSG_IIS_REMOVEAPPPOOLS##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVESITE</td><td>##IDS_PROGMSG_IIS_REMOVESITE##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEVROOT</td><td>##IDS_PROGMSG_IIS_REMOVEVROOT##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEVROOTS</td><td>##IDS_PROGMSG_IIS_REMOVEVROOTS##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEWEBSERVICEEXTENSION</td><td>##IDS_PROGMSG_IIS_REMOVEWEBSERVICEEXTENSION##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEWEBSERVICEEXTENSIONS</td><td>##IDS_PROGMSG_IIS_REMOVEWEBSERVICEEXTENSIONS##</td><td/></row>
		<row><td>PROGMSG_IIS_REMOVEWEBSITES</td><td>##IDS_PROGMSG_IIS_REMOVEWEBSITES##</td><td/></row>
		<row><td>PROGMSG_IIS_ROLLBACKAPPPOOLS</td><td>##IDS_PROGMSG_IIS_ROLLBACKAPPPOOLS##</td><td/></row>
		<row><td>PROGMSG_IIS_ROLLBACKVROOTS</td><td>##IDS_PROGMSG_IIS_ROLLBACKVROOTS##</td><td/></row>
		<row><td>PROGMSG_IIS_ROLLBACKWEBSERVICEEXTENSIONS</td><td>##IDS_PROGMSG_IIS_ROLLBACKWEBSERVICEEXTENSIONS##</td><td/></row>
		<row><td>ProductCode</td><td>{9A89B654-669D-4576-A394-B6042054702B}</td><td/></row>
		<row><td>ProductName</td><td>Surface Installer</td><td/></row>
		<row><td>ProductVersion</td><td>1.00.0000</td><td/></row>
		<row><td>ProgressType0</td><td>install</td><td/></row>
		<row><td>ProgressType1</td><td>Installing</td><td/></row>
		<row><td>ProgressType2</td><td>installed</td><td/></row>
		<row><td>ProgressType3</td><td>installs</td><td/></row>
		<row><td>RebootYesNo</td><td>Yes</td><td/></row>
		<row><td>ReinstallFileVersion</td><td>o</td><td/></row>
		<row><td>ReinstallModeText</td><td>omus</td><td/></row>
		<row><td>ReinstallRepair</td><td>r</td><td/></row>
		<row><td>RestartManagerOption</td><td>CloseRestart</td><td/></row>
		<row><td>SERIALNUMBER</td><td/><td/></row>
		<row><td>SERIALNUMVALSUCCESSRETVAL</td><td>1</td><td/></row>
		<row><td>SHOWLAUNCHPROGRAM</td><td>0</td><td/></row>
		<row><td>SecureCustomProperties</td><td>USERNAME;COMPANYNAME;ISX_SERIALNUM;SUPPORTDIR;ISACTIONPROP1</td><td/></row>
		<row><td>SelectedSetupType</td><td>##IDS__DisplayName_Typical##</td><td/></row>
		<row><td>SetupType</td><td>Typical</td><td/></row>
		<row><td>UpgradeCode</td><td>{856CE885-382A-4590-AA4B-A65699E86621}</td><td/></row>
		<row><td>_IsMaintenance</td><td>Change</td><td/></row>
		<row><td>_IsSetupTypeMin</td><td>Typical</td><td/></row>
	</table>

	<table name="PublishComponent">
		<col key="yes" def="s38">ComponentId</col>
		<col key="yes" def="s255">Qualifier</col>
		<col key="yes" def="s72">Component_</col>
		<col def="L0">AppData</col>
		<col def="s38">Feature_</col>
	</table>

	<table name="RadioButton">
		<col key="yes" def="s72">Property</col>
		<col key="yes" def="i2">Order</col>
		<col def="s64">Value</col>
		<col def="i2">X</col>
		<col def="i2">Y</col>
		<col def="i2">Width</col>
		<col def="i2">Height</col>
		<col def="L64">Text</col>
		<col def="L50">Help</col>
		<col def="I4">ISControlId</col>
		<row><td>AgreeToLicense</td><td>1</td><td>No</td><td>0</td><td>15</td><td>291</td><td>15</td><td>##IDS__AgreeToLicense_0##</td><td/><td/></row>
		<row><td>AgreeToLicense</td><td>2</td><td>Yes</td><td>0</td><td>0</td><td>291</td><td>15</td><td>##IDS__AgreeToLicense_1##</td><td/><td/></row>
		<row><td>ApplicationUsers</td><td>1</td><td>AllUsers</td><td>1</td><td>7</td><td>290</td><td>14</td><td>##IDS__IsRegisterUserDlg_Anyone##</td><td/><td/></row>
		<row><td>ApplicationUsers</td><td>2</td><td>OnlyCurrentUser</td><td>1</td><td>23</td><td>290</td><td>14</td><td>##IDS__IsRegisterUserDlg_OnlyMe##</td><td/><td/></row>
		<row><td>RestartManagerOption</td><td>1</td><td>CloseRestart</td><td>6</td><td>9</td><td>331</td><td>14</td><td>##IDS__IsMsiRMFilesInUse_CloseRestart##</td><td/><td/></row>
		<row><td>RestartManagerOption</td><td>2</td><td>Reboot</td><td>6</td><td>21</td><td>331</td><td>14</td><td>##IDS__IsMsiRMFilesInUse_RebootAfter##</td><td/><td/></row>
		<row><td>_IsMaintenance</td><td>1</td><td>Change</td><td>0</td><td>0</td><td>290</td><td>14</td><td>##IDS__IsMaintenanceDlg_Modify##</td><td/><td/></row>
		<row><td>_IsMaintenance</td><td>2</td><td>Reinstall</td><td>0</td><td>60</td><td>290</td><td>14</td><td>##IDS__IsMaintenanceDlg_Repair##</td><td/><td/></row>
		<row><td>_IsMaintenance</td><td>3</td><td>Remove</td><td>0</td><td>120</td><td>290</td><td>14</td><td>##IDS__IsMaintenanceDlg_Remove##</td><td/><td/></row>
		<row><td>_IsSetupTypeMin</td><td>1</td><td>Typical</td><td>1</td><td>6</td><td>264</td><td>14</td><td>##IDS__IsSetupTypeMinDlg_Typical##</td><td/><td/></row>
	</table>

	<table name="RegLocator">
		<col key="yes" def="s72">Signature_</col>
		<col def="i2">Root</col>
		<col def="s255">Key</col>
		<col def="S255">Name</col>
		<col def="I2">Type</col>
	</table>

	<table name="Registry">
		<col key="yes" def="s72">Registry</col>
		<col def="i2">Root</col>
		<col def="s255">Key</col>
		<col def="S255">Name</col>
		<col def="S0">Value</col>
		<col def="s72">Component_</col>
		<col def="I4">ISAttributes</col>
		<row><td>Registry1</td><td>2</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\IniFileMapping\system.ini\boot</td><td/><td/><td>ISX_DEFAULTCOMPONENT1</td><td>1</td></row>
		<row><td>Registry10</td><td>2</td><td>SOFTWARE\Microsoft\Windows\CurrentVersion\Policies\System</td><td>ConsentPromptBehaviorAdmin1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT1</td><td>0</td></row>
		<row><td>Registry11</td><td>1</td><td>Control Panel\Desktop</td><td>ScreenSaverIsSecure</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry12</td><td>2</td><td>SYSTEM\CurrentControlSet\Control\Terminal Server</td><td>fDenyTSConnections</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry13</td><td>1</td><td>Control Panel\Desktop</td><td>ScreenSaveTimeOut</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry14</td><td>1</td><td>SOFTWARE\Microsoft\Windows\CurrentVersion\RunOnce1</td><td>Win10-RunOnce</td><td>"c:\Program Files (x86)\Mouvent\Win10-RunOnce.exe"</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry15</td><td>1</td><td>Control Panel\Sound</td><td>Beep</td><td>no</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry16</td><td>1</td><td>Control Panel\Sound</td><td>ExtendedSounds</td><td>no</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry17</td><td>1</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon</td><td>Shell</td><td>cmd /C "%ProgramFiles(x86)%\Mouvent\Win10-RunOnce.exe"</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry2</td><td>2</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\IniFileMapping\system.ini\boot</td><td>Shell</td><td>USR:Software\Microsoft\Windows NT\CurrentVersion\Winlogon</td><td>ISX_DEFAULTCOMPONENT1</td><td>0</td></row>
		<row><td>Registry3</td><td>2</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon</td><td>DefaultUserName</td><td>mouvent</td><td>ISX_DEFAULTCOMPONENT1</td><td>0</td></row>
		<row><td>Registry4</td><td>2</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon</td><td>DefaultPassword</td><td>mouvent</td><td>ISX_DEFAULTCOMPONENT1</td><td>0</td></row>
		<row><td>Registry5</td><td>2</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\Winlogon</td><td>AutoAdminLogon</td><td>1</td><td>ISX_DEFAULTCOMPONENT1</td><td>0</td></row>
		<row><td>Registry6</td><td>1</td><td>SOFTWARE\Microsoft\Windows NT\CurrentVersion\AppCompatFlags\Layers</td><td>c:\Program Files (x86)\Mouvent\Win10-RunOnce.exe</td><td>~ RUNASADMIN</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>Registry8</td><td>1</td><td>SOFTWARE</td><td/><td/><td>ISX_DEFAULTCOMPONENT2</td><td>1</td></row>
		<row><td>Registry9</td><td>1</td><td>Control Panel\Desktop</td><td>ScreenSaveActive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td>0</td></row>
		<row><td>_004DD8F5E193E7847221EC629BDFEC64</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TryAgent</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_00ECCCE86C3AD4A2B05A808D6DCD77D5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_010677A7B38392610248EFBA6BDF7455</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Xterm256Colour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0126C04BFB646B2C6BD1704343AE44E2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_015440B906E68A6581D0D43232B34916</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>DownloadsDragDropInternal</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_01578CAB0F7FAF52105BF6F7A687770B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxDownloadSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_01C492282649F6D364A8551B221CB967</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BackspaceIsDelete</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_01CFCEE0B686F7F22C609C7CE8959C57</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>ToolBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_020B067CFC1F02252B18AE583BB72F7E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteFilesOpenedInExternalEditor</td><td>#6</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0217FDE0252258359F24750DB7DDBCF1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SSHManualHostKeys</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02191F78952B1E66290C318EEADDB94C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>TipsNoUnseen</td><td>#23</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02356CEAF65AD290CA59F36C43833A82</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02437A41792FCE6205FFCF169ED2F009</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AuthSSPI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0253C8437787D62EEE0623BD12BA74EB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0267CCC2FF35B953DF4DC79451DF8C4D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FindWholeWord</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_027D158595B3D15080CB2E79A1494FAB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.5</td><td>PublicKeyFile</td><td>C:%5gui%5Cid_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_027EEFF2390CFD68F33E0AE3EB58A7D8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingWorkspaces</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02AAE385B785F68BC10B3A4680778932</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>Logging</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02BCD286E2A5E7641EDDA82C52893C20</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CRImpliesLF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_02C39C0E337A7C82108CA5BBC816062D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateNotifications</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0329C1EDFEE8E640F212A48AC6D5E2A3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FontHeight</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_035400B924CA80A3FF846E54812ABA01</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>TypicalInstallation</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_035C2D4E0A32FE85BDEDAC4F5D3B9D8F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmOverwriting</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_03C65703BD1342192C90D4E9C5DB3B01</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyLocalhost</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_03D25AA1C6E3A9DC335D777AA6FABABE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateDownloadOpensStartup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_04425EBAA11F1EF53189D21912562D63</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Font</td><td>Courier New</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_047284880240D415A823A219B2D3050A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TryFtpWhenSshFails</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_047715416E6F1DA2C03FC0E148B966F4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.1</td><td>HostName</td><td>192.168.200.1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0477DF532A41CF4AA599B33BBA63BF8D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>InstallationsUpgradeTypical</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0480D6D2652445DB3F54DA79FD623426</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>Text</td><td>rw-r--r--</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_04E77537EEF096908B2B911D2C294CE8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>BeepSound</td><td>SystemDefault</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_04F7B2C913E82FAF0F0B379DD23D47F8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LogFlush</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_051B56DF2A5AC65DC5A648927F0FD447</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SelectDirectories</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_055992ED6F497B8A749304F1CC195AB7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_057D8FBA8CE1B143873063CD0238689C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TryAgent</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_05CA7E505827F13B8878534D78DC5224</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.9</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0674B9AFD3A93B5414BEC1AE63B75782</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SshNoAuth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_069D31C71710239473D5797F459B33A8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_06BCA430D1A3FED068F73A3EF37BA013</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_073B64DB88AFC8CD20A0BCA6F3C44633</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>ExternalEditor</td><td>"C:%5CProgram%20Files%5CMicrosoft%20Office%5Croot%5COffice16%5CEXCEL.EXE"%20!.!</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_073E6EDE35B45541D1405BEF212E5466</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>TemporaryDirectoryCleanups</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_076B5B9C2DCA2B61235522BAB41F5226</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogFileName</td><td>%25TEMP%25%5C!S.log</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_07ADF11B4737BDE0EA90EEE4F5446D21</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RemoteCommand</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_07E8935AED13991EAC34B569ED523FAD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateDisplaysNew</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_07E8E3EE47C4BFC0ACB9407FC7A14E28</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>ExternalEditorText</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_07FF66613226E4781E1A0925631B0E78</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_082A1CC4DD0A69E9E76E9D9C3580DB73</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SshProt</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0837BB2CA08ABB0268E6E9ABA4BFE49C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsSSH2</td><td>#46</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0876E7CED8BE4CF8454983FAEE3BE321</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour14</td><td>0,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_088C576975220595C2C7EC3554544872</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ThemeMainInstructionFontSize</td><td>12</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_08C1E1B720ECAB1D657814550C822740</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExtensionsPortableCount</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_08EE35F9DF70DF03B2C215016A9F3D7A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>HostName</td><td>192.168.200.1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_08F76A227256E700F20B63E2333B4AA0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxOpenedSessions</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_08FDDA9C7A9F3F93714EE95DF317F210</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BellWaveFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_092C939FA4498FB7917AE2062D180CA4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FullScreenOnAltEnter</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_09390FA742AB6A7A373EC2A0722E5AF6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour15</td><td>85,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_09566F22C4FF39F9DDD2DEF7259E1BC4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyPassword</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_096817C7CAD9380CDEF95B5EB4FA9872</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.1:22:ssh</td><td>ssh-ed25519%20256%2073:b3:fc:32:92:7b:75:00:f2:26:7e:7d:de:6c:77:47</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0979BC10B95672FE5D786C6E70D5FAEB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_098B0278E630E0E07362F8C2D95B8611</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CopyOnDoubleClick</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_09FB0D7155368F5EC1E03EDCCDD1F276</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>MaxWatchDirectories</td><td>#500</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0A9F0783CCC9BA5612015FF936BC251F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TCPKeepalives</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0AC040074EDC63E77906E07265203A02</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SynchronizeModeAuto</td><td>#-1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0B0504A9A0A9181E38FC9906ACE32A74</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDTransferConfirmation2</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0B0BCEF163A40706B816FF49B877F22F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0B36B7DD5295DF31253D086319024E6D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ShowingTips</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0B5A8EBD7AF028425A18953C0ABAA473</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>Masks</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0B9542937384BCBBF7221F06067894D3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>SessionsTabs</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0BB579EDEC22C0270DBA967197A2C128</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingStoredSessionsFolders</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0BD5B4A42050983FF6A925BE0806ADC5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>TipsWidth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0C63E1FBEF3CDD124915CB1581864CF8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Printer</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0C653DE11958A559093467A0F351D0DF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0CD01E45EB842462B7B618791F060AD2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsSCP</td><td>#36</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0CD805F2EC14F409BCADD1B8C965C23B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>SDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0CD8E12C86579AF6159D194FA668F659</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>2</td><td>/opt/radex/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0D4DA1689F9BC9BF45C6AEAC36B653FD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExtensionsDeleted</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0D69623D35C41D28DD64AE3338348069</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.1</td><td>FSProtocol</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0D892DC620D899E6F7934CCDE66BB7EE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour13</td><td>255,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DBC960D991F79871EBA0BF504B74A7B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DCBBEAB5C34BE0033B7E047E0131325</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AltSpace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DD25AB51766ACDBFEDB769ED17BCD7F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DE9D457EBD705A42B40559DF301E79C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyMethod</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DED5D60198EEADBD6FF5DD90444D9E2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>StampUtmp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0DF266713ED04F0BD00C5C821BA4C704</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoMouseReporting</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0E668BECF120E1FC1F8F51B0E14C5E45</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialSpeed</td><td>#9600</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0E6DCFE4D3027EE78FB0B0BE5BD432ED</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Bookmarks\Local</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0F1AF32F088DB4CC4995E5F2E7BF5899</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PingType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0F6C24CEA8C293DCFF0F51EEC4278381</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>CurrentVersion</td><td>511037995</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0F8BFCBE61E21725BC34C98E56E2C2CE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour11</td><td>85,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0FC04D752DC013C8E73996A78CC13291</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>ActionsLogFileName</td><td>%25TEMP%25%5C!S.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0FDD300BD40EFCDCBFE614DD2584C654</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LocalEcho</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_0FE3B238745D63472B8EFF5EDC1FFA93</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DriveView</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1006EB62BBE9F4D80B0EA2818D16054E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>Editor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_100CF796501416980017F2B9B591EBF6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AltSpace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1016F33C0BA03144EBC1C8315292F220</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BellOverloadT</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_10BAE0AC2946025ECE1220508B060A96</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogSensitive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_10C9308F05EA1C2F141F83767461D179</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_10CA8B8AB1A3FCD62B019D253010DB7A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoSaveWorkspacePasswords</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_10F6103D239D023DFBE68D8F93D98E23</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>DetectMDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_10FEFF2BDD296C7F8D1BA01FC4B2E2D7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Xterm256Colour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_111E10E4767F91AB16D790ABF67E46D2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoApplicationKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_11CC81CBFBC3E6F23DE0CC6A0A5154CD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExternalEditors</td><td>EXCEL,notepad</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_11D673B60CDF139F8A44A23F335DECC9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsPrivateKey2</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_11E6F9B85197E545751695118F65F873</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BellOverloadN</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_11F3213A05EA978C9C6E408740DE8FBF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>InProgramFiles</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_122592925B6A49F3945F503871B65191</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollOnKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1234066701A1D64BF709F01570360C87</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ConnectionType</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_12442AEA0A5D58E63131D5937EEE86F4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FontSize</td><td>#11</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_12829B07C06F9B69322491C3D0B16D27</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxDownloadTime</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1283854083AE4DABEB116887C2753164</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>TipsSeen</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_12D554A2EB7FB730D1938C56A2016CC2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.11</td><td>HostName</td><td>192.168.200.11</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_12D65478225D2332AA414695771F336F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_137D94100DCBD5B03F176AC2B0B14162</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour10</td><td>0,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1396C5DDEB5137BCBAE420691CE0151D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LinuxFunctionKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_141480475D97A4532500D01F21AD8675</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>Interface</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1444A7C84FD8AF3FC7947EB1681516F4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TipsShown</td><td>#xfde01a635107e540</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_14506D188FAFA31185A113E565702AC3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_14BC2AF4F5AFD4BA7917CFBFE6FA5A86</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TermWidth</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_14F95E7D32E002B537C63F79E5AB2762</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_15176674827C3872F13AD20274B890B7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ShadowBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_152EC963CC5939D1886A2278CABE3384</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PasteRTF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_15579D62529771A593FBC71D73E845EB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>IsInstalled</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_15819E4616F48F671CECB3806125D927</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExtensionsUserCount</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_159E2BD8EA7EB39C602A06392A31787F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AlwaysOnTop</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_15D486378EB99175C8B0DA102753DBBD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountPassword</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_16898ABF75DACB00CD4C2E4844D64DD6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SshNoShell</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_169ED26E1F5B534C09845FCB54E5D7DB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_16A32E9ABBE000C7C858E18E85452135</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>DotNetVersion</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_16AF89ADACDBE4DFB975969C23A5C397</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Beep</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_16CAA8BFE6A88EF8832CA28D6AC9ECCE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateStartup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_16D52D0C7A83146157E0142B83975D68</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsMachine</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1718E3A2C03DCE554F2BBB8F3A61A471</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Version</td><td>#511030000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_173C4627ACFA6F197C6C0C9699821671</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Beep</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_173D7DE05D5D32904FBB592248FCCCBB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_17A0717224A5872C411CA76C3B556229</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Release</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_18471FCEE3B6E41A4EB7A2777E34EF7B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AuthGSSAPI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_184FC8F79AA615C2692D9D769B5F4A12</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\Mask</td><td>0</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_188F67AE6D63087AE977CBC559959FD7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour0</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_18C98943FC4AB8BB32C9442E6CB52F4D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugPKSessID2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_19077210B124A64EF8F66FD1A2BBA413</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1922EC38FD02C0430ABB265EEA4DBE69</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>LastCheck</td><td>#xaed28e718d09e540</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1959263F046B0DFE19DC94A6F354E142</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>Uploads</td><td>#75</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_196E62AF35037F85E66DD55AB7552E47</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountSCP</td><td>3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_19883ABCE6898E9DEF9A307474712BAE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmCommandSession</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_198F6630A310AFB6283F7C002ACFB363</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>HideMousePtr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_19E757130AC6FD6290CE51952626B3B7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>CopyParamsCount</td><td>5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1A1CC1E53617CEB3F5366D51112BBB90</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PathInCaption</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1AD83C9D71AFE05B441659478555A118</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>HostKey</td><td>ed25519,ecdsa,rsa,dsa,WARN</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1B086B58FDA0EDBD3028859047F33061</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LockToolbars</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1B110A5ABC3D488943AD7C708140C727</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>ExternalEditorText</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1B9B4E549321794AD65477F6D0FA4C43</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1BA99C4FE153F1CFF47043885A22D4BE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>Text</td><td>rwxr-xr-x</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1BB5CDEB708291A24C2AC4D8614DA1F1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>UTF8Override</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1C1E60ED641AF9758A5F2249C0060DF1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>BeepOnFinish</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1C2F7322FD8F5C53EDF62A2E6FD8E779</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SshProt</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1C4420C31C621C27201EA93C90EF9CDF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour12</td><td>187,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1C63F74C6E5D82DEFB9C491A4A1D4713</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RekeyTime</td><td>#60</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1C81B50BD85B090445616F736ABF95E2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>UserNameFromEnvironment</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1CC665A30E0CC3934DC5FDE36BA0ED1D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RFCEnviron</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1CEE45F4C5191904F892DDF53F491A59</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyDNS</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1CF5679C344C2CB2E82EFE197ACACF2D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SSHManualHostKeys</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1CF6E0B66C4C42FFCB6809F1A43D34E0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DeleteToRecycleBin</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1CFA5097A1663EABB07041E6AED7A548</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour7</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1D0EEA29AC9E2972B4B95C09E6F2F057</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>StatusBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1D3E29391E6846D8D581848F5792428F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>HasRule</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1D9262ED5FD157E51AC5F88E6ADF3821</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness96</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1DC94316BF1482F928AD092A111621FE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugWinadj</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E00350B76B247A95D94456961A8CC32</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>FirstVersion</td><td>511027781</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E16819937680B973F14FE47E3679638</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Xterm256Colour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E510C6AEFB35A8B5A04D65FDB605928</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugMaxPkt2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E84BC0A285BA747DD503C9D3968B378</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LocalUserName</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E8E9501801130CF2E58CE89181D1D31</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SshNoShell</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E94E93CFDCD5C155D34863EA4928DDC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>EnableQueueByDefault</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1E9D31C8F894D35B7C016149300DA0BD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1EA4422742442298B46CA13A41713D31</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugIgnore1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1EBB42F0755A3F41D2952CE3C09B039C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugRSA1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1ED651F913B9DA5CF1066CD126B7A4D5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BellOverloadT</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1F1D3AF59AD83814446D47764DA771D9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountWebDAV</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1F3492FF71F27FEB9179183066847563</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugRekey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1F622CABEF6AF97F71BECF1E7255E632</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ThemeMessageFontSize</td><td>9</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1F7B9EEEC37E64D12D8E2D88EF46C5FB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LinuxFunctionKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1FACEB9FA99777AF2A6DF2BA4682AEDE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1FCF8FBA669391B577482D39DC02F545</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>X11Display</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_1FE2B3C3376ECAFDDA0C0EC743B735BA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LoginShell</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2030206AB72C30B95D81BD56623FD5A3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollBarFullScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_207C05A9BC0872B90576F9E27F68F1F1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>DownloadSha256</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_207EE7BD07BEB6267AF79514E2FE02B7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2124E8B5A9F44C1B12F1914CDC62D03B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SelectiveToolbarText</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_21A0CF82D317A5956882C75FFDF95290</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>Upgrades</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_21C41A9DD36E0A5127C281E888808BDD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour5</td><td>0,255,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_220C014711EC26E4C034F5741E047441</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\SynchronizeChecklist</td><td>WindowParams</td><td>0;-1;-1;600;450;0;96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2223322A6C27730A639E2AC05709735F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TelnetRet</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_223D81405F4EA255FEF78BBD7F2438E6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_224073F1BC2EBC49DC501F69A8C4ED58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsFailedAfterCancel</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_22B0C64954CB8CC1F78A287A8CE94C45</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>CommanderExplorerKeyboardShortcuts</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_22CBF95E90DD2B34768624D4DF6AEFCB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>BackgroundColor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_232D648C4830B95C83AA6D66E1AFA27C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>SysColorWindowText</td><td>000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_23523C9A3F0FDC815FF48F1FF5534783</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.4:22:ssh</td><td>ssh-ed25519%20256%202f:f6:fd:f1:d8:cc:ca:5e:fb:22:a4:7d:30:18:f1:f0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_23839474FC54278A6E0CD02E5417852C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>UserNameFromEnvironment</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_239DD35F51E473713CE8F356FAC7F990</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness0</td><td>0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_23C60B0397EDED8388ED977FBC07C030</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsFailed</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_23C66BC0A392F4EED35A4C7E7DDB6BA9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>InstallationsLaunch</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2455BA63A05D26922D0D8467458AF983</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Present</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_24728FEE8719F0B032C0BF04B7B04B60</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollbackLines</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_24F0D8492474890B8D7A63E10BA961D6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogMaxCount</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_25074E1A87AA103ABA0984FB223E66BC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollOnKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_250BA1553FC1556B8B6F767D5CB09C83</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>LastMonitor</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_251244E9AFC13E8FC7256458061C39DD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RFCEnviron</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_251F7662321C843A52D29B61E0246A0A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CtrlAltKeys</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_252EDA5E5C14D6D4A58EC94080099567</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SshNoAuth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_253CF37DCB7E879FAC8F82AE0E0F7398</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour8</td><td>187,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2562E289E7492FE25089BF0CEE3B4FC3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugRSA1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_25B9A288AA698F55B673D2EB132EC2A1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>QueueTransfersLimit</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_267E6D129E19C846E7FFFECF4C617ADD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>DownloadUrl</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_26B76CB457B409CD365703DB3EA00B13</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_271BF3ED9176E8E209990F0971264FEE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>SDIShellEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_271C460A14336BC8D5D1B9FEFBD3BA79</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_27C9453F6A76EF2DA1D65D1A179269D7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>X11AuthFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_28388B41C793EBBF3B23DA04DF2927B8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_284D486EEA013B5C2D783801DA03F9CC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AutoWrapMode</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_28952B6788E880675636B3F306D5E301</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RemotePortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_28D858DD0260BA2ACDF31E132E36C4B4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_291A7257E88C5F36BEA17CA18A7E42D8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideBoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_293F466DD33FAEA0E1C4B78454E53EA2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>WindowsProductName</td><td>Windows%2010%20Pro</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_295D916583E61266F7469FC65EC60BE7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SSPIFwdTGT</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_29B862522014D9CB160C055E3CA32E83</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TelnetForFtpInPutty</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_29C05196CEA7B68335F695FD61226D5F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TerminalSpeed</td><td>38400,38400</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_29FE56265AE94FCC6B9564C0B4EACB2F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CJKAmbigWide</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2A69A3871DF654CEBCA120E2DCD2A68D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollOnDisp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2A6D170D6B6E08FCD91650BA5F58BD1B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Critical</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2AA1AF98A56A7C4B439E392F381AA317</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ChangeUsername</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2AB723739D94C1ED41D012330F8C7304</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2AD2AFE96987EFDB0722D0AB3FA24EA8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmTemporaryDirectoryCleanup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2AE32454ED9D6A3E6CE4A8620963CC6F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\LoginDialog</td><td>SiteSearch</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2AE574A1D87DE7E5BD7708A56DF3B60C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour13</td><td>255,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2B29525E97D77C3EE7E809B0A4845A6A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>StatusBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2B381B5C70EE5495A4B45A0A74A283C9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>TemporaryDirectoryCleanupConfirmations</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2B72D18887B9ED617BA369AA26FD18E6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RawCNP</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2BC23E77B5A7EEDE469BF91027AED7F3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TermWidth</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2BD670C26BC85D081F824548C1170259</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>WindowParams</td><td>-1;-1;960;720;0;96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2C1211232753EF9FB98683FF80141027</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsLaunch</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2C6A1E232750DCFA71AEF3D9666FB726</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour2</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2C84FBEFE7A89E14E9BA480A213ED246</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LastMonitor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2CCD3DE7031CBAA8A5811429838ECEA8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Windows64</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2CCDECA4F97B68E42EEB8C5F6F834F58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>UseSharedBookmarks</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2D0DDE658654BABF37F6CC7789ED18A2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PuttyPath</td><td>%25ProgramFiles%25%5CPuTTY%5Cputty.exe</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2D426B1FB33CBBBF620D45ADFFD32467</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Font</td><td>Courier New</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2DA0F5C51EBB5ECE60000295F8D11A06</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>9</td><td>32</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2DB69509E180E4855F8CF93D01543E08</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CtrlAltKeys</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2E3D5506B3037C7A871B9A0C6F5C0A7E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RemoteQTitleAction</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2E5B26A70C1B9869CE47CC876217B7F7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LogFlush</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2EB7AD0E4F2D62B645D120728D027239</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ConnectionSharingUpstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2EC58C0BCB381B148CBF78AFD5135CDD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteFilesOpenedInInternalEditor</td><td>#57</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2F4C08A56AAC996B1C05EC843D258915</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>DownloadedFiles</td><td>#9</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2F4F66370BE9042ED43D5E779E77CB49</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>DownloadingUrl</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2FA18DAAC4F6E3FABDE284A813035B15</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>TabSize</td><td>#8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_2FD3B1D6E6CFD29AB4C485AAB6BAF618</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>LocalFilesOpenedInInternalEditor</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_300ABD36D3F5D210A8B5F29970BF4093</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3016A3D7C7F41C0A12BB92E5C426B5D8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>NewerOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_308AB2D9CB3947E731E5ED4E47102580</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ComposeKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_30A42DF90C8AA24F820E3F9FAB13934F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>UseSystemColours</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_30D78648B19E073E15F23EDE8E705104</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LogFileClash</td><td>#-1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_30E34F77471658A10CD73F3D1BAEC5FC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountWebDAVS</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_30FEC15529088DAFE0FFE48560C21DAD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Cipher</td><td>aes,chacha20,blowfish,3des,WARN,arcfour,des</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_313C3D4185EEDDADC447A3DA0C9258A6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ConnectionSharing</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3171F5CA4307691A2050B58F1D7BC91C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>LastLocalTargetDirectory</td><td>C:%5CUsers%5CMarcel%5CDocuments</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_31806AF6CE89162BF8D1874D179CDE7F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour7</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_31988A763402D5A2EED7405C3D961300</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RemoteCommand</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_31DB9CD483A660D3F8A0996BEEFA7D23</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsFailedRecovered</td><td>#20</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_31E29950BC46067BBB41EE1E075683C7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Protocol</td><td>ssh</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_31F6CDC20B1AB164FCCDD285977EACAA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_32145EA76F2752C51F744CC60D8CE041</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyUsername</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3229F0BA6C7C047C3F77F5A6A6B2ADAD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AuthKI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_325E834268BE0C86CAB084CD06E87054</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>WindowsVersion</td><td>10.0.15063</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_32940C198F1649BC93783948447997FB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_329E14A283C8BD42070937FC5CC1D53D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ApplicationCursorKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3356B23238054EA246669C1EF9C25251</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BackspaceIsDelete</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3368106B41E6484FB9CC6747A31A0498</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialFlowControl</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_337DF378913E9EF24F23D020D26A32EC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>DetectMDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_338347FA202FAA0A9D7444D8C493B9B8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ConfigurationIniFile</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_33B24BA9E190AF5B00DA8169E45C6045</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyUsername</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3466ADC920B79A1FDA18BEBB1558BA6A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\Mask</td><td>1</td><td>%2A.csv</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_34A32E7F412C2207CAA798A8C6A46828</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>MaxEditors</td><td>#500</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_34B72AF7B685F419583E628D9B84585F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PingInterval</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_34E19280611AC8479D7955BB0CAFC6BD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FontVTMode</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_350AA295DEC6862446AD2FC840469F58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateDownloadOpens</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3570671AD88DB4815EFBB24DAFA30668</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FontHeight</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3581D326678DF8F812BC55079A765800</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideBoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_35CE2CA4CF715C2E79238FE6C0364865</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>MonitorCount</td><td>1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_35EE1E1570B56EA01018A38E53BD6C1A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_369560C6D0EA4EA809CC24F3AEC6F6CA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DriveViewHeightPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_36B4C870572AA0DE222A735AD5AB8F78</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AgentFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_37000C4435033E1C89223919243D4348</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour18</td><td>0,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_37196B5568AF53DF4DC628E84401C9E4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Bookmarks\Remote</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3751F50B8D789D93442ACE79FE5E810D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Period</td><td>#7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_376D0596FBCDF4E3B3F47C98BFC3C3A4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TelnetKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_37794ABECE3A613F14C98E5D8C6F216F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ShowOnStartup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_377A715668D468B1038B59E08189B484</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>DisableArabicShaping</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_37C1CA13B4E132C81E57E2CEA74A100E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>ToolbarsLayout</td><td>Queue_Visible=1,Queue_LastDock=QueueDock,Queue_DockRow=0,Queue_DockPos=-1,Queue_FloatLeft=0,Queue_FloatTop=0,Queue_FloatRightX=0,Menu_Visible=1,Menu_DockedTo=TopDock,Menu_LastDock=TopDock,Menu_DockRow=0,Menu_DockPos=0,Menu_FloatLeft=0,Menu_FloatTop=0,Menu_FloatRightX=0,Buttons_Visible=1,Buttons_DockedTo=TopDock,Buttons_LastDock=TopDock,Buttons_DockRow=2,Buttons_DockPos=0,Buttons_FloatLeft=0,Buttons_FloatTop=0,Buttons_FloatRightX=0,Selection_Visible=0,Selection_DockedTo=TopDock,Selection_LastDock=TopDock,Selection_DockRow=3,Selection_DockPos=0,Selection_FloatLeft=227,Selection_FloatTop=445,Selection_FloatRightX=0,Session_Visible=0,Session_DockedTo=TopDock,Session_LastDock=TopDock,Session_DockRow=6,Session_DockPos=0,Session_FloatLeft=39,Session_FloatTop=160,Session_FloatRightX=0,Preferences_Visible=1,Preferences_DockedTo=TopDock,Preferences_LastDock=TopDock,Preferences_DockRow=4,Preferences_DockPos=0,Preferences_FloatLeft=0,Preferences_FloatTop=0,Preferences_FloatRightX=0,Sort_Visible=0,Sort_DockedTo=TopDock,Sort_LastDock=TopDock,Sort_DockRow=5,Sort_DockPos=0,Sort_FloatLeft=0,Sort_FloatTop=0,Sort_FloatRightX=0,Address_Visible=1,Address_DockedTo=TopDock,Address_LastDock=TopDock,Address_DockRow=1,Address_DockPos=0,Address_FloatLeft=0,Address_FloatTop=0,Address_FloatRightX=0,Updates_Visible=1,Updates_DockedTo=TopDock,Updates_LastDock=TopDock,Updates_DockRow=4,Updates_DockPos=416,Updates_FloatLeft=0,Updates_FloatTop=0,Updates_FloatRightX=0,Transfer_Visible=1,Transfer_DockedTo=TopDock,Transfer_LastDock=TopDock,Transfer_DockRow=4,Transfer_DockPos=194,Transfer_FloatLeft=0,Transfer_FloatTop=0,Transfer_FloatRightX=0,CustomCommands_Visible=0,CustomCommands_DockedTo=TopDock,CustomCommands_LastDock=TopDock,CustomCommands_DockRow=7,CustomCommands_DockPos=0,CustomCommands_FloatLeft=0,CustomCommands_FloatTop=0,CustomCommands_FloatRightX=0,PixelsPerInch=96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_37DAD1F65F9198B210E9B844AF130A58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3817B36AEF62F3479623076C865D59D8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ConnectionSharingUpstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_389A3B516DDFBE48F0E1B929022C0C57</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogFileAppend</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_38E194E18A5F902FED71482C56BAFB41</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxQueueLength</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_38EB543A70FC5DCFF0A49761E53812B8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>HaveResults</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_391CFFB63FB19D7896E43C901FDFF085</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_391D69613832671539741842AC9D95AD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.11</td><td>PublicKeyFile</td><td>C:%5Cgui%5Cid_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3975EB40D2F273CB82CD1FC783B1C6B3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3981D2B21F437FB84379535E2A0B203B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ThemeIconFontSize</td><td>9</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_39DFF2FFB766EBC208AF2BB4F32CDFE4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugIgnore2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_39E2FA1F4DCCB70C087DA1A8C6B4FB15</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>TransferMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_39F884EBACDB15C64162DE374F55E745</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialDataBits</td><td>#8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3A47D17924CE98F6012D5D94835F6779</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PanelFontSize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3A964E1C7A0314E98CEBBB37B6562AA2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness128</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3AD0E5FE7796D9B6030A4710439D785F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugDeriveKey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3AF747AB692FB679301BA08202AA9460</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>DriveViewWidth</td><td>#180</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3B078AD2779F498348539C7A63B337D2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PortNumber</td><td>#777</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3B909FD5D510607804F70FE69B1EC1F7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>OpenedStoredSessionFolders</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3B97CB52FD87F6911AAAE112E38FAF3C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SSHLogOmitPasswords</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3BBA490B1582DB7B1E0FA26C2376847A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PuttySession</td><td>WinSCP%20temporary%20session</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3BE91F4740458C7E3DCDAE296A1496E0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Answerback</td><td>PuTTY</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C25516691228D6D342B10CB865BBB1C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogFileClash</td><td>#-1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C3253732F6680C8AD0B6AB1D3BB84A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration</td><td>JumpList</td><td>192.168.200.1,192.168.200.11,192.168.200.9,192.168.200.4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C460CEEBCD485D85A9F4939CA878B31</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\NewDirectory2</td><td>Valid</td><td>#x00</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C502E4181ECC9799B7B879214E945EC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Installed</td><td>2017-12-15T10:53:56.937Z</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C5C70F9AFAF766A4585CEE01A94E6B9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionReopenAutoIdle</td><td>#9000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3C8558F82B6F92C6A84148462B789E2F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>RunsSinceLastTip</td><td>#42</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3CE9B8B0FB3FCC8A80B0F189883A0740</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour3</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3D0A3983E1DDAB5E27A039BC618760A1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness32</td><td>0,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3D111481A3038A6F1301F2E444AE4130</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TryAgent</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3D11AF142CE1D0AB3D8299C50CEC5170</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3D5963D31CA442DBA40EE13B22600EEE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ProxyPort</td><td>#8080</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3DE4338EE56F0D086C32279D19D3DE8F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BCE</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3E35090EF9EE963AF40D905CEAD1F10E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AuthKIPassword</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3E500C5C8DD8088E373AE0F485D80B9A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>CustomCommandsCount</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3E5DC26E3F897B37E4D249BB62E93312</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>NewsHeight</td><td>#200</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3E7AFD7ADD8B59AD290B6B973E0AF0F3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour11</td><td>85,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3F126DA5675B68C6A7E896D224EFCD6F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>X11AuthType</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3F3FCA1C060826768A4DECC169192FDF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ChangeUsername</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3F7AE115890AE1214A17EA2C9708A633</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>TransferMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3FA2C440B55524566161C3CE8F32E967</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>X11Display</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_3FC770B54784C8BD3B21FE4A9B662EDD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>GSSLibs</td><td>gssapi32,sspi,custom</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_408595246AB6DE872843D5EDE579E956</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Protocol</td><td>raw</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_409CC5C33CB2B5EBAC473240470F5616</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>CommanderNortonLikeMode</td><td>2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_409D73892348F5AECF10C29AFE338576</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoApplicationCursors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_40DBEB7D433C76448605F72501AFF4D8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WarnOnClose</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_40F28A590486C35A6A5A9AD75A7D6CC6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PassiveTelnet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_412132FCB30234B78C1265A645463F95</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>RemoteFilesOpenedInInternalEditor</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4165D1F7DB7526E2AB6C74D04FA44603</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_41B1A5690C661B7EF15C9DB75E30598B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour19</td><td>85,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_41CBD67296BF7612EECA294ABC0E9F75</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour10</td><td>0,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_42225F54739742F85F354389EB2A5291</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CloseOnExit</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_42529E80311E3E10E9D5917B861B3793</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TerminalType</td><td>xterm</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4336D5145B78D0DF9D585D47060EBE78</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>ExplorerStyleSelection</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_434ADB86A15CEEEA0743698A99E32394</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FontCharset</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_436BFD9961DF0EDAD9C6E8363C140F97</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>LastInstallationAutomaticUpgrade</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4397A3C4ADCF2F8C9F1942922DB64EFC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugDeriveKey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4413986068C8033C307904B1ABFEED1F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AltF4</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_44224096A4B5B1283826B0C16238063A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideBoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_44AF70F62E1BE27D44C1E9A500EBCE5C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>GSSCustom</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_44D765C4AEAEC834AD231E5EAAE4DD88</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugPlainPW1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4572F53F62753F5CC4443BEFE6FA7B48</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>ExternalEditorText</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_45B2FE379909D92E62D8237F5C9E2DD6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateChecksSucceeded</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_45B4BE5B191DC900CE1412B99FC521F9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>MinimizeToTray</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_464BFDF1D25C8A9A2BD03DF536A31751</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Font</td><td>Courier New</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_46D371CDB867B999FB453FAA647576F0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CurType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_46D9241915FE94243A2CDF509E8EBA56</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PortNumber</td><td>#22</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_46F30327278EC15B15983047C46BC611</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideBoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_471D897740409B826ABCDD427DFF274B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>MouseOverride</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_472E28B3B310DD6F00CD7EF2F38A2DA0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DriveViewHeight</td><td>#100</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_472F391B2E3BEAE32BC4AAF5F5405A86</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateDisplays</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4768684EBB3500E691EFE1B6918C62D4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugWinAdj</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_478544722F36BE12997675A4120036BC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RXVTHomeEnd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_47CB014F1618760DA10E1A1215E15C05</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>DriveViewWidthPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_47D5966CBED6864747F6DEC6200C46D8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Wine</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4856D326457A06C018DB0E8C5EF82ABB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxQueueLength</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_48638EA27444BEFC6EAA5AC41B0D9030</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDExtEnabled</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_48763C44F25B0FC42D229BFA30B20575</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour10</td><td>0,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_489D7343910FE54E8FC425A1FAA163A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>PixelsPerInch</td><td>96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_492C04A1F270A0EB0F498215DF31FA9D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugHMAC2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4944BAE757EB674109C8D1709EDDA04A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SshNoUserAuth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_497E3D785D213A06E8D80A80C44465A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmResume</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_499CFC2A990C55F2BBB0F85EA6AC1CC6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>ShowFullAddress</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_49F2FF8F35EF375A0D2D115EB71632C1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A0292219BD1F3D35D36DAC6A0D9ACB3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ShadowBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A217F37A140927F23E3F9E0C4280F14</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>OpenGettingStarted</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A319C64A9D6C44038A5BFF60D8F869B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness32</td><td>0,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A3880BB13415FE3721F5EEEF4D350A8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>Masks</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A3FAB0EC1E8A4226B676CA83CFDB593</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A7C72F0EE5AA4A62E25BF4D4D56FE95</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>2</td><td>4096</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A808997006D88CD6F27899C56E0F722</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>PixelsPerInchMainWindow</td><td>96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4A97E1A5BD0B4B12F2D2FF3BCB0584DE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BellOverload</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4AB9295D47D45917829A39F9C47FA3E0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour0</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4AC42FC3D2CA8E2296412DF40CD3EF82</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Url</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4ADD11FB5FBAA3C349BE50059B87E723</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>PowerShellVersion</td><td>5.1.15063.0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4AE3E576C14AEC7B1044DBA69D149129</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollbackLines</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4B00870E68CF1AE6CC5F04C14CFC7A3A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>GenerateUrlComponents</td><td>#-1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4B4383392D4414D17DE9C64F41170BE4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PanelFontStyle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4B9D01AE5432159734D69FEDDDF11279</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>StatusBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4BF4DD27B94DA57DAC4EA8BDB2787D9A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollbarOnLeft</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C3F9A4F4D324A6BA287FDDE972B8AC8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>SessionsTabs</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C44F51C8290A5E6CB1CC8DD6E18731A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.11</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C49F03AD90B32E396C0C1B49A74AFB6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenInPutty</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C51ABCB85B1EE846C5C02FA11D39C66</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LocalUserName</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C6F99C3AD6B37187CB0DF8BBDEB5C18</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoImportedFromPuttyOrFilezilla</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4C859943CFA10D2D96FE60BC3C64D424</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoPTY</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4CA5E8AAB957A34730BC15BA95026F32</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>MouseIsXterm</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4D6BB85BCF5AFC8256972615C97C20C4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FontQuality</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4D962BBF2FCAEF98EF4120EDD34A5BDE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>GssapiFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4DA4C7CF97606C5B880DCA7A1366A77C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>Name</td><td>%EF%BB%BF&amp;Bin%C3%A4r</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4DB9C4EB0D58444A96F0ACF2CA24498E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideBoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4DE9CFC8A777BDD31F744E010C7AFC9B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyLogToTerm</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4E17283044624654AE307B09280E8B72</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>HideMousePtr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4E1AE2DDE4ED8EA915F70ECEBB13A6A1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WindowClass</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4F1747A980259484AC9A46759CC9FD59</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness160</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4F89087574252F88A527FDDE4FCF712C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugPlainPW1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4F93A979118C0D8A066CF82D9671355F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>DirViewParams</td><td>0;1;0|150,1;70,1;150,1;79,1;62,1;55,0;20,0;150,0;125,0;@96|6;7;8;0;1;2;3;4;5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_4FB9C4874FADFF09D0026055C8251B59</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Present</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_50108906F6B57E51DEA45E81B5C8DF21</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialDataBits</td><td>#8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_505409ADBECE19479430CE1B27407070</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RemoteQTitleAction</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_505AF40CE3D192D38930B08D392379E3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AuthTIS</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5080D3228A6770FBA86B927179E743AA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>SwappedPanels</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_50F7C8A0E909256C21B902062E804AB5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxUploadSize</td><td>#181119</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_513ABB7CEFB609ABCE65B7F59FB98653</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialParity</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_51D04F55E0ECABCD5D7D64FE4FDE702F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AutoWrapMode</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_51EA1098505E417B16CCD03CDBEC8E81</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>HostName</td><td>192.168.200.11</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5235341E7F1B7DBAB17FDC39139B98EE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LineCodePage</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_527A7023DEBAE6CAD9D0DC0C18B68D98</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LocalPortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_52B9F598C16BACAD51318C89B33B1A77</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ShadowBoldOffset</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_52FB6E68D9725FDA8B6ECA3F9525BD10</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsUser</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_53DE5D8CEB163A715332978EF3AA5248</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour16</td><td>187,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_545F67F58033646E2EE0F96F39CE7994</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsFailedAfterCancel</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_546B55EBA2ACE0C02818664AC628766F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FindText</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_54C2AB3D7B31C1DC334E9FC25D24C983</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour3</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_54D11295551E13D27A0EA5EC269CD7AB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>PreserveRights</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_554EF1EA0E5EE8788BB0396EFE6F1421</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoMouseReporting</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5582DA59795DFC6067727433A026F691</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogActions</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_55A3D74AFD283B4E682A1A6B0FB439B5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LocalEcho</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_55F47B2A9CBCFBD9C753C66B34EFC7E0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness0</td><td>0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_56079B849C66E91C1E333BEE8F251522</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>3</td><td>/opt/radex/bin/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_563527339B031D557117E6ECD886BB4E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DimmHiddenFiles</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_56361096E6715C7B11A005232133BE8F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\FindFile</td><td>WindowParams</td><td>646,481,96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_56747FDC7505ACE59C9A05CF5FAED8E9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>NotUseThemes</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5684C14E56C08D7B90FF2556CB50BB69</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Tips</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5688CA33A71BFBB2C288CBB770F069DF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_56967030DB5F086707FC536B9EDC6926</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoRemoteWinTitle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_570FB4AEF9FF53E8C228D0FE27D4EE74</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BlinkText</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5731224FACCCFFE547E3AEEB6839F220</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TermHeight</td><td>#24</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_57600338D54D16EE5824C116147B70F9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.11</td><td>0x4b81c5769f1d16906fb3745dd6febe80d62f8df2e35356a2f649abbaca869f80,0xfd055d07242fea2a5d63cf531f4c0087424ebb9a91780af11630eebf779ae6f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5778269AC9C8D0C6EEA4193D3F91F99C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour17</td><td>255,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_578086C26A9620DAB5EF9336574D083A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>HonorDrivePolicy</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5797020F1647C484691EF7995ADA1C78</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>QueueRememberPassword</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_57A079EDCB7094DBB97E9179FFDA80BE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PanelFontName</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_57C60A98EEBFBED8C5BBFD6BAD76E293</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>10</td><td>16</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_57F08FEFB77371A2D8461D0BCCDFD210</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoRemoteResize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5805CBBB24597C43588AED77C4C1C680</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmClosingSession</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_58139AE6B8D927754847914AA3001D0B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Answerback</td><td>PuTTY</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_58FE05016E5FB4485409A48FEC59DDBD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideBoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_590E0623E0CAB13A9A3177E57AFEB707</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialStopHalfbits</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_595CD3B7CE29404824DCA000F7F2C21C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>FormatSizeBytes</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59667E7048C016AC219653FAD3FCBB9B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5997C99E4137A376C6AAA304A68F9944</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BlinkText</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59A59ECE8DA4F0D4A948B370DEA7A0A3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TemporaryDirectoryAppendSession</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59A96036771DBA6BB1AB61C949AFCD06</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AltF4</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59ABD5888FFD89D4E997547AB81440F2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Environment</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59B9F533DCA9A64814BF5D97E7A9463A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ShowHiddenFiles</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59D4DD40575989424BD8485EB2B18FF9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_59DE2657BCE457217906C0AAC7BA317D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>AuthenticationEmail</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5A015339D9CBAF88788C4215FE0DC815</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>DisableBidi</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5A03877750FAFB9A3684221406FED778</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UploadedFiles</td><td>#183</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5A5DC8090861A4353D55A420BA8F193F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>UseLocationProfiles</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5A664A53BF6A279C267D0F2F69998C0D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Present</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5B1D5525F61F9048693E5081E74AEF1B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Beta</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5B802F097F35B4EA07A62ACD01063906</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AutoWrapMode</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5B9223CF1EB4007D956408032583AB34</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ThemeDefaultFontSize</td><td>8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5BCDC3938B67C85A62433D24E98E8304</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WindowClass</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5C2F87D2E044C4FA41CB91E7609CB58A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SshBanner</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5C396F5BE632D6B54319A28349404328</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TelnetKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5C6B90B031F766A54445B4141DF5161F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>NewerOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5C97248E1EFC4EA2DF2584F03FEBD689</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness192</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5CD1988C1B9AFE301970DCC870B4B494</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour18</td><td>0,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5CED849169449EC362CA3BB3A06A32EB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyHost</td><td>proxy</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5D6E4A274B664862EE6FE1F036005720</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ConnectionSharingDownstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5D8061EEE0895B34E1C64FA1E9E7F55C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5D80968B3380EA507340C7DB8D87FCEC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>ExternalEditor</td><td>notepad.exe</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5DD6FA6EA2FC8588B9872B28BA1AD10E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoRemoteClearScroll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5DF027B9AFEF5F15334ADA39493CBB47</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TerminalModes</td><td>CS7=A,CS8=A,DISCARD=A,DSUSP=A,ECHO=A,ECHOCTL=A,ECHOE=A,ECHOK=A,ECHOKE=A,ECHONL=A,EOF=A,EOL=A,EOL2=A,ERASE=A,FLUSH=A,ICANON=A,ICRNL=A,IEXTEN=A,IGNCR=A,IGNPAR=A,IMAXBEL=A,INLCR=A,INPCK=A,INTR=A,ISIG=A,ISTRIP=A,IUCLC=A,IUTF8=A,IXANY=A,IXOFF=A,IXON=A,KILL=A,LNEXT=A,NOFLSH=A,OCRNL=A,OLCUC=A,ONLCR=A,ONLRET=A,ONOCR=A,OPOST=A,PARENB=A,PARMRK=A,PARODD=A,PENDIN=A,QUIT=A,REPRINT=A,START=A,STATUS=A,STOP=A,SUSP=A,SWTCH=A,TOSTOP=A,WERASE=A,XCASE=A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5E1028FEF2F0D6C2C338B384100BC132</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RemotePortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5E61C8745A06A0737F5CCEC26186CDCB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialDataBits</td><td>#8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5E7383758600047234B9B736C74BA5A3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RemoteCommand</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5EA81C111D3570983BBB890EFD3E5591</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour1</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5EAAB77309AA062CD5ADD8CCCCD3FFF2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.9:22:ssh</td><td>ssh-ed25519%20256%202f:f6:fd:f1:d8:cc:ca:5e:fb:22:a4:7d:30:18:f1:f0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5ECBD5FAA4D10C95100E48022063D382</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>NewerOnly</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5ED987F48C6F7F0FE44E6549E76CC76C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmDeleting</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5F15F0257F917AEFFAB58568C8D46DAF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BellOverload</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5F2EA77AE2AEC6C63FFD758F711C144D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5F49F0DEBF688D639BE8111EE0FE003C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoRemoteCharset</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_5FEA3E513DDF11F82A8DD5431B0E68F9</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.9</td><td>0x22d26096fc9d947258b94c68c746d25bdd82c91a393e39c5ddeeb49bb650297d,0x26c59a16cf8d67065af619b4d2ce30a2139e512797f74805196c6c13bb57ad63</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_60074A34B549ABF5C3D8C116A817C03D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DriveView</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6094D81443D3BFBF742EF4834DB16A79</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LockSize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_60E2E49D8D69B94007BBC945C0B834F8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness160</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_619DC5242355BAC6134C37D87A7BF093</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsSFTP3</td><td>#17</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_61ABEF689CB08E2CBAF189F1273794E7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoDBackspace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_61BE19EF70F26982CE76EB403FC9A3D1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoDBackspace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_625D8C5B79A14B63FEE2328EBCB94104</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RawCNP</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6281855EA1A13E63C437B940D2453956</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BlinkCur</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_63574588DAD916C24C338349B8EC2806</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LocalEdit</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_635F30DEED1DF656663C5AE00994F0CE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PassiveTelnet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6374BE2E699F551E89A94936CD89FDD1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_637EFF948E6EBD132E39CBC1B6C38908</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AuthKI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_63D8BD7A967C62049CE68E343A18A7AE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>SDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_63DAB99310D48D7D2B17A5F1C1D6F722</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LoginShell</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_63E0F0DA34E020E26E185969E3E7F412</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TelnetRet</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6453717238185701552E8E893228DA32</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_645A362A9FBD3DBE41C34363B456D389</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_647B2D8413BECE5AE516D473D2B6C519</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollBarFullScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_648AD13E1436DFC671567E84DA78FED6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsSFTP</td><td>#20</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6499F38AAE8BB17CA26E63B419F0794B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PortNumber</td><td>#22</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6516A095093AAAFE25A5143145053D19</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour20</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_652881E3091FFA7846D235D823F80ED2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>TemporaryDirectoryCleanupConfirmations</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_655B175F4F45B881E086C129EAA5CD3A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BlinkCur</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_66430553FE3C9463AD1DFC486141C70D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoPTY</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6644FE26A2EBC7AF15FFE193B8AC610E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugHMAC2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6698484033D05D2DFD760222307EB74B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour12</td><td>187,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_669E3D423DED05B9CF9C01DC9178A19A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>PreserveLocalDirectory</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_66CB90F33CEB2EC4EF1A8205D027BEB5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LocalEdit</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_66D6A3D877FA9C4DB2DD19E65297CD0F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>WindowParams</td><td>953;1;1919;1040;0;96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_66F11A2A3424E558A6A1ECF4CE312051</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_673BCD9D0D3EB24E60202365CFC2B524</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoAltScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_67963F5B65B432E7DA94BD480BC77638</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>8</td><td>64</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_67CADCF00F2BDDB029B44E602EECD371</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugPKSessID2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_67D079C40A1C679E629ECC91FF2B734C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>SDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_67FC5A11C075FF6CCC7C7DBF83E3231B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AuthKI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_682D7D338038C2D788493F812C58AD68</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollBarFullScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_683733179C909FBA34913FBAFA151584</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ExtensionsOrder</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_684B26751CF688BF180F64D77F68D04E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>DownloadsCommand</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6859850A91CE04088CDF7BA3BFD1C7DE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>DisableArabicShaping</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_68AEF9024E16C327F14B11478BAE9C5D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PingIntervalSecs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_68DD9794E528D923F54C9D2A15E9AEE0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ConfigurationIniFileCustom</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_68E2525D22FDDC6D6785560204821C5D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>GssapiFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_693F68305D4E3626D4FD82A0FE092791</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\LocalTarget</td><td>1</td><td>C:%5CDevelop%5CRX-Project%5CProject%5CUbuntu%20Develop%5Cusr%5Clocal%5C%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_694213D6A79F4B8AC34DA106B9BE15C7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugIgnore1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_69446C07DDC72511C7FEA67DB30B65E5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LastMachineInstallations</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_699A3C53AA63E03EEEC681AC1F8F2309</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugIgnore1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_69B70FF3E5C5EEB3E69C033AEEF8EE56</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BellOverloadN</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6A079E1545F7E7E54AED2936E766FD87</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TerminalType</td><td>xterm</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6A2B93FD5718F9BC3F1B11D841DFEB4C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>Editor</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6A35AAC448A184F18CB7BD415FC9A12A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>IncludeFileMask</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6A9566158D587E6C6A0688D31AAFD1B6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LocalPortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6AB6241694C5D7607CB81E6BF2872186</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>PreserveRights</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6B5991D37A1E79E01F0FD57C7F5301BF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\LocalTarget</td><td>0</td><td>C:%5CDevelop%5CRX-HEAD-Project%5Cbin%5Chead%5C%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6B7BD7E5F9EF26796CF4C233B1676658</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6B9424AA12F5508F15A5914F42A41C8E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6BA1208D92B0CE5C88ADF675E857D3C6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Compression</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6BAE5EC98E90CCDD1BE0360C51BA680B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LFImpliesCR</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6BC4A2F80037CA67974D7D8A68A7705F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6BCEA96B4913F16938FC326CD077081D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DirViewParams</td><td>0;1|150,1;70,1;120,1;150,1;55,0;55,0;@96|5;4;0;1;2;3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6BF2D7066F00DAA7FABCD98A9DD2A75D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SshNoShell</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6C947A913B45BFAEE08FBDA3900B38CC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LogFileName</td><td>putty.log</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6CC5320BA9B8D00784C9F6634B44118C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BellOverload</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6CD2BA59A11A37DB8E9F90CD575FB8F6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FontQuality</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6D05D0001401A2512555B3F0C5D1EBF6</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.112</td><td>0x52ab6d39f5671a435b58fe855afd5c0d052970c2020d5a3e48e7c03bc2c93682,0x65d3592100a648ea7b9e269b6f3ad5633d9c781670ecea400429bd66150e3749</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6D2AF66E7BD8D64741639139D9D59252</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CopyShortCutHintShown</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6D84C17C88CAF7C573CE4C673B3F74B9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>IncludeFileMask</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6DD6A7F12B939653208497DF4A1C761E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>CompareBySize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6EA5D1B9E37660DF639395D2E36B8448</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyExcludeList</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6EBFD26D073CB4E2213E8607E612E129</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CloseOnExit</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6EE86AECFE34ECB9FBF0374D3CC295F7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>Masks</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6F3AD16BD030ED58EBFA0B95EEC33E8F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoPTY</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_6F3EFD78FDC7F09A39EC2FDF327F3A9C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>ViewStyle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_700B5223D40818DFB260F11AC46DFCDE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour21</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_700BFB938C74195890D0E17F26D30E48</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_701144A9C4B2A174D65A34871CA47546</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugChanReq</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_70287B961C4FCEBB0B5808AADB47E7CF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>IncludeFileMask</td><td>|%20%2A/</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7035A906F5D4A1FD73CD8AFD0985299E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountNote</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7049D9ABC9AC0D349F7F09B41384A84D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ApplicationKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_704CFF658616B843B3C3F1C6AF7C951A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>Downloads</td><td>#70</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_70AAAC4838A063F7E5057F43AFEBB13D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>InstallationsMachine</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_70B7909F391BC6924BCC68AC7A83A792</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsFailed</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_70DC845641672EE1EB7F8F0712E67E7F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ConsoleVersion</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_70F856C0941FFFF71ABE68FFD3804F64</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoRemoteClearScroll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_713CE0B938B8A3A8C45AE0090AB5992C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_718128C4E28CA15DAEAE06E675C52B5B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>StampUtmp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7195AB9A3417CC1A774F3153FAAE483E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>MinimizeToTray</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_71AA9D78290D4A866C74505A40E0AF16</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour8</td><td>187,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_71C3DADFC4AB591C4CFE9C786F6521E5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ProxyHost</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_722420550E5603719DCE517106E4C7CC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UploadedBytes</td><td>#44230</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_722D90DE2A528366DB7B13F3F91822AA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>EraseToScrollback</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_729E2F74AB876E7E6C8B182942221F68</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>LocalPanelWidth</td><td>#x000000000000e03f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_72FA6705C44ABA9477481A3D0683FA68</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>AuthenticationError</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_73873937B4B2CF27B0C4AB30FC97FC15</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FontColor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_73C50851CC6C867E5E0D4913BAB94CDF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>HostName</td><td>192.168.200.11</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_73CF1A3394A17DEADD845CFD2741C8EB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingAutoSaveWorkspace</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_73D13E03269F1CADA17396A052BA1416</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour6</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_73D18312310FE0ED158CADFF8947C9C8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\EditFile</td><td>0</td><td>%2Eprofile</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_744EA1C0B7EC8455F9113F98128AA904</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AltSpace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_745D398098D42041CD5E63DFC79BAB3A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ChecksumAlg</td><td>sha1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_74E6C64C60104E404E8D13A7F75C3D3B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyHost</td><td>proxy</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_74E7BE8381913C75CAF1764F769375B0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ParallelDurationThreshold</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_752573EED5C584C639A17625F6AB030B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoApplicationKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_75526665085DBF6E6079C16450B4F2DD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoRemoteResize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_755DFF684FFF93B6A0A639FA2EFD2E43</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>X11Forward</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_75884CCCE3854C3FE24CC5A78E09BECE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>GSSLibs</td><td>gssapi32,sspi,custom</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_762FF21E631C40D0C192235A479078C9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PortForwardings</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7641B660D8FE110932D1A84A7D860FB2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_76585C222F0212820714E96D9ADFE17A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PingIntervalSecs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_767A97C5A609BBA04F95DA30D3D23197</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7711A2C537CD8B45D1F3273DAD425C69</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TemporaryDirectoryDeterministic</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_772095836DD0C5E087644965C2B2AE48</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DriveViewHeight</td><td>#100</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_775939D1069CB787168F71C0E94EBFE7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LineCodePage</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_77B8113808E3120B56197BBC84053000</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RectSelect</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_77BBE9892B9A23D4D309D5178E157217</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_77D5FC7CF585743A5E281270AD1E11DE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7861895BFC6D2C7539581A5411DAAF6E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BlinkCur</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7865E7899528566FF3BE81E93BF21841</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugRSAPad2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_788601F6DFEB5BD76C69836399761886</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\FindFile</td><td>ListParams</td><td>3;1|125,1;181,1;80,1;122,1;@96|0;1;2;3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_78908AC211131241F9122AAAFF4C57F7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_78D27261F1EAA7C74C6A3B4ADF21C5D4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WinNameAlways</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_78D8D7D261D72EECE3A98B7733B47F7A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>EraseToScrollback</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_78D8F2C989390F1CA482BF0D2E7F0C4E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>DownloadTime</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_790230EFE146E8336F61F8679A54E704</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CapsLockCyr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_793FE8C8AF2610B0A1E18B5B6AD31505</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Interface</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_795A309FC4D6F2277D2B5F0F20BA6E8E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>Editor</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_797A93A7A9C779413F3E0B1FF9DAC419</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ApplicationCursorKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_798A9D02EBD867C14E96AE4B8B73DD91</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.124</td><td>0x7ddadc365ec6c340a0795fa9a34a4a56839954b3b9ddf9e36be9c3152dde87ed,0x331d3675f7314cf4456eede6be33163ae868b03daaf51dfe2a6d78a95ab2e06f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_79A2080D43A432643D87AEB0C0DCF1DC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour17</td><td>255,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_79AE5253E8062AD5C044DD1E1861DCA4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LogHost</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A086651D3F68CB37D2111529635028D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideBoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A1DDC65901B8E56CE3854E3F6A22501</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionColors</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A29A6BC1466039ADEC33FDB34BD925D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ApplicationKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A3476391CF875FD4F6A96F82E9D09D9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TerminalType</td><td>xterm</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A6450F55E5CB226429AA84E706AAD21</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteFileSaves</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A71CA605B3ACBED960F49BE9DBD35A6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7A99566DA924F23F3378F92F67A94C5F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyPort</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7B1CE51E86E27E72C6F961F788561BC7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7B775277836F4160B16571A36D52A468</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Timeout</td><td>#15</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7BBCA7A0D67D99626CC373C0CD7E24CE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>1</td><td>8192</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7BBEDDF6522735DBD051C188A9EDBD7B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UploadedFiles</td><td>#40</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7BFC65F4FB29F9CCF0A5B2A3619A3FA3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoRemoteWinTitle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7C0CD37D5C87B12328C8CB8040F7AB3A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7CB0C2FE41189D773083DC65015FB8A6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7CB6668210DEB3C4641A789234F74DA5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>FirstUse</td><td>2017-11-27T13:02:25.246Z</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7CBB0BF3DC6119AC877E7468C5BCCC01</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoDBackspace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7CBC3A0729DA07F1D8821286EB691948</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>DefaultLocale</td><td>0807</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7CC91D2F43EC3E0C79737AAF3640D343</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>PreserveRights</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7D15FBEF6163902FB719246D033DEEE2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugRSAPad2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7DC385D9DA89D78620512D958948AEE7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugIgnore2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7DDF98005C553E91F1BCF32D280AC69E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness0</td><td>0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7DFC7FAFD0CF3EE426F451500811BB7B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>Height</td><td>#140</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7E4211108D203C97723DB0E305B07A07</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoRemoteCharset</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7E7975F176C0205AF050E8D7483FC2EF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>Name</td><td>Verzeichnisse%20a&amp;ussschliessen</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7E82B16C95FBCB93CBEFDC56420533E0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour14</td><td>0,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7E89CCC85029D5F14D8A91002BC368BB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>TipsNoUnseen</td><td>#6</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7E8C60376DAE99F81C1E7FDE2BD417AE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>EraseToScrollback</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7ED5538EF3C79274EC0DD03E954B5EBB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7EF0C89E261CFF0940372818252097DB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UploadsDragDropInternal</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7F31FCEBA3426A7ADCF10DF481A1F702</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TunnelLocalPortNumberLow</td><td>#50000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7F6F25A679A88F7367327113969397F5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TipsSeen</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7F71AA10FF598959C28A72213A1FEFF6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TcpNoDelay</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7FB1E4014B19310F30F00E620611066D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollbarOnLeft</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7FD1D2AD29CC0D4BDE1AF33BB6EC904D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>KerbPrincipal</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7FF270E9D6DDBE38050D656A4714E97E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmTransferring</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_7FFB613A1F7A04B25E358063353CB4F4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour12</td><td>187,187,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_805E42EB41579BA3575F28F594E6421B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour20</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_808997803328C98E61100DCA5A81EDA0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\HistoryParams</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_80ABE1A4AB8DC5EE0F2B6992B89034F8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UploadTime</td><td>#30</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_80B7CF5F7E1658DFAE96AB575FF43E5B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.1</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_80C23D29A563EB0F339B75CE0CF002EF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_812B1805451847C5043F41E93795497C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateChecksSucceeded</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8152565979854FB62391DB8B13AD3046</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RawCNP</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8178D3256A6EDDB4C64DB6E57E9F3BD4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountSFTP</td><td>4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_81814C94E9C0F01522F3E00F2B3DFE8E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollOnDisp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_81A59740E850E60B380AAADDFA0A1AAE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour18</td><td>0,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_81B506CF75F4131FB52373C34CA08C61</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_81CC279EAB7CBF5CA1CC7AADC3A9B5D9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Compression</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8221F76C7522D24D37ABB3720B6EDF8A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyMethod</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8246BAA66319F24BC92B343E1C505CC6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TCPNoDelay</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8262F4F8CCE4E96B7F3F5E86E9E2FC36</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>HideMousePtr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8278B18FE709A2A116D07C22BAF3256F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UploadTime</td><td>#151</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_82837EA07969F8F0A46A416E0AF2516E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateDisplays</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8283B19D7288C536A3B9B37248E7A1AE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AuthTIS</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8288D06769730B170909E4518C49DD2F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>Show</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_82C477465CC0FD895F4E9CA32A0B7792</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SynchronizeParams</td><td>#66</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_83F678583B6540CEAD8CE7746F48D357</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour4</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_83F882BB2468F3110F596F98838090FC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>DisableArabicShaping</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_843F0B5BD7FF50125995EC701529518F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LogHost</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_84537411062424E9C351BF2445ADBDE0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness224</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_84698CAEA4C4443FAAF94CD50B2A0C7F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>KEX</td><td>ecdh,dh-gex-sha1,dh-group14-sha1,rsa,WARN,dh-group1-sha1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8486EDA52E4114789593CC18EF27E435</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PublicKeyFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8506254A5C8D12D2A72359510B6CF5EA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>ToolbarsLayout</td><td>Queue_Visible=1,Queue_LastDock=QueueDock,Queue_DockRow=0,Queue_DockPos=-1,Queue_FloatLeft=0,Queue_FloatTop=0,Queue_FloatRightX=0,Menu_Visible=1,Menu_DockedTo=TopDock,Menu_LastDock=TopDock,Menu_DockRow=0,Menu_DockPos=0,Menu_FloatLeft=0,Menu_FloatTop=0,Menu_FloatRightX=0,Preferences_Visible=1,Preferences_DockedTo=TopDock,Preferences_LastDock=TopDock,Preferences_DockRow=1,Preferences_DockPos=230,Preferences_FloatLeft=0,Preferences_FloatTop=0,Preferences_FloatRightX=0,Session_Visible=0,Session_DockedTo=TopDock,Session_LastDock=TopDock,Session_DockRow=1,Session_DockPos=602,Session_FloatLeft=380,Session_FloatTop=197,Session_FloatRightX=0,Sort_Visible=0,Sort_DockedTo=TopDock,Sort_LastDock=TopDock,Sort_DockRow=2,Sort_DockPos=0,Sort_FloatLeft=0,Sort_FloatTop=0,Sort_FloatRightX=0,Commands_Visible=1,Commands_DockedTo=TopDock,Commands_LastDock=TopDock,Commands_DockRow=1,Commands_DockPos=0,Commands_FloatLeft=0,Commands_FloatTop=0,Commands_FloatRightX=0,Updates_Visible=1,Updates_DockedTo=TopDock,Updates_LastDock=TopDock,Updates_DockRow=1,Updates_DockPos=619,Updates_FloatLeft=0,Updates_FloatTop=0,Updates_FloatRightX=0,Transfer_Visible=1,Transfer_DockedTo=TopDock,Transfer_LastDock=TopDock,Transfer_DockRow=1,Transfer_DockPos=364,Transfer_FloatLeft=0,Transfer_FloatTop=0,Transfer_FloatRightX=0,CustomCommands_Visible=0,CustomCommands_DockedTo=TopDock,CustomCommands_LastDock=TopDock,CustomCommands_DockRow=3,CustomCommands_DockPos=0,CustomCommands_FloatLeft=0,CustomCommands_FloatTop=0,CustomCommands_FloatRightX=0,RemoteHistory_Visible=1,RemoteHistory_DockedTo=RemoteTopDock,RemoteHistory_LastDock=RemoteTopDock,RemoteHistory_DockRow=0,RemoteHistory_DockPos=172,RemoteHistory_FloatLeft=0,RemoteHistory_FloatTop=0,RemoteHistory_FloatRightX=0,RemoteNavigation_Visible=1,RemoteNavigation_DockedTo=RemoteTopDock,RemoteNavigation_LastDock=RemoteTopDock,RemoteNavigation_DockRow=0,RemoteNavigation_DockPos=252,RemoteNavigation_FloatLeft=0,RemoteNavigation_FloatTop=0,RemoteNavigation_FloatRightX=0,RemotePath_Visible=1,RemotePath_DockedTo=RemoteTopDock,RemotePath_LastDock=RemoteTopDock,RemotePath_DockRow=0,RemotePath_DockPos=0,RemotePath_FloatLeft=0,RemotePath_FloatTop=0,RemotePath_FloatRightX=0,RemoteFile_Visible=1,RemoteFile_DockedTo=RemoteTopDock,RemoteFile_LastDock=RemoteTopDock,RemoteFile_DockRow=1,RemoteFile_DockPos=0,RemoteFile_FloatLeft=0,RemoteFile_FloatTop=0,RemoteFile_FloatRightX=0,RemoteSelection_Visible=1,RemoteSelection_DockedTo=RemoteTopDock,RemoteSelection_LastDock=RemoteTopDock,RemoteSelection_DockRow=1,RemoteSelection_DockPos=369,RemoteSelection_FloatLeft=0,RemoteSelection_FloatTop=0,RemoteSelection_FloatRightX=0,LocalHistory_Visible=1,LocalHistory_DockedTo=LocalTopDock,LocalHistory_LastDock=LocalTopDock,LocalHistory_DockRow=0,LocalHistory_DockPos=207,LocalHistory_FloatLeft=0,LocalHistory_FloatTop=0,LocalHistory_FloatRightX=0,LocalNavigation_Visible=1,LocalNavigation_DockedTo=LocalTopDock,LocalNavigation_LastDock=LocalTopDock,LocalNavigation_DockRow=0,LocalNavigation_DockPos=287,LocalNavigation_FloatLeft=0,LocalNavigation_FloatTop=0,LocalNavigation_FloatRightX=0,LocalPath_Visible=1,LocalPath_DockedTo=LocalTopDock,LocalPath_LastDock=LocalTopDock,LocalPath_DockRow=0,LocalPath_DockPos=0,LocalPath_FloatLeft=0,LocalPath_FloatTop=0,LocalPath_FloatRightX=0,LocalFile_Visible=1,LocalFile_DockedTo=LocalTopDock,LocalFile_LastDock=LocalTopDock,LocalFile_DockRow=1,LocalFile_DockPos=0,LocalFile_FloatLeft=0,LocalFile_FloatTop=0,LocalFile_FloatRightX=0,LocalSelection_Visible=1,LocalSelection_DockedTo=LocalTopDock,LocalSelection_LastDock=LocalTopDock,LocalSelection_DockRow=1,LocalSelection_DockPos=353,LocalSelection_FloatLeft=0,LocalSelection_FloatTop=0,LocalSelection_FloatRightX=0,Toolbar2_Visible=0,Toolbar2_DockedTo=BottomDock,Toolbar2_LastDock=BottomDock,Toolbar2_DockRow=1,Toolbar2_DockPos=0,Toolbar2_FloatLeft=0,Toolbar2_FloatTop=0,Toolbar2_FloatRightX=1,CommandLine_Visible=0,CommandLine_DockedTo=BottomDock,CommandLine_LastDock=BottomDock,CommandLine_DockRow=0,CommandLine_DockPos=0,CommandLine_FloatLeft=0,CommandLine_FloatTop=0,CommandLine_FloatRightX=0,PixelsPerInch=96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_851F938C68230990C0388963F1ACD793</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RXVTHomeEnd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8535AD6419D33F61F3F2754458842D37</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour6</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_85391F2682A11F8128476717955B0C75</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour5</td><td>0,255,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_85F8AF8219E8067A3B61BBFB19CAAFCF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Security</td><td>MasterPasswordVerifier</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_85FD1E37A15CE35D1EBC4D685F77EAD5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RekeyBytes</td><td>1G</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8607DC1C035EA4E3F2A7B2D9E98B0715</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour4</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8609894F43DD37755319228EE407CD45</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Putty</td><td>putty</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_863DF525501DBE215CB30DF092221433</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoRemoteClearScroll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_86484834A3F8DF613744DF2A103ED6F4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness64</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_865D79F9AFB1FF00704C436C6D024CC2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>TipsHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_86737C4E289905F424BDB565963BDC4E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_869851A2AA6C4CBCCC42BEB65809995B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>11</td><td>8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8783538C761A7D82EA19E35A6F739D58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>ProtocolSuggestions</td><td>#7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_879BF95EF14FD5AF49F515D5AAA23DEA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RectSelect</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_883221F9B48C2A20EF361D29267641AD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PublicKeyFile</td><td>C:\gui\id_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8838F928016D6FE74308233DA68E4AA8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TerminalSpeed</td><td>38400,38400</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_88B0CD3A211D133D391C1D46B6DC9707</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WindowBorder</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_88D654D09A67638B9D8BDFB6541D3F17</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AgentFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8927F0CB3B5769E8B8EEA966D083850F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountFTPS</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8929AC3D95258F10A32F57421F069D15</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BoldAsColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_893F9A18F2475518E0189EED2C6AC27D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>HasRule</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_898C53A6B2396B753981DEE81BF6C05A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>8</td><td>/etc/rc2.d/S98radex.sh</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89994827EA06AE1A766482D03B506663</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DefaultDirIsHome</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89A322393B34730798D6B13E93729663</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.9</td><td>HostName</td><td>192.168.200.9</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89A9223D7F4D8EE1122B32A1B30484F4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SshBanner</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89E675D12C19A3D0A4543551C915F3E7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TemporaryDirectoryAppendPath</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89F65A5601C64DD84C20D23D5E992BEB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_89F6E9EEB59E62A0C24E535813541820</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CapsLockCyr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8A0B234FC9D5D8BE74C1C3D3ACA55BC5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour15</td><td>85,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8A0D5BCD1AA4D756B03FA76C89FD2E06</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour2</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8A0FED08FE57F3A9B5EA0536F87508E2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>QueueAutoPopup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8A39FB0559FFCF18FDBDFDA83B5CB8A0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxUploadTime</td><td>#6</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8A57A24FA4408D5C7E2CCDEFC7F5C140</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FullScreenOnAltEnter</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8AB8966A9063DE4D6D6A5F172ED7D34A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoApplicationKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8BA6A607C869437B9C738C3B37C0E61F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>RemoteFileSaves</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8BADD7B4EF61BB85699A9D3993F20CE4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8BB9E0B8E06151EA427F311F21742E71</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>Layout</td><td>70,250,250,80,80,80,100,;96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8BBB8A1AC39DA4AC366C94BE8E01DA73</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>DECOriginMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8BC40F711BDB941DEC6BE2A1A9875442</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TerminalModes</td><td>CS7=A,CS8=A,DISCARD=A,DSUSP=A,ECHO=A,ECHOCTL=A,ECHOE=A,ECHOK=A,ECHOKE=A,ECHONL=A,EOF=A,EOL=A,EOL2=A,ERASE=A,FLUSH=A,ICANON=A,ICRNL=A,IEXTEN=A,IGNCR=A,IGNPAR=A,IMAXBEL=A,INLCR=A,INPCK=A,INTR=A,ISIG=A,ISTRIP=A,IUCLC=A,IUTF8=A,IXANY=A,IXOFF=A,IXON=A,KILL=A,LNEXT=A,NOFLSH=A,OCRNL=A,OLCUC=A,ONLCR=A,ONLRET=A,ONOCR=A,OPOST=A,PARENB=A,PARMRK=A,PARODD=A,PENDIN=A,QUIT=A,REPRINT=A,START=A,STATUS=A,STOP=A,SUSP=A,SWTCH=A,TOSTOP=A,WERASE=A,XCASE=A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8C115F7C3B8B4152CAB3802B7156045A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsSSHOpenSSH</td><td>#7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8C4A606D79F23D1BA55BE15FC71217BF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>Upgrades</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8D1FFD8E05EF0DF2F00E868A4A409FB5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>QueueNoConfirmation</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8D7B575A65DC2508917F0944DCD100A1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyExcludeList</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8D99240F5884AC333D525AF845EB1479</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8E09D1D68949DD066AD64BED34191B69</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BeepInd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8E09D45D9E067FA1D47E945FE90A7D64</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>Text</td><td>rw-r--r--</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8E252DF1BCAC8A936875A8B285A47CDC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TryPalette</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8E2D2806005E8D96702854B2E301910B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PingInterval</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8E9D77D35002BCC87DE49702CCD4CDB1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoWorkspace</td><td>Mein%20Arbeitsbereich</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8EB19CA078E1D96716AE8B527A37AF9C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>4</td><td>/home/radex/ripped-data/altona/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8F0C69099DE1F23493AEC6885F313A9D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Cipher</td><td>aes,chacha20,blowfish,3des,WARN,arcfour,des</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8F3B7D4F30D870BC91F35F5437CC4465</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyPort</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8F6A00E03F4F754EF5BA86FC1B94AFCA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8F9B4C2D246DFD8A6AFD063AE8D51AD9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PassiveTelnet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8FBCB4286ECC73624293D5E2E04775E6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NoApplicationCursors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_8FF3FAEB059AFA077F2ED977826F6C7E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>PixelsPerInchMonitorsDiffer</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90203F1E16B4CE43BD51E84B4B22798A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PingIntervalSecs</td><td>#30</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90245D99202C5B7587DEE368E3AF52ED</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>UseSystemColours</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9083C0C329D74A70243DA0150747A33F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness64</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90938565EE5D3E70F8026F3EDAD32F2D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ShowFtpWelcomeMessage</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90C2482F92E87EBEC39957BE47F321B0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TermHeight</td><td>#24</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90CECABB7170957DC07EE751F337D7F3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DirViewParams</td><td>2;0|307,1;119,1;150,1;79,1;62,1;55,0;20,0;150,0;125,0;@96|6;7;8;5;0;1;2;3;4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90D731A109E247F5817C8D48471C39B8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateDownloadOpensStartup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_90F8DDF991F08CF0AFBA3FADB939E205</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialSpeed</td><td>#9600</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_913D626030E63D63F5D1772E416FCEFB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>X11AuthFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9176473B0ADBC70CD4C1D504DE6E179B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialStopHalfbits</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_917674F356AC07E2D77851897E95AE1D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugOldGex2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_91824F9B242B1F83569EFC484DF6D30F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsUpgradeTypical</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9198E9E75A10DF8C80B23E3F69494BC1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DriveViewHeightPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_919954115A14FA0948A66D6F1D2BA01A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>Text</td><td>rw-r--r--</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_91A4A7EB1D651474206A9A060841CFB0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>4</td><td>1024</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_92583E91BBF654EFCA7708B87364E0B9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PreservePanelState</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_92BBE5A71AF48F4800A4DAB388CF1AB8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SSHLogOmitPasswords</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_92E4E390C5495470E85F878ED4F459A4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyPassword</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_931D944E474065A4776ABEB70B23B211</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PuttyPassword</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_936AB6A0F56B4D6DB1E9639FADDBE370</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CRImpliesLF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_937142E145BD7E272D2A78DB68415A44</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.1</td><td>0x4f2b4148adf2d1a28e3b655b38c7373f861dbd6dab2d4b3e25cb700c6456f745,0x1f79158149a1b4599b7320216abfa8b139feb1933c37e0278bfac7dea3a715c7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_93C141DE666350728F9F3095EDEFFBA9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_940A160E837D5E66B906717E1557EEB5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>TreeOnLeft</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_948A79C84B907FAF5A65CFA7E6C31637</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>Name</td><td>CTRL</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_94928E7434FC3D3B820BD0020286DB9E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness128</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9515B8BE18762BB4D22D477475ECE3AB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollbarOnLeft</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_956100DB7FF51BA44B4B5C2BDF84B3E4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SSHLogOmitData</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_957B5CACFEB5FBBF6AC0D5F532B8243F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialLine</td><td>COM1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_958F66A08AF9A8F7B98970546B0D39C4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_95964B200FD3D69F0839DC9825E498D3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>QueueKeepDoneItemsFor</td><td>#15</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_95B233E6F2F4F66E9BC48AE31398A6C0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TryPalette</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_95D972F6BC2BFFC05B81EFCEA79687BC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>DECOriginMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_95EFCB16B5DC3C1C8890E6A7E972185D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxOpenedSessions</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_960144C2F3595565647CFDC522D969ED</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LockSize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9614251AF72C2332E948AEA8D37DC862</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionReopenBackground</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_961CF9BB932327887A54372643B4B748</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ShadowBoldOffset</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9620418B847D18497CBBF76EEFC3B74C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour11</td><td>85,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_96A723D6BE93C81E676BCE5377D01ED8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TelnetRet</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_96BCC9F83EC4F4DCDDB13CE1E94A11A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CopyParamAutoSelectNotice</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_96EC44186C4D3BD21227679D1AEE088C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>InstallationsUser</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_970F1CDF33C7243A54FB7860BB948AFF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.9</td><td>0x22d26096fc9d947258b94c68c746d25bdd82c91a393e39c5ddeeb49bb650297d,0x26c59a16cf8d67065af619b4d2ce30a2139e512797f74805196c6c13bb57ad63</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_973BF0560C2166831D9E7638EB33BB3B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxDownloadSize</td><td>#181119</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_97468F63B2E01D6039C7275A8698E9F3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FontVTMode</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_975058EA3FBCF267FA5D65EE97F641E0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>DetectMDIExternalEditor</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_97A4F4B1339D8D9F866AA9FBF11B6612</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Unofficial</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_986E243FC0DDFC981CD0A3C055E28B6F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LockSize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_987FCB2AFCB6D75C1380A87CEFB3CEE8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SunkenEdge</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_98A41F53C3D984BC492D991085480A4C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>0</td><td>/home/radex/ripped-data/Gilbert/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_98ECAA764073351F78AF99E31B24CBAD</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.11</td><td>0x4b81c5769f1d16906fb3745dd6febe80d62f8df2e35356a2f649abbaca869f80,0xfd055d07242fea2a5d63cf531f4c0087424ebb9a91780af11630eebf779ae6f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_992FB82F3AD0BB96FA1A60DEE943E6C8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NetHackKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_993A31E94488BA024EA6903036B71041</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>SftpFailureErrors</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9956978BBBB2519179602573B22E925A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>WarnOnEncodingFallback</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_997A804DF5DA549932B9BF3575A61B56</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>PreserveTimeDirs</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_997CA08C049B65892AE08F1E1DB58284</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>X11Display</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_997D7C599CEA856BE25ABDFE0F511873</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsFirstTypical</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_99872AEFFC8C6D304B1E78152C9FE3D1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialSpeed</td><td>#9600</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_99EFE67CE8CA9B28EE801D48323F2241</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugWinadj</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9A51F9DFBF62AFCEC70FBE64BF174503</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogicalHostName</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9A5C254C228BE45F05650771620153C0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>UTF8Override</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9A7C7B064E6D8E39DD3481B8863D37BE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>PreserveRights</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9A9BD9409C0875E74B39E1E94AD53978</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>EditorAutoConfig</td><td>P</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9AA136AFD0B6CCA636CBF3352BC6D6CF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>TipsUrl</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9AA53F1DA5601BF6E09B517D85EA550A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteFilesSaved</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9ACA8ACAC6DE6E381F62E33A931C1B88</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyUsername</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9ACB11C1AB509E618454FAFFB63CCC44</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>NotificationsStickTime</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9B0621996393399D7F9FF5B5333BE1C3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>X11Forward</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9B63AA73852BD7ECFBA6F83FEF927E8B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ExternalIpAddress</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9BCD14FE2DE750123DD226CA57083DAC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9BD6E89D4768EB2B92F02849D7453CBA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UploadsCommand</td><td>#13</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9C4A9DA2983C18B9A157DFD11E564894</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LocalUserName</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9C69484FA31AA7EB594C18FCDA9345F7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LFImpliesCR</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9C97D55D642E4133245A3626E1D4B9BF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>TreeVisible</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9C9D6DB24166F127F01163C629DAD482</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DriveViewWidth</td><td>#100</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9CC66B09D343C932C66C75EFCDDD1EDC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugMaxPkt2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9CD69A54B013B445FFAB19460D6BE5DD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>Runs</td><td>#9</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9CE44FE40928E87DB39779FA39946766</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AltOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9D21DDA59D28AD7BD07B48094C0F5867</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionReopenTimeout</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9D31E00EC45CEC3AAD5443985C8182E9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ConnectionSharing</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9D5D39F034E84529C9D2F109F37111B0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>Queue</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9DABAA1344A5236C9E8E3AC1FB4EC6C0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.9</td><td>FSProtocol</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9DD0D0C7EC85027F28FB2452F55A25EC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>NewsWidth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9DE5E999C79E91D65AF73DCCF546705C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugRekey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9DF28A9CB526C096D384CBD8D7148DD3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ConnectionSharing</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9E4BC8B9998773631A58946C0C237312</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionReopenAutoStall</td><td>#60000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9E6978BB8A31B26121CC5D2DD4A6FA8C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Environment</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9EC14D775D7A863D8ADD3FE11393B868</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountColor</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9EC63180CE31D50DF86BE6DE40325BC5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MouseSelections</td><td>#591</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9EC72EF9EABF983AA9A5F26913636B1E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsSSHOpenSSH</td><td>#46</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9F3E86FEBCD2579E590AF8506830746A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.4</td><td>PublicKeyFile</td><td>C:%5gui%5Cid_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9F6053F54A4270D32134B528689DAAD8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness192</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_9F761974267858320963801317844E97</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour14</td><td>0,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A12D174F0D3C9F593BB54562E58EECED</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>NewerOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A154931D098BC18142F86B8372AC4B55</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AuthTIS</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A1904EAA91E10A0345083EDF38728D27</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogMaxSize</td><td>#x0000000000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A1DFE12D62C9CBD938FFCB2568CC5D1F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour19</td><td>85,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A1F20BD8924114FEF95C6EFEABF442C1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ScrollBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A1F98EF9CCCA8DABB3A40BDE0462AF23</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollOnDisp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A23D120622FE3F60221A617AA8751175</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TryPalette</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A287D7CB4A0701BFEAF5D16676099066</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialFlowControl</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A2A22CEAC3BA76863B4CF94DA8295AF6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LogFileName</td><td>putty.log</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A3036F3ACEB0A25084688010810AFEC8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>FileMask</td><td>%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A34FF8CA8B4F2D2F95955774777E31EF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ANSIColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A397A4677B598EAD14DF64D799C6F314</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.4</td><td>0x22d26096fc9d947258b94c68c746d25bdd82c91a393e39c5ddeeb49bb650297d,0x26c59a16cf8d67065af619b4d2ce30a2139e512797f74805196c6c13bb57ad63</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A415CC6F2019C84758DF7FD0F8BE6066</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WindowBorder</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A4210254DFFD18C4DF332B037DE7E5A0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>GSSCustom</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A44CFCCFE942D3D8613976D0F0592C8E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>IgnoreCancelBeforeFinish</td><td>#xdfbc9a785634023f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A485B38E56CBF9BB3776746CC0142CC3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A49ABCCAA363197AE80BB92D78A8D99A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour4</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A4F711E9F1477846E0113751F5B2C4A3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A513A766A9451D0B447082DE7FA93A0D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AgentFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A515744F88B27A2804960412D98EA6A1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>WorkAreaWidth</td><td>1920</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A545E54AFAB26F2B6579BEAD750DFDE0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>WindowsProductType</td><td>48</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A55ADBFEAF1AAF77F50CDFFDB89BF502</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyPort</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A58624D62B5AF9A507F60C09BABBEDBE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>RenameWholeName</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A59CDE26ED08B09394265A3D4A283740</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyLocalhost</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A5FBEA13002F6F53822B14CD00C894E4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>5</td><td>/opt/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A63832187D3527A493A7985EE50F9C31</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>Name</td><td>%EF%BB%BFNur%20n&amp;eue%20und%20ge%C3%A4nderte%20Dateien</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A640F5B981A892AC8103A97639FA6836</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>PreserveRights</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A64AC0DB3DA5E53D8D656693A53A3A33</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness224</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A654B4B889CAA8435762250D84BE3AF1</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.1</td><td>0x4f2b4148adf2d1a28e3b655b38c7373f861dbd6dab2d4b3e25cb700c6456f745,0x1f79158149a1b4599b7320216abfa8b139feb1933c37e0278bfac7dea3a715c7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A6785B4E1255DC75E8335A14148BED8C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>X11AuthFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A68EFAB931FAFA04865FBBB2C11EBDD4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>TransferMode</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A6B02254002C496950AD866E03BE1E57</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Printer</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A6D9D49F74DF840C005DEE26D3E49587</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmRecycling</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A6F15039682773E6B46836F8F9A49698</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour6</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A768577E2E14B15D81E52E9D88B600E3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\Mask</td><td>3</td><td>%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A79881477CAF0338372722E14D6F5EC7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LogType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A7BD73E5657047D4CDD9864A5B964A55</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A80A34E332B373E053190C99E30E716D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateDownloadOpens</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A80D89650F7CD9BABD3F0F7E8F8DC7E7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness224</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A824FDF5F6F0C86612E4FA6FB9FE312D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyExcludeList</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A89CDE7FFAA10B517BEDF7B592AAFD34</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>WordWrap</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A8F0F9043686FA97C94F288CFF3970AE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A8F6B32C70DA52125574DBEB06F22DC4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RFCEnviron</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A8F8BD4AB5047083E2192BBDA081DBB9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>MouseIsXterm</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A9080CB09A1FA44FB0893665043AAFF5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BellWaveFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A914F3DE4D41F49AA7EFCA65708503C7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ForVersion</td><td>#511027781</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A92B497888694F6A7BEBAB2DE65A820A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Compression</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A932A7EAEC4C2DE58CAC0F40A1243933</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FontQuality</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_A952B91B8F57DE42E9BC2E20279BDFA9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>NewerOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA08A5F7954329E53CB686A966D8E844</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AddressFamily</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA3D00BF7FC2659A6CEE8806FADC16E9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LinuxFunctionKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA4F7D41D0CB425751E7DA8EDC2F4529</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDAllowMoveInit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA588C9C54B7AACDF375E79E4A8EF6EA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>QueueParallel</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA7572F3C74D52600BADB6DA7B966B88</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyPassword</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AA90A1F5A7AC1C67FDBC8E1833516630</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour16</td><td>187,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AAC2501073560E34587F2BF838DC3A6D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingDefaultStoredSession</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB07B70405B3EA4868939F4EDC366056</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour21</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB1952FFDBEF8A85D33C38BD8CE7EA47</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SSH2DES</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB4C2CD9B5A1139B8E2C70D10324D650</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>FontVTMode</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB6A95E173ADE0AD3CFE51B11BE8E294</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>TipsIntervalRuns</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB84D95DB60725BC3DF510CFE174D11F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\LocalTarget</td><td>2</td><td>C:%5CDevelop%5CRX-Project%5CProject%5CUbuntu%20Develop%5Cusr%5Clib%5C%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AB8B6FE967EEB4296B527C7987425A7A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ABA095B071C10AE58277DB3FDE65B9DE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\IncludeMask</td><td>0</td><td>%2A_ctrl</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ABF7E581DF8F51E9E937EBEB8F1EB9D8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AC4D81C0471D0CD77AC948FDFB1D5EDD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AC646531B2A7051AC8B93A77A7628A9D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogHost</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AC6F067BF15C89483B1FC419EC33D435</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteCopyDirect</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ACC2360C80EDA7DC84F1D495F2CC39CD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsPrivateKey2</td><td>#17</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ACDD7F62C33D250BEC8C3F75DE6ECC8B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour19</td><td>85,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ACE795B265EB41607730B5DC547C91D3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness160</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ACE7BA356EF6FE69040FB8DBA79A3002</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BellOverloadN</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AD1FC88AD4674691F018E095558F04E6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenedSessionsAdvanced</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AD555FC85234132F049A0B6D6B934037</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ADBFF0AB64B3D6FEAF0DCAE327C8360A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>6</td><td>256</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ADC8AF57FE10F30EDF04684E38907F14</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>TransferMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ADDC419C22CFACCD1EDD45D8EFB4755F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BellWaveFile</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AE395FFFB480FD60273FED09D3D06EF7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogFlush</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AE4B8C5E82D5B0F5E557FB6AD4FD1CE1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ChangeUsername</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AE62133BB13CD8582B440DD11236B44B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingMasterPassword</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AEAE0A7C677E0EC9B37D5F5AE4591E8A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>Downloads</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AF021CF93722E9ABB33DF6E6EB4338B3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>SysColorBtnFace</td><td>F0F0F0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AF2FCA8B6F68A6BB5A2F3C09E7AF4BC9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>Text</td><td>rw-r--r--</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AF31D82638A923467910C6ADC5F479EE</td><td>1</td><td>Software\SimonTatham\PuTTY\SshHostKeys</td><td>ssh-ed25519@22:localhost</td><td>0x52ab6d39f5671a435b58fe855afd5c0d052970c2020d5a3e48e7c03bc2c93682,0x65d3592100a648ea7b9e269b6f3ad5633d9c781670ecea400429bd66150e3749</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AF44C9421B3576A9272338786DE9D04D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AuthGSSAPI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AF664BB65964E4E12D5A25EF782C1BF7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SSHLogOmitData</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AFA6DC489626F1A60E7427E0355453D9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>RandomSeedFile</td><td>%25APPDATA%25%5Cwinscp.rnd</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AFA82305FC92BCBB13E97CD05A5F75A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>IncludeFileMask</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AFC0EFF5D045EE836044C217C0AEF7A0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour15</td><td>85,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_AFDCC63B76F1CB9A2254137B7BA24B5B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RekeyTime</td><td>#60</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B01519040F478F38C1551CF95F42E8B0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PingInterval</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B015375509F93EEFA467FBC0850E1DDD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>5</td><td>512</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B0321DFCBBFB8BBD22629A46D68963BE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AddressFamily</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B06834DFFA937F194DC5D0EE99F7BDCF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyTelnetCommand</td><td>connect %host %port\n</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B087627AF4DF753D92EE7ED9FC36F697</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BCE</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B0D2EFEB91451F63453FC3E036D3D764</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B0E08BA3DBBE29AE1D3D7DCAAD36C078</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PanelFontCharset</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B151F0726851D56C386F0ACD5CD00C1B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LocalEdit</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B195CE025AF7B9AA161B5967F2D822A8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>Name</td><td>&amp;Text</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B1ABAD480C27CA9B274394C9D98D7527</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoReadDirectoryAfterOp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B1AE73EBFE83DD3FBA455B028DCD8E6B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ComposeKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B22B94B7E1734886F634F912639C7022</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.4</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B24BA0A6EDEDD23BB7E802A41B29B8ED</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\LocalTarget</td><td>3</td><td>D:%5C%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B27BCE9FBB731F86D2B549F050B648E3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>CurrentPanel</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B2A4E1C9E4B92ED20A3701B5AFC4061E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B30B5132D8604DE9A315808D4CCF77F1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideBoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B379AC4434D4FC49B3EB9BD7B54D96BE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AltOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B39993E387802B05D312BE6A1F4417DE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DriveViewWidthPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3A64BB780D11E4F875E60A5664E811A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogFileName</td><td>putty.log</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3AAE0BB883ABDC952099550D7C5BD4E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyTelnetCommand</td><td>connect %host %port\n</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3ACB301ABE0C5855FEBECE1EC480957</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\ExternalEditor</td><td>0</td><td>"C:%5CProgram%20Files%5CMicrosoft%20Office%5Croot%5COffice16%5CEXCEL.EXE"%20!.!</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3B2029930A2B3E0D99915A4C686664C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BlinkText</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3CCC19786B4B28032877D123AC3F9AB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoRemoteCharset</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B3D7EAAB574A480C2D26C384FBC6892E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LFImpliesCR</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B46D20C0A4C06A2DFB389E402201DA23</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyLogToTerm</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B4A45C9D60D94364F3250B596BEEF6A8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugIgnore2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B4F989EDCBFE268873116E1C8EEE2016</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>GSSAPIFwdTGT</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B574634588A6D0881F94EF17DD0371DC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour9</td><td>255,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B5878618A16C6D4CCC240C04CE137BDE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SSHManualHostKeys</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B612136FC15CAD58E2CFA82A8CB23BD3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\0</td><td>FileMask</td><td>%2A.csv</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B62DCFA7AA793749C21D323994F4A209</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness192</td><td>2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,2,2,2,2,2,2,2,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B65662B6B5FBBAEB02C1C5BC4AC53591</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoOpenInPutty</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B6750CF38295760A469C6248EAEF1EA0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExtensionsInstalledCount</td><td>6</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B6C3C09E994CB0864575967AB5D02D4C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>GenerateUrlAssemblyLanguage</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B6F5B8CE9BAB71FE6EEFECCCA6CA6B6B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>PasteRTF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B6FCB61E90623B4EC5877D72B6BECB9F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>PuttyRegistryStorageKey</td><td>Software%5CSimonTatham%5CPuTTY</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B7045B29DCCF3C768309B0530DCB4AB6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialFlowControl</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B74FD347DCBAAC642F008364AED8E40B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>NetHackKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B756B57D346EB3483C68503E17F8073D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ANSIColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B7A76A02A065EDFA9D95AF395594BD90</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>StatusBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B7D022FBD986253ADF0DAC341FF8F1DE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CollectUsage</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B7D30239049AE4942D870EFD04FA5D9B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B809E923BDA47C4CCD472744DF50B44A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>X11Forward</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B80A9F8A8A24A01315D39DEF9F084BB7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>BidiModeOverride</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B822F53F718CE452293F1C8815A55C72</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ApplicationCursorKeys</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B8272D4A02BD2A9FAAAFDD460714157E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>OfferedEditorAutoConfig</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B883687213E8EEC47CA8F7776AC5EB9C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TemporaryDirectoryCleanup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B8869BD2032364CA0CDE4079DFC89706</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateDisplaysNew</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B888F09C8D05E6EFCD59C23C2BD22FEE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>RekeyTime</td><td>#60</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B89118F2E604ACC6CC2224BC2D5654ED</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B897439E07CEA5410EBA3D693905BC40</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>Masks</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B8C406E312AA4DD380C26BEFFD05A146</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Printer</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B95C0AC326B8A2E8AB13F80423965DEF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\Mask</td><td>2</td><td>%2A_ctrl</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B9AED690D7D26038EBD4DA33965ECC54</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>BeepOnFinishAfter</td><td>#x176cc1166cc1363f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_B9C8790FAE232C5F2084C3CA6B8D9251</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>CalculateSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BA1B2528341438263CCE60ED6A4B68C1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ExternalSessionInExistingInstance</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BA248AD8CD171241E97176E03C31FD65</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>HasRule</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BA7ABFBAAD3897D3C3C0662332A45514</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>X11AuthType</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BAACE6A0A3EEA1A3543ED63B06742C52</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Security</td><td>UseMasterPassword</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BB0AFFB1708721777FB1783860D8C424</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RXVTHomeEnd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BB39ACE9E3512AD0B525FE119B22CB97</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>KeepOpenWhenNoSession</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BBE898A109286A1AA17CC4406D88121A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SessionReopenAuto</td><td>#5000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BBF1DA2873821D814801DAC5040F32E8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Bookmarks</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BCBE632D3A19A3A02452FE240A99BA30</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>GenerateUrlScriptFormat</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BEB7E066A34C3700AD2F97FF06EBE3E4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BEE90B4F03341B418CC6CAA801E95E3F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>9</td><td>/etc/init.d/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BEEDD50557ED264455198FDE563DBCE0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\RemotePanel</td><td>DriveViewWidth</td><td>#100</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BEFF7CABCC5C9F50FA85399A441E938B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.5</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BF150F01177292D7956AD8AA5C22E9B8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ShowTips</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BF2802E11C9845C269490E1122F0EA42</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BF59D0CE9ABE7BAC753F41B04B3E5BE7</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoRemoteWinTitle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BF714C29EAB28F7B08C36C87BBFDE2C2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BellOverloadS</td><td>#5000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_BF8919931D2DE997BD28153ADE23A58D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>TipsCount</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C013432D669072E5BB7ABE5DC61B20C2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.112</td><td>0x52ab6d39f5671a435b58fe855afd5c0d052970c2020d5a3e48e7c03bc2c93682,0x65d3592100a648ea7b9e269b6f3ad5633d9c781670ecea400429bd66150e3749</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C049FE0E868A5B974EA5543F63B317B5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C0941B0B916F4B791F3D52379FC859AD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ShowInaccesibleDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C0B31DBE23D598D8BDB23CE941F936FA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxUploadTime</td><td>#47</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C0E57705086DAF3F93E667C51D2D37AB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>KEX</td><td>ecdh,dh-gex-sha1,dh-group14-sha1,rsa,WARN,dh-group1-sha1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C18CF875AC7E2D442591EF595B195B6B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>RemoteFilesSaved</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C1A508A12E44B2CD61AF47F34FCC46E9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LocaleSafe</td><td>#1033</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C22178FCE4609F5FA3FD3749A9ED2E65</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour3</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C2275683AFE7A8B2BA9BA8E116CACD6D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\Default%20Settings</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C25CCD5CFE406627E9048BE806EEACB4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountTunnel</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C2BEF21C951F22AA6A83C8A7A3C3F2A5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C305B16495BB18F2F793776AB854C271</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C346759BCAE7650916F1815DDB187F49</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>EarlyClose</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C35C28275AD7F78495BD90BC37779CCE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness64</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C3EAA4CCD88FE9EE60F3CBF7FE4C88C3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ColorDepth</td><td>32</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C4138BB2FF0FD7560B09CDC00EB6A7EB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDWarnLackOfTempSpace</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C4BE07A72753F1B935A59CB9DA39C7DC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>OpenInPutty</td><td>#12</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C4CCE2E9F93C3D8D2303150BECB91EBB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>DownloadSize</td><td>#x0000000000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C5A6338738EBBE3260EEE2B3E7173508</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugPKSessID2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C5AAC1BD5E2E1FC457CA015120146259</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>WindowParams</td><td>-8;-8;1928;1048;2;96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C5BCBF9E343E12DA4CF5294D020225E7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C6043FFB65247B467821EEBCBE15D207</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>TunnelLocalPortNumberHigh</td><td>#50099</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C649D2EC527A5A607186CFF0B2595FE4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Message</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C65A79224DDE6E12167A41920860B010</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ConnectionSharingDownstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C67B6886080F86048C066BC023158664</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LocalIconsByExt</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C6802E686C1E3D75B0B550902D57B28B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>LastHideShow</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C6C938B08A5397EA1AB459E88686C6A6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TelnetKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C756D1EEC9135CC371BC38D7DECA4843</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C7737B1C921DE30A5F5758A3D225A138</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>ShownResults</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C787E59AA83C1E6ABFDEF2DC165D0CDB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugRSA1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C79E1F06C7C8DB9D7CD877D9F5597611</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C7B17D06693F882C92FDDF13529060AB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FindMatchCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C80D9C80B0EBF8445B666B0A18CC791C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>0</td><td>Unlimited</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C81547F42F4AEB2670EE0A2CB9EE3688</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C81D0BE87E3E325D1CF38D8B32FC4326</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>localhost:22:ssh</td><td>ssh-ed25519%20256%205f:f4:cd:25:bc:95:8c:84:e5:81:6a:06:04:54:0e:48</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C8287D9E42E221ADB4189DBDF87A1A49</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>StampUtmp</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C8292B9C74C3D2FC480A1434B279068D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C8340AB3036B46B55D46D8247A14D13E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.5</td><td>HostName</td><td>192.168.200.5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C854C9198DB03D851CF69335F7FED0EB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ShadowBoldOffset</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C8A3ACA06B3D2B842F1E93D851C37C6D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RekeyBytes</td><td>1G</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C8B23B3623A71F8F0BFC4708BB76776C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.11:22:ssh</td><td>ssh-ed25519%20256%20f8:11:05:da:08:8d:a1:31:9f:e5:82:4f:aa:26:5c:2c</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C91E59C2726439E408C0F88560AF9589</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDTemporaryDirectory</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C94839F10867B276E3D6A55E63BD1247</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:192.168.200.4</td><td>0x22d26096fc9d947258b94c68c746d25bdd82c91a393e39c5ddeeb49bb650297d,0x26c59a16cf8d67065af619b4d2ce30a2139e512797f74805196c6c13bb57ad63</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C9A7232DB2CB0618F98B388C3DAF0780</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C9BDE861A487ABD1E0E0B4A5A013124F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ShadowBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_C9C8CE8E47DF7BF5A94139ED0844134D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>LastReport</td><td>2017-12-15T09:04:57.681Z</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CA6C66A1AE448F53DD9436C942415262</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>DownloadedBytes</td><td>#210425</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CAA6CA2A5CC9ED80D203973A9A2A9A22</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>FullScreenOnAltEnter</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CB49F07092E78F60BBC4A4C8B8889D91</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoSaveWorkspace</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CBED0E99EB9C3907E40675177EF18927</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>QueueKeepDoneItems</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CC324070772BA8430FA0AC124AD25C44</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>VersionHistory</td><td>511027781,stable;511037995,stable</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CC4D1382E05FAF45D0A4B85D73D647E0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SSHLogOmitData</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CC6C9389939E76C926CB7075E2DA9FF9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CCD99FF147B583F35058A80DD669999A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollbackLines</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CD420A7798F14412717B5FFDE96FAD58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.11</td><td>FSProtocol</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CD5D516FA9CF80DC236691795AD8A3AE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>HostKey</td><td>ed25519,ecdsa,rsa,dsa,WARN</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CD687B23B63737705A5D68600E3FBF5A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Bookmarks\ShortCuts</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CD72FD612A93BAD5618E12B5D4E83189</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>PixelsPerInchAxesDiffer</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CD75176B6B9617C04DF3CE7394E4E9BC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour9</td><td>255,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CDA913533D87D8F188625952DD01BF49</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WinTitle</td><td>192.168.200.11 - PuTTY</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CDED7BEDAB0E5EB18C2FF6514C013E53</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoAltScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CE2E614F2121A30D9E7F6E6D8E86BE0F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness128</td><td>1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CE621EF777FD9D6A6FED06EEDE6014E8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>PreserveTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CE666DBCA19E9E627595A6C6ABE5CCCB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Cipher</td><td>aes,chacha20,blowfish,3des,WARN,arcfour,des</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CE6EAF3EDF45C3E3E3210D1F19C2F4E4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>TipsIntervalDays</td><td>#7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CE7BF718BC6DFEAAF7B14ADC99FD7189</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PublicKeyFile</td><td>C:\gui\id_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CEF887F44BEB839D40C1516D9343E8E5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugHMAC2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CF641EBC137DCE64B90FFD7777E50889</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>UpdateStartup</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CFB5CD211A71DE8F2BC430D12B78B80F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\ConsoleWin</td><td>WindowSize</td><td>570,430,96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CFB8BF12025AA033646FC3378D53DA49</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WindowClass</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_CFC27EBB05DDD855A4AF494E7DC7BF92</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AuthGSSAPI</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D03869A6D5BEFD5536DF0DD911E0CDAD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ComposeKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D09C0BB75A71169A05D44D5C24F825FD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>UTF8Override</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D0A24F6BAE08BBD62785F066B3960C9A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WarnOnClose</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D0C8B9E5ED2F61175BA5E9E1955445DB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SSH2DES</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D0DFAAC45BF2D2857D58EE1627CA22AC</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>ResumeSupport</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D0E5631F9429F1F6B2918798F5F0BA5D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.112:22:ssh</td><td>ssh-ed25519%20256%205f:f4:cd:25:bc:95:8c:84:e5:81:6a:06:04:54:0e:48</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D0E9DD0C121A145579857BF238B2D4BF</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideBoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D11342D1729E3DF728F554E2988E4428</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CJKAmbigWide</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D131215DF83259B134672AAF760BD525</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>RemoteMove</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D19E4539D8C84D283F706AA85B2FBD59</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateChecksFailed</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D1A304C8896A3470E5DE4F15D80E0DAB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour21</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D20C2C1DA037D1C0D8DC517370B72060</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CacheDirectoryChangesMaxSize</td><td>#100</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D22C77317134B729BA5AB2EAAE6C07A4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>MouseIsXterm</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D24D19B83D25FC5E02022E1AB7C17FD5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PasteRTF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D26F6488B4795A607B6CEB3A5E1ECDF4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>AutoStartSession</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D27721B5294E8C86AF2FB633A3E3F5C8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ConfirmExitOnCompletion</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D29F2C7EDE3FBCD1819479876F8F0234</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugChanReq</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D2A9281CAB627E465EBBE3ACBA8D0312</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Logging</td><td>LogProtocol</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D301262A59CE02718D0A14B51B160A5E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>HasRule</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D3D22696181A809F922A69BD65239B5F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TCPKeepalives</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D3DEF76E33A0B53E1A2F47F067374019</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>SysColorWindow</td><td>FFFFFF</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D3E3C0E08A967E159041C07A21F77F65</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>TransferMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D441B1A1C04BE6C6E7651A484AAE9A74</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ScrollOnKey</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D4465277EB4439A5A6204BC4B243D860</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BeepInd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D457DBE8F14439E4453F2E9267472FF8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour7</td><td>85,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D480BA3C5874C0F9CFB275751F0E68EB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Note</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D4C17A4B7C06B14B5F91CE80FA445823</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TCPNoDelay</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D50ED30780ACD5950A1D43F7D96684F4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BoldFontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D514A5B25E149D9B947FD8FB76C4F49F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Wordness32</td><td>0,1,2,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D52F825FBA2FE170F9132D6CC946E2C0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RectSelect</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D58AB43721841191FAE31B4A6BC8BC58</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>PreserveRights</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D59771A0A0824A599BFA1452B8D9DB1D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour5</td><td>0,255,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D5A4F90E08573B374D919DC34A4680AD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.4</td><td>HostName</td><td>192.168.200.4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D5E4069B822D6FDD45DFADCC84F27969</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>Runs</td><td>#47</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D70118A13E5AA23FBEE6B85EBE5E5825</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>UsingLocationProfiles</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D701F2C0B7E997A58093E752CDF37754</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MouseSelections</td><td>#84</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D7A48B45443FC274089A186DB960AF6D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WinTitle</td><td>192.168.200.1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D7B3F0E6A6D66C767EF9C28EE347B9F0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LogType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D85FDE4CC3B417DDEB5CFB4567DC808A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ExtensionsDeleted</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D892D5ADC736128C1CF6119485B352A0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SshNoAuth</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D8CA6F40FA92691445C4677D0637E978</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>ExplorerViewStyle</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D914E5C240B378F5FB77D255E9537778</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>NotificationsTimeout</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D914F0247957FACA97F5071965863FC9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>AddXToDirectories</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D91EF4ECBB615EBC2659AF1D40D8FC7A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WinNameAlways</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D932E20D6D62B715A22FDE99B5B34AAD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LogFileClash</td><td>#-1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D936DE3FEF93A66A0F45D3E685A866C0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>DisableBidi</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D9446B5F18952B33657317805AA4FD28</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D95168E87AA355755ABF25473AA58468</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ScrollBar</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D9C26AE5717BAC65C3AE8C3F6C213641</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LocalPortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D9D8E31631E02B2BDF96A94C12CD898C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour2</td><td>0,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_D9F721667B137C0B257589C88ACF4555</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDAllowMove</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DA1632A7F05D9824224C7BB124CD7EE6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TermHeight</td><td>#24</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DA50DEA9629928159D5FF5D9DF0BF4FD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SshBanner</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DA578D85C37FD6319B99E68B30AD0258</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CJKAmbigWide</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DA85E5EE6F6C72B55BA17B2F1A3A38E3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DAFE6101B68EB56C87D87A0F35B2D29D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CRImpliesLF</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DB27C8FA7947662CDF41B1F821E3CCE8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour13</td><td>255,255,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DB5D05155D2404657720F434782D26F6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>UseSystemColours</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DB888E620457F9C4FBDC0D32A95CA0AD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>DECOriginMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DB8D500797BD589403745E72D7D29843</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>IncludeFileMask</td><td>%2A_ctrl</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DB9120E134FF21FC7291FDF06C9583FC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ProxyDNS</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DBC48BBE0D4F9C577463A3EE218FC778</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>HeightPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DBE90FC2A99E235B0F2F10886F1B6902</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>TransferMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DC5BE3E1AEF9B581B00242E0157BFB42</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>KEX</td><td>ecdh,dh-gex-sha1,dh-group14-sha1,rsa,WARN,dh-group1-sha1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DC7BD8F628CDEE1D20500170C92F8E21</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UploadsCommand</td><td>#64</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DC8D5FEA75F8527504C412FFAF433AB3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>NewsUrl</td><td>https://winscp.net/updatenews.php%3Fv=5.11.2.7781&amp;beta=0&amp;to=5.11.3.0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DC941FF6150F5319EB528B9A863A6BBB</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>BalloonNotifications</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DD0ADA424F2E309EB4FD1C77FF5FBDD0</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>CPSLimit</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DD6A565F9AD24DB0E338E4D4A03F7A30</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>Encoding</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DDD8AA083C9CE1F6F5362551CB667A4C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugPlainPW1</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DDDC6931D691EB50835D4712D00FD6CD</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>MaxDownloadTime</td><td>#50</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DE63A5AD02B83DE19E182EFCB813602F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CloseOnExit</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DEC1B8C26C7E6FF35B4FBF9148862B54</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>RefreshRemotePanelInterval</td><td>#x176cc1166cc1463f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DEC728CE7A51A6CA81FC917EA661EBE6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>TCPKeepalives</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DEE7AFD39C9BC7A24B17A4B4574CD638</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Environment</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DF032A9C93CD4CAFF6D20850B601CC89</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\1</td><td>ExternalEditor</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DF1A6100473750B85AF70216D6B1FA6A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Wordness96</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DF27ED5FA2E66C18FAF7654D86D103D5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialParity</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DF654FDBB11A34ED72D7089010D234D5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>SystemContextMenu</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DFA6C24715C276657EA570C22A5FDE57</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LineCodePage</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DFBA0031435B0D76FC1DC0F7EE3FD0E3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ConnectionSharingUpstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DFCDE5FEBBD1AA56D967834435BC4C33</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>CopyParamList</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DFE21227C443868F9875C3F53804750D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SerialLine</td><td>COM1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_DFF50ED81C3C8E25260C169E8D304E3C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialLine</td><td>COM1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E00AEFD092DD3592C55FD44B0F7060EE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>LocalEcho</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E014A172AE022FF63301AE200165EE28</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>LogType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E0165DD11B55C779B7E7C894EF212FE2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BoldAsColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E032751B81F86517D46191B98830EBB7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>IgnorePermErrors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E0FCA4EBDB82E82B9BF055DF20E25483</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TermWidth</td><td>#80</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E146BA94D11EF0455D102C103F581460</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsSCP</td><td>#8</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E17194B6001818D3CF5CAC8CDD73B428</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>ResumeThreshold</td><td>#x0090010000000000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E1A51FB18C33BD4EBEF34E96663DDD65</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>LoginShell</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E1E2E4FE8530D91EF689DFFA539F502F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>3</td><td>2048</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E1F2F4A260A8B5BC965276725D203636</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SynchronizeOptions</td><td>#5</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E21E6DC50BF6FDFA7EFF823137C8467F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>ExplorerKeyboardShortcuts</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E228DC116C23B689193602D98F813F3C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>LastStoredSession</td><td>192.168.200.1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E23EAC1D23D514000D8DBABBBA25CE2B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WinNameAlways</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E2499826588D17E598449EE94280FA00</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour9</td><td>255,85,85</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E2B3702B164BE479B31B6E70BBA87C59</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>PortForwardings</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E2D905B1E1CCCC44C377DCD3E923F400</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>HostKey</td><td>ed25519,ecdsa,rsa,dsa,WARN</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E2DF85D70B57FA014BD76C4E2912D91F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>7</td><td>/usr/lib/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E2FE9F0E6B134AF2B34146C9F9646495</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>Locale</td><td>0409</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E3335D19EF9EFF384D5C0A555E814284</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FindDown</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E354F17270D659F90E800BE496459357</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>WarnOrLargeFileSize</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E387D79E4E45553FF55430A87A64C25E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyMethod</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E393FFFA2D983E106227ADE50BA46EB8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Beep</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E3BB05686B6DE05045D4CB06B36C8C69</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AltF4</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E4037E9C14950C5D11B0CB04F1F6F457</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CurType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E40A2205FB9AE1B49342EE189E0E0592</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>DownloadedBytes</td><td>#4</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E40BA1533DA0F3397CEA4487E288F0A1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WinTitle</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E41750C443D35CD81EDC8A9B29F6A83A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Explorer</td><td>DriveView</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E41B9A31D70BA0B182862D263E70A080</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\LastFingerprints</td><td>192.168.200.124:22:ssh</td><td>ssh-ed25519%20256%2058:0d:4f:09:b2:23:ae:d5:9f:36:0b:54:47:5f:e8:3c</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E421F571B540BDAE9E6FAA88576CAB37</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E4826DDCA9394FDFA35DE378D7E48F95</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugOldGex2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E4B7DBF0E43CC00F3D7412C0FB770FAA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BeepInd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E4DAF1CE537D9EADD4FC61DFCB17717E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>ReplaceText</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E4E11AE32DA424870BFA88D7830BE029</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyDNS</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E5450DC3F9D3B073546D5649B0673EBB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SerialParity</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E55445483D16E719692ABBEBBDCF78F6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BellOverloadS</td><td>#5000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E5721EF0401BB0E810593ABB6F600AA4</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>AltOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E598DED7A3574A3B9AAF7D2AF6E92981</td><td>1</td><td>Software\SimonTatham\PuTTY\Jumplist</td><td>Recent sessions</td><td>[~]Head 1[~]WinSCP temporary session[~]Serial[~]LX-develop</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E5A78AC90472AE982B12474769BC415C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>ContinueOnError</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E5A79274AA9CF9FD7FFA3556D249E814</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>MouseOverride</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E6656C1549666AB583C3CBF2CDA8F46F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>PreserveReadOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E69D20FC5059982AE9EF5E421C997ECE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>MaxUploadSize</td><td>#12188</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E6FBC22B3D9A9591D877A0E0629A0ED4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>Uploads</td><td>#14</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E75AD6345CFDD36EFD2CC3698FBAFCEA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\LoginDialog</td><td>WindowSize</td><td>640,430,96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E7A6EFBAD3C69FA4D1B9930EB6B947E9</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>GenerateUrlCodeTarget</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E7B1C7323900B3C2E2EA16D4225D6733</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\SynchronizeChecklist</td><td>ListParams</td><td>1;1|150,1;100,1;80,1;130,1;25,1;100,1;80,1;130,1;@96|0;1;2;3;4;5;6;7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E7BB1A6B0A22CB5039D7C3812F0198E2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>DownloadTime</td><td>#56</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E80499178AA46A100FE05DBC8DBB7E7E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountFTP</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E81241E1794725675C289E0E86B7FDD3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugRekey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E831A4220DCD3A2FCABE5DD5C63F7FE5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Ssh2DES</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E863F3228A0C48CDBD2F64E3A9C9ED97</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>WorkAreaHeight</td><td>1040</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E8771AB42550FD85BD8474B76CCF289C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander\LocalPanel</td><td>DriveViewWidthPixelsPerInch</td><td>#96</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E8988CE0A49494A49AFEBB56AFCD6873</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugDeriveKey2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E8B2CC0EBFD54890BBF51FCF19E9ADDA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>NewerOnly</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E8D6EC5881966DE42488E09B0802D934</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SunkenEdge</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E8DC45DC6F716B246DEE6622CBFAC88D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>FlipChildrenOverride</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E9017511CC6C6B1BD4EC5B93024FA837</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>FullRowSelect</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E93B21A89BE9EB97CC7F5D537399197B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NoMouseReporting</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E9B71299D3ABF6F17A707C00C248B2A0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ANSIColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E9BBA03FF116F53B71A7EB5A51D3DD6C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>Masks</td><td>%2A.%2Ahtml;%20%2A.htm;%20%2A.txt;%20%2A.php;%20%2A.php3;%20%2A.cgi;%20%2A.c;%20%2A.cpp;%20%2A.h;%20%2A.pas;%20%2A.bas;%20%2A.tex;%20%2A.pl;%20%2A.js;%20.htaccess;%20%2A.xtml;%20%2A.css;%20%2A.cfg;%20%2A.ini;%20%2A.sh;%20%2A.xml</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E9C25386B5BCE5302E6567C1817BCAA5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>CapsLockCyr</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_E9D8992A893A28EDA454EAF08E38E23C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>KeepUpToDateChangeDelay</td><td>#500</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EA05406B3F5B91CAC07FA813B21094AD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>PortForwardings</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EA1E814A6FB660BA5021231CCB318CCC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugOldGex2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EA43A40F2B0F5BD496CEFFC648E36101</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>RekeyBytes</td><td>1G</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EA62C875CDEB5D4B68F1D3285C0E85C7</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EA7F0E621A4D3BBD4416F3C31A475FF2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WideBoldFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EABBA96DE9123FF9D0F757527FD3D5AD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WarnOnClose</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EB133FB8894A581975E3492062878156</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDWarnLackOfTempSpaceRatio</td><td>#x9a9999999999f13f</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EB6F4E68D2ED6C6D9277EB2E70DCD16A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>CopyOnDoubleClickConfirmation</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EB8582002149A1115E7FEDB517ED170F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>Text</td><td>rwxr-xr-x</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EBCA4A1215F4E24EA3A0B868287D715B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>UserName</td><td>root</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EC05813FF5768C19A3C08F00C258CDEB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BugMaxPkt2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EC3B7CD23785A895DF5A1B1741E7DF0F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>RemoveBOM</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EC618D6334BF4A49C6E9C5287BC06E91</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BCE</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EC6D82732396BD2F706981495924B4C8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Answerback</td><td>PuTTY</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ECA3A1A5F955C1238AD1E0F7C03D9032</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\SshHostKeys</td><td>ssh-ed25519@22:localhost</td><td>0x52ab6d39f5671a435b58fe855afd5c0d052970c2020d5a3e48e7c03bc2c93682,0x65d3592100a648ea7b9e269b6f3ad5633d9c781670ecea400429bd66150e3749</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ECA8C65EA108F0750FB6C4E61B4CE37B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>TerminalModes</td><td>CS7=A,CS8=A,DISCARD=A,DSUSP=A,ECHO=A,ECHOCTL=A,ECHOE=A,ECHOK=A,ECHOKE=A,ECHONL=A,EOF=A,EOL=A,EOL2=A,ERASE=A,FLUSH=A,ICANON=A,ICRNL=A,IEXTEN=A,IGNCR=A,IGNPAR=A,IMAXBEL=A,INLCR=A,INPCK=A,INTR=A,ISIG=A,ISTRIP=A,IUCLC=A,IUTF8=A,IXANY=A,IXOFF=A,IXON=A,KILL=A,LNEXT=A,NOFLSH=A,OCRNL=A,OLCUC=A,ONLCR=A,ONLRET=A,ONOCR=A,OPOST=A,PARENB=A,PARMRK=A,PARODD=A,PENDIN=A,QUIT=A,REPRINT=A,START=A,STATUS=A,STOP=A,SUSP=A,SWTCH=A,TOSTOP=A,WERASE=A,XCASE=A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ECB68EAF0EF8E0C2C2A61C5DFC0F5525</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RemoteQTitleAction</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ECDBBF4CAAF4921858E7D5CBBB8A1BB0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>X11AuthType</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ECF1AB1F2A9D2747B3B1CCD82A10BE77</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>StoredSessionsCountAdvanced</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ED32A182576E05DD68CE5680A13EFBCB</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BoldAsColour</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ED53118A277F903761C97544DC2EC2E1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyTelnetCommand</td><td>connect %host %port\n</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_ED875055CB479851CACCF613EB2FBDD5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FontCharSet</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EDE1FA559E02F17BB6D8221CE71C78F2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\SpeedLimit</td><td>7</td><td>128</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EE20730D7649660DFB40F6FBE97B12E4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FontName2</td><td>Consolas</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EE3FE336FC91824A5772C1C7F19724B5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>UrlButton</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EE4EC5E8E1D95D2EF965FECCED12A76C</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SynchronizeMode</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EE516D6229F825D2E1AF22C8E72CE4FD</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>ProxyHost</td><td>proxy</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EE56FD0AD3C10CE4304D3C48C23CEE7D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>HasRule</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EEB49B3401108114D0CF8631010B223E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour1</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EED9A418697F30029D465549D5410FE6</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Wordness96</td><td>1,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,1,1,1,1,1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EEDCFD0B32A1418BDCA2CCE681A1B45D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Colour1</td><td>255,255,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EEE82A463F027B2F784A58E28C864DC9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>CtrlAltKeys</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EF5445E9249EF45A824140C3503BBC83</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>OpenedSessionsFailedLastDate</td><td>2017-12-18</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EFCE1BF757EE09FC3C61EDC1952C530E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>BetaVersions</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_EFD75C0E27B2F44B081023866B28D52E</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>AnyBetaUsed</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F006D811F068EF1147BD495460E537F6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>TemporaryDirectoryCleanups</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F09201D4DAEEC943DC7FA8B0612B37A3</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Bookmarks\Options</td><td>*</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F09B2B6F9E6EEEA3BABECFC40ABFCD42</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UploadedBytes</td><td>#354920</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F0D54C9B9BBDEDF14F6FD03B8BA115A6</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>1</td><td>/opt/radex/bin/head/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F0F54BE48BBA124029B2A73BC1AAF21D</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>ReplaceInvalidChars</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F194001E6EBA807C29C050958B712532</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoRemoteResize</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F2438DEEE31E8C4432BB31D40FF4488E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>RemotePortAcceptAll</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F26D080B100B37BCA4EA321461528781</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>GSSLibs</td><td>gssapi32,sspi,custom</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F26DAF5C5CC499C5D79D5AEAA748111A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F27666D0DD32963621769A34F5C5FCB2</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\1</td><td>LocalInvalidChars</td><td>/%5C:%2A%3F"&lt;&gt;|</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F286114B695E95745CBBEA8777FEE3E8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyLocalhost</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F289ADC2769635274BCFE432AC69C5F8</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\QueueView</td><td>Label</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F29DDC3A8CFADF0CBF1156AB7BEB0FB9</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>SerialStopHalfbits</td><td>#2</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F35974F4886CDF9B650799E5D9F265DE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>MouseOverride</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F3D5685DD589E4CA74A09ADF48125B0B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>UpdateNotifications</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F3E865781A1E79CEB69324967A489DDA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Sessions\192.168.200.9</td><td>PublicKeyFile</td><td>C:%5Cgui%5Cid_rsa.ppk</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F406BE8EAA5F5151072C35B26A5386D2</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>CurType</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F408B8611C6335BE453AA031EDCA9B43</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>SshProt</td><td>#3</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F4141295375D2CEAEAA71A928F49FB53</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>EncodingMultiByteAnsi</td><td>0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F431F7C43E8B63696F16B271A955E1A0</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BellOverloadS</td><td>#5000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F44A3A7ACBB64A70B5DA11D7B9FB61D1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\PeriodCounters</td><td>OpenedSessionsSSH2</td><td>#7</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F455300E1DBA6082C56E644F47276D18</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>AlwaysOnTop</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F46B273C5DB6B3714D3EA1D8D05C810F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>FontHeight</td><td>#10</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F4F1B9C7AC9B3EC19F6E43362E81ED4A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour17</td><td>255,85,255</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F4FB43C3DC119826D18B2C82000D44BE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SunkenEdge</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F50E53A40812B9167D2176C719694AC3</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideBoldFont</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F5AD9054F58624341E6878549A89EACA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\LifetimeCounters</td><td>InstallationsGettingStarted</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F5C0AFF3ABDCABEB698C82EB1DE80D29</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor\2</td><td>FileMask</td><td>%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F5D03450616C9DE2C7C6AACBFCBF002B</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>BellOverloadT</td><td>#2000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F617DE2405E1A9D675E91F84DDF680DA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AddressFamily</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F627194AB538FCE2305817961D1F7134</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>AlwaysOnTop</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F63630ECE8591E3A8DC9101E5710767F</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam</td><td>IncludeFileMask</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F68A401B28C3233C35B0F5086F58D25F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoApplicationCursors</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F6920C13A57E98F7B8EE4E5A1ED6052F</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour16</td><td>187,0,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F6E3E8D4EDDE83562F436FBF35EDE0C1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ProxyLogToTerm</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F752B12960A2AA6BDC9DF70BCB164E2E</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BackspaceIsDelete</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F759B16981636EEE9E860DEDFD966C3C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>WideFontIsBold</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F84D5E0BEFAA8BAE656CC88ED2DAE756</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>GSSCustom</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F8504F9F61DD29863F07D27ED3E48409</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>BugChanReq</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F8CA6B0941A7F91283CC5F632B8BDF4A</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F8E2507712B6F25D47893F6435660B63</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>Protocol</td><td>ssh</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F8E613888938DA5E8E1AAD4E3EE33DCC</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>ApplicationKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_F95CC4AF341536A8F83F8FE79359A7EA</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>GssapiFwd</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FA296CE3CE779E031AE8CEAFB9AB1D8B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>RefreshRemotePanel</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FA4DC2D7852511279C4C80995807468C</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour8</td><td>187,0,0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FA4E12370327CE961699BEAB5CFAA123</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>SSHLogOmitPasswords</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FA79937B3B94C4C41C8A4A39674AD2EE</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\3</td><td>ClearArchive</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FAA874784A09F1FF6DA557A7EAF0B4BF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>SelectMask</td><td>%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FAC84F8AED049EC6F1D8D42F5E02C5FF</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Editor</td><td>FontStyle</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FAE036244ECCD8986F597DE61E4B02C5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface</td><td>DDExtTimeout</td><td>#1000</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FB189A41116A19421E771856CEA79B5B</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\0</td><td>Masks</td><td/><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FBA1623563AC96371F2BC5C9313E6BBE</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>ConnectionSharingDownstream</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FC46D6427E2B1D1AE9C737E6D238D846</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>NetHackKeypad</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FC76567D638C3B39455E8DFDE536CD7A</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>WindowBorder</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FCA7DE539A868F8C1315E8076DE78E69</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>TerminalSpeed</td><td>38400,38400</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FCDA9D2F8C3B37D6FF902EF681F462A4</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\4</td><td>RemoveCtrlZ</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FD18C8396EB9525133C799E3DC1D57FA</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Usage\Values</td><td>NetFrameworkVersion</td><td>4.7.02558</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FD964E9E3193FD1264A1075750696EB8</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>WideBoldFontHeight</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FD987D5B0A95BB7E42D7708D788D0EA5</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\CopyParam\2</td><td>FileNameCase</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FD9C6B596BC5671162657B9E14E2F477</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>NoAltScreen</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FE17D77B5307AB4FEF592E200FE87E02</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>Colour0</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FEB1809C80F708A3FA4FA47D2F7F877D</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\WinSCP%20temporary%20session</td><td>BugRSAPad2</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FEC1E6D6A6A66F182A80837A21460BF5</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Head%201</td><td>DisableBidi</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FECD6267AA10D7858698A57DBB8A2EC1</td><td>1</td><td>Software\SimonTatham\PuTTY\Sessions\Default%20Settings</td><td>Colour20</td><td>187,187,187</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FEE4312500EFD136B6A427E248DD77D1</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\History\RemoteTarget</td><td>6</td><td>/usr/local/lib/%2A.%2A</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FF8DC799562072CDEF49866ACEEEDD91</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Commander</td><td>CompareByTime</td><td>#1</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
		<row><td>_FFCF8CFBDFEEDA68B822BFB1B9633190</td><td>1</td><td>Software\Martin Prikryl\WinSCP 2\Configuration\Interface\Updates</td><td>Disabled</td><td>#0</td><td>ISX_DEFAULTCOMPONENT2</td><td/></row>
	</table>

	<table name="RemoveFile">
		<col key="yes" def="s72">FileKey</col>
		<col def="s72">Component_</col>
		<col def="L255">FileName</col>
		<col def="s72">DirProperty</col>
		<col def="i2">InstallMode</col>
	</table>

	<table name="RemoveIniFile">
		<col key="yes" def="s72">RemoveIniFile</col>
		<col def="l255">FileName</col>
		<col def="S72">DirProperty</col>
		<col def="l96">Section</col>
		<col def="l128">Key</col>
		<col def="L255">Value</col>
		<col def="i2">Action</col>
		<col def="s72">Component_</col>
	</table>

	<table name="RemoveRegistry">
		<col key="yes" def="s72">RemoveRegistry</col>
		<col def="i2">Root</col>
		<col def="l255">Key</col>
		<col def="L255">Name</col>
		<col def="s72">Component_</col>
	</table>

	<table name="ReserveCost">
		<col key="yes" def="s72">ReserveKey</col>
		<col def="s72">Component_</col>
		<col def="S72">ReserveFolder</col>
		<col def="i4">ReserveLocal</col>
		<col def="i4">ReserveSource</col>
	</table>

	<table name="SFPCatalog">
		<col key="yes" def="s255">SFPCatalog</col>
		<col def="V0">Catalog</col>
		<col def="S0">Dependency</col>
	</table>

	<table name="SelfReg">
		<col key="yes" def="s72">File_</col>
		<col def="I2">Cost</col>
	</table>

	<table name="ServiceControl">
		<col key="yes" def="s72">ServiceControl</col>
		<col def="s255">Name</col>
		<col def="i2">Event</col>
		<col def="S255">Arguments</col>
		<col def="I2">Wait</col>
		<col def="s72">Component_</col>
	</table>

	<table name="ServiceInstall">
		<col key="yes" def="s72">ServiceInstall</col>
		<col def="s255">Name</col>
		<col def="L255">DisplayName</col>
		<col def="i4">ServiceType</col>
		<col def="i4">StartType</col>
		<col def="i4">ErrorControl</col>
		<col def="S255">LoadOrderGroup</col>
		<col def="S255">Dependencies</col>
		<col def="S255">StartName</col>
		<col def="S255">Password</col>
		<col def="S255">Arguments</col>
		<col def="s72">Component_</col>
		<col def="L255">Description</col>
	</table>

	<table name="Shortcut">
		<col key="yes" def="s72">Shortcut</col>
		<col def="s72">Directory_</col>
		<col def="l128">Name</col>
		<col def="s72">Component_</col>
		<col def="s255">Target</col>
		<col def="S255">Arguments</col>
		<col def="L255">Description</col>
		<col def="I2">Hotkey</col>
		<col def="S72">Icon_</col>
		<col def="I2">IconIndex</col>
		<col def="I2">ShowCmd</col>
		<col def="S72">WkDir</col>
		<col def="S255">DisplayResourceDLL</col>
		<col def="I2">DisplayResourceId</col>
		<col def="S255">DescriptionResourceDLL</col>
		<col def="I2">DescriptionResourceId</col>
		<col def="S255">ISComments</col>
		<col def="S255">ISShortcutName</col>
		<col def="I4">ISAttributes</col>
	</table>

	<table name="Signature">
		<col key="yes" def="s72">Signature</col>
		<col def="s255">FileName</col>
		<col def="S20">MinVersion</col>
		<col def="S20">MaxVersion</col>
		<col def="I4">MinSize</col>
		<col def="I4">MaxSize</col>
		<col def="I4">MinDate</col>
		<col def="I4">MaxDate</col>
		<col def="S255">Languages</col>
	</table>

	<table name="TextStyle">
		<col key="yes" def="s72">TextStyle</col>
		<col def="s32">FaceName</col>
		<col def="i2">Size</col>
		<col def="I4">Color</col>
		<col def="I2">StyleBits</col>
		<row><td>Arial8</td><td>Arial</td><td>8</td><td/><td/></row>
		<row><td>Arial9</td><td>Arial</td><td>9</td><td/><td/></row>
		<row><td>ArialBlue10</td><td>Arial</td><td>10</td><td>16711680</td><td/></row>
		<row><td>ArialBlueStrike10</td><td>Arial</td><td>10</td><td>16711680</td><td>8</td></row>
		<row><td>CourierNew8</td><td>Courier New</td><td>8</td><td/><td/></row>
		<row><td>CourierNew9</td><td>Courier New</td><td>9</td><td/><td/></row>
		<row><td>ExpressDefault</td><td>Tahoma</td><td>8</td><td/><td/></row>
		<row><td>MSGothic9</td><td>MS Gothic</td><td>9</td><td/><td/></row>
		<row><td>MSSGreySerif8</td><td>MS Sans Serif</td><td>8</td><td>8421504</td><td/></row>
		<row><td>MSSWhiteSerif8</td><td>Tahoma</td><td>8</td><td>16777215</td><td/></row>
		<row><td>MSSansBold8</td><td>Tahoma</td><td>8</td><td/><td>1</td></row>
		<row><td>MSSansSerif8</td><td>MS Sans Serif</td><td>8</td><td/><td/></row>
		<row><td>MSSansSerif9</td><td>MS Sans Serif</td><td>9</td><td/><td/></row>
		<row><td>Tahoma10</td><td>Tahoma</td><td>10</td><td/><td/></row>
		<row><td>Tahoma8</td><td>Tahoma</td><td>8</td><td/><td/></row>
		<row><td>Tahoma9</td><td>Tahoma</td><td>9</td><td/><td/></row>
		<row><td>TahomaBold10</td><td>Tahoma</td><td>10</td><td/><td>1</td></row>
		<row><td>TahomaBold8</td><td>Tahoma</td><td>8</td><td/><td>1</td></row>
		<row><td>Times8</td><td>Times New Roman</td><td>8</td><td/><td/></row>
		<row><td>Times9</td><td>Times New Roman</td><td>9</td><td/><td/></row>
		<row><td>TimesItalic12</td><td>Times New Roman</td><td>12</td><td/><td>2</td></row>
		<row><td>TimesItalicBlue10</td><td>Times New Roman</td><td>10</td><td>16711680</td><td>2</td></row>
		<row><td>TimesRed16</td><td>Times New Roman</td><td>16</td><td>255</td><td/></row>
		<row><td>VerdanaBold14</td><td>Verdana</td><td>13</td><td/><td>1</td></row>
	</table>

	<table name="TypeLib">
		<col key="yes" def="s38">LibID</col>
		<col key="yes" def="i2">Language</col>
		<col key="yes" def="s72">Component_</col>
		<col def="I4">Version</col>
		<col def="L128">Description</col>
		<col def="S72">Directory_</col>
		<col def="s38">Feature_</col>
		<col def="I4">Cost</col>
	</table>

	<table name="UIText">
		<col key="yes" def="s72">Key</col>
		<col def="L255">Text</col>
		<row><td>AbsentPath</td><td/></row>
		<row><td>GB</td><td>##IDS_UITEXT_GB##</td></row>
		<row><td>KB</td><td>##IDS_UITEXT_KB##</td></row>
		<row><td>MB</td><td>##IDS_UITEXT_MB##</td></row>
		<row><td>MenuAbsent</td><td>##IDS_UITEXT_FeatureNotAvailable##</td></row>
		<row><td>MenuAdvertise</td><td>##IDS_UITEXT_FeatureInstalledWhenRequired2##</td></row>
		<row><td>MenuAllCD</td><td>##IDS_UITEXT_FeatureInstalledCD##</td></row>
		<row><td>MenuAllLocal</td><td>##IDS_UITEXT_FeatureInstalledLocal##</td></row>
		<row><td>MenuAllNetwork</td><td>##IDS_UITEXT_FeatureInstalledNetwork##</td></row>
		<row><td>MenuCD</td><td>##IDS_UITEXT_FeatureInstalledCD2##</td></row>
		<row><td>MenuLocal</td><td>##IDS_UITEXT_FeatureInstalledLocal2##</td></row>
		<row><td>MenuNetwork</td><td>##IDS_UITEXT_FeatureInstalledNetwork2##</td></row>
		<row><td>NewFolder</td><td>##IDS_UITEXT_Folder##</td></row>
		<row><td>SelAbsentAbsent</td><td>##IDS_UITEXT_GB##</td></row>
		<row><td>SelAbsentAdvertise</td><td>##IDS_UITEXT_FeatureInstalledWhenRequired##</td></row>
		<row><td>SelAbsentCD</td><td>##IDS_UITEXT_FeatureOnCD##</td></row>
		<row><td>SelAbsentLocal</td><td>##IDS_UITEXT_FeatureLocal##</td></row>
		<row><td>SelAbsentNetwork</td><td>##IDS_UITEXT_FeatureNetwork##</td></row>
		<row><td>SelAdvertiseAbsent</td><td>##IDS_UITEXT_FeatureUnavailable##</td></row>
		<row><td>SelAdvertiseAdvertise</td><td>##IDS_UITEXT_FeatureInstalledRequired##</td></row>
		<row><td>SelAdvertiseCD</td><td>##IDS_UITEXT_FeatureOnCD2##</td></row>
		<row><td>SelAdvertiseLocal</td><td>##IDS_UITEXT_FeatureLocal2##</td></row>
		<row><td>SelAdvertiseNetwork</td><td>##IDS_UITEXT_FeatureNetwork2##</td></row>
		<row><td>SelCDAbsent</td><td>##IDS_UITEXT_FeatureWillBeUninstalled##</td></row>
		<row><td>SelCDAdvertise</td><td>##IDS_UITEXT_FeatureWasCD##</td></row>
		<row><td>SelCDCD</td><td>##IDS_UITEXT_FeatureRunFromCD##</td></row>
		<row><td>SelCDLocal</td><td>##IDS_UITEXT_FeatureWasCDLocal##</td></row>
		<row><td>SelChildCostNeg</td><td>##IDS_UITEXT_FeatureFreeSpace##</td></row>
		<row><td>SelChildCostPos</td><td>##IDS_UITEXT_FeatureRequiredSpace##</td></row>
		<row><td>SelCostPending</td><td>##IDS_UITEXT_CompilingFeaturesCost##</td></row>
		<row><td>SelLocalAbsent</td><td>##IDS_UITEXT_FeatureCompletelyRemoved##</td></row>
		<row><td>SelLocalAdvertise</td><td>##IDS_UITEXT_FeatureRemovedUnlessRequired##</td></row>
		<row><td>SelLocalCD</td><td>##IDS_UITEXT_FeatureRemovedCD##</td></row>
		<row><td>SelLocalLocal</td><td>##IDS_UITEXT_FeatureRemainLocal##</td></row>
		<row><td>SelLocalNetwork</td><td>##IDS_UITEXT_FeatureRemoveNetwork##</td></row>
		<row><td>SelNetworkAbsent</td><td>##IDS_UITEXT_FeatureUninstallNoNetwork##</td></row>
		<row><td>SelNetworkAdvertise</td><td>##IDS_UITEXT_FeatureWasOnNetworkInstalled##</td></row>
		<row><td>SelNetworkLocal</td><td>##IDS_UITEXT_FeatureWasOnNetworkLocal##</td></row>
		<row><td>SelNetworkNetwork</td><td>##IDS_UITEXT_FeatureContinueNetwork##</td></row>
		<row><td>SelParentCostNegNeg</td><td>##IDS_UITEXT_FeatureSpaceFree##</td></row>
		<row><td>SelParentCostNegPos</td><td>##IDS_UITEXT_FeatureSpaceFree2##</td></row>
		<row><td>SelParentCostPosNeg</td><td>##IDS_UITEXT_FeatureSpaceFree3##</td></row>
		<row><td>SelParentCostPosPos</td><td>##IDS_UITEXT_FeatureSpaceFree4##</td></row>
		<row><td>TimeRemaining</td><td>##IDS_UITEXT_TimeRemaining##</td></row>
		<row><td>VolumeCostAvailable</td><td>##IDS_UITEXT_Available##</td></row>
		<row><td>VolumeCostDifference</td><td>##IDS_UITEXT_Differences##</td></row>
		<row><td>VolumeCostRequired</td><td>##IDS_UITEXT_Required##</td></row>
		<row><td>VolumeCostSize</td><td>##IDS_UITEXT_DiskSize##</td></row>
		<row><td>VolumeCostVolume</td><td>##IDS_UITEXT_Volume##</td></row>
		<row><td>bytes</td><td>##IDS_UITEXT_Bytes##</td></row>
	</table>

	<table name="Upgrade">
		<col key="yes" def="s38">UpgradeCode</col>
		<col key="yes" def="S20">VersionMin</col>
		<col key="yes" def="S20">VersionMax</col>
		<col key="yes" def="S255">Language</col>
		<col key="yes" def="i4">Attributes</col>
		<col def="S255">Remove</col>
		<col def="s72">ActionProperty</col>
		<col def="S72">ISDisplayName</col>
		<row><td>{30E2AA7C-E65E-461C-90B8-741957491142}</td><td></td><td>1.00.0000</td><td></td><td>768</td><td/><td>ISACTIONPROP1</td><td>NewUpgradeEntry1</td></row>
	</table>

	<table name="Verb">
		<col key="yes" def="s255">Extension_</col>
		<col key="yes" def="s32">Verb</col>
		<col def="I2">Sequence</col>
		<col def="L255">Command</col>
		<col def="L255">Argument</col>
	</table>

	<table name="_Validation">
		<col key="yes" def="s32">Table</col>
		<col key="yes" def="s32">Column</col>
		<col def="s4">Nullable</col>
		<col def="I4">MinValue</col>
		<col def="I4">MaxValue</col>
		<col def="S255">KeyTable</col>
		<col def="I2">KeyColumn</col>
		<col def="S32">Category</col>
		<col def="S255">Set</col>
		<col def="S255">Description</col>
		<row><td>ActionText</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to be described.</td></row>
		<row><td>ActionText</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized description displayed in progress dialog and log when action is executing.</td></row>
		<row><td>ActionText</td><td>Template</td><td>Y</td><td/><td/><td/><td/><td>Template</td><td/><td>Optional localized format template used to format action data records for display during action execution.</td></row>
		<row><td>AdminExecuteSequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>AdminExecuteSequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>AdminExecuteSequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>AdminExecuteSequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>AdminExecuteSequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>AdminUISequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>AdminUISequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>AdminUISequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>AdminUISequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>AdminUISequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>AdvtExecuteSequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>AdvtExecuteSequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>AdvtExecuteSequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>AdvtExecuteSequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>AdvtExecuteSequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>AdvtUISequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>AdvtUISequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>AdvtUISequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>AdvtUISequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>AdvtUISequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>AppId</td><td>ActivateAtStorage</td><td>Y</td><td>0</td><td>1</td><td/><td/><td/><td/><td/></row>
		<row><td>AppId</td><td>AppId</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td/></row>
		<row><td>AppId</td><td>DllSurrogate</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>AppId</td><td>LocalService</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>AppId</td><td>RemoteServerName</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td/></row>
		<row><td>AppId</td><td>RunAsInteractiveUser</td><td>Y</td><td>0</td><td>1</td><td/><td/><td/><td/><td/></row>
		<row><td>AppId</td><td>ServiceParameters</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>AppSearch</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The property associated with a Signature</td></row>
		<row><td>AppSearch</td><td>Signature_</td><td>N</td><td/><td/><td>ISXmlLocator;Signature</td><td>1</td><td>Identifier</td><td/><td>The Signature_ represents a unique file signature and is also the foreign key in the Signature,  RegLocator, IniLocator, CompLocator and the DrLocator tables.</td></row>
		<row><td>BBControl</td><td>Attributes</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>A 32-bit word that specifies the attribute flags to be applied to this control.</td></row>
		<row><td>BBControl</td><td>BBControl</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the control. This name must be unique within a billboard, but can repeat on different billboard.</td></row>
		<row><td>BBControl</td><td>Billboard_</td><td>N</td><td/><td/><td>Billboard</td><td>1</td><td>Identifier</td><td/><td>External key to the Billboard table, name of the billboard.</td></row>
		<row><td>BBControl</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Height of the bounding rectangle of the control.</td></row>
		<row><td>BBControl</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>A string used to set the initial text contained within a control (if appropriate).</td></row>
		<row><td>BBControl</td><td>Type</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The type of the control.</td></row>
		<row><td>BBControl</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Width of the bounding rectangle of the control.</td></row>
		<row><td>BBControl</td><td>X</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Horizontal coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>BBControl</td><td>Y</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Vertical coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>Billboard</td><td>Action</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The name of an action. The billboard is displayed during the progress messages received from this action.</td></row>
		<row><td>Billboard</td><td>Billboard</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the billboard.</td></row>
		<row><td>Billboard</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>An external key to the Feature Table. The billboard is shown only if this feature is being installed.</td></row>
		<row><td>Billboard</td><td>Ordering</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>A positive integer. If there is more than one billboard corresponding to an action they will be shown in the order defined by this column.</td></row>
		<row><td>Binary</td><td>Data</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>Binary stream. The binary icon data in PE (.DLL or .EXE) or icon (.ICO) format.</td></row>
		<row><td>Binary</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to the ICO or EXE file.</td></row>
		<row><td>Binary</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Unique key identifying the binary data.</td></row>
		<row><td>BindImage</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>The index into the File table. This must be an executable file.</td></row>
		<row><td>BindImage</td><td>Path</td><td>Y</td><td/><td/><td/><td/><td>Paths</td><td/><td>A list of ;  delimited paths that represent the paths to be searched for the import DLLS. The list is usually a list of properties each enclosed within square brackets [] .</td></row>
		<row><td>CCPSearch</td><td>Signature_</td><td>N</td><td/><td/><td>Signature</td><td>1</td><td>Identifier</td><td/><td>The Signature_ represents a unique file signature and is also the foreign key in the Signature,  RegLocator, IniLocator, CompLocator and the DrLocator tables.</td></row>
		<row><td>CheckBox</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A named property to be tied to the item.</td></row>
		<row><td>CheckBox</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value string associated with the item.</td></row>
		<row><td>Class</td><td>AppId_</td><td>Y</td><td/><td/><td>AppId</td><td>1</td><td>Guid</td><td/><td>Optional AppID containing DCOM information for associated application (string GUID).</td></row>
		<row><td>Class</td><td>Argument</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>optional argument for LocalServers.</td></row>
		<row><td>Class</td><td>Attributes</td><td>Y</td><td/><td>32767</td><td/><td/><td/><td/><td>Class registration attributes.</td></row>
		<row><td>Class</td><td>CLSID</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>The CLSID of an OLE factory.</td></row>
		<row><td>Class</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table, specifying the component for which to return a path when called through LocateComponent.</td></row>
		<row><td>Class</td><td>Context</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The numeric server context for this server. CLSCTX_xxxx</td></row>
		<row><td>Class</td><td>DefInprocHandler</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td>1;2;3</td><td>Optional default inproc handler.  Only optionally provided if Context=CLSCTX_LOCAL_SERVER.  Typically "ole32.dll" or "mapi32.dll"</td></row>
		<row><td>Class</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized description for the Class.</td></row>
		<row><td>Class</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Feature Table, specifying the feature to validate or install in order for the CLSID factory to be operational.</td></row>
		<row><td>Class</td><td>FileTypeMask</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Optional string containing information for the HKCRthis CLSID) key. If multiple patterns exist, they must be delimited by a semicolon, and numeric subkeys will be generated: 0,1,2...</td></row>
		<row><td>Class</td><td>IconIndex</td><td>Y</td><td>-32767</td><td>32767</td><td/><td/><td/><td/><td>Optional icon index.</td></row>
		<row><td>Class</td><td>Icon_</td><td>Y</td><td/><td/><td>Icon</td><td>1</td><td>Identifier</td><td/><td>Optional foreign key into the Icon Table, specifying the icon file associated with this CLSID. Will be written under the DefaultIcon key.</td></row>
		<row><td>Class</td><td>ProgId_Default</td><td>Y</td><td/><td/><td>ProgId</td><td>1</td><td>Text</td><td/><td>Optional ProgId associated with this CLSID.</td></row>
		<row><td>ComboBox</td><td>Order</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>A positive integer used to determine the ordering of the items within one list.	The integers do not have to be consecutive.</td></row>
		<row><td>ComboBox</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A named property to be tied to this item. All the items tied to the same property become part of the same combobox.</td></row>
		<row><td>ComboBox</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The visible text to be assigned to the item. Optional. If this entry or the entire column is missing, the text is the same as the value.</td></row>
		<row><td>ComboBox</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value string associated with this item. Selecting the line will set the associated property to this value.</td></row>
		<row><td>CompLocator</td><td>ComponentId</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>A string GUID unique to this component, version, and language.</td></row>
		<row><td>CompLocator</td><td>Signature_</td><td>N</td><td/><td/><td>Signature</td><td>1</td><td>Identifier</td><td/><td>The table key. The Signature_ represents a unique file signature and is also the foreign key in the Signature table.</td></row>
		<row><td>CompLocator</td><td>Type</td><td>Y</td><td>0</td><td>1</td><td/><td/><td/><td/><td>A boolean value that determines if the registry value is a filename or a directory location.</td></row>
		<row><td>Complus</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the ComPlus component.</td></row>
		<row><td>Complus</td><td>ExpType</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>ComPlus component attributes.</td></row>
		<row><td>Component</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Remote execution option, one of irsEnum</td></row>
		<row><td>Component</td><td>Component</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular component record.</td></row>
		<row><td>Component</td><td>ComponentId</td><td>Y</td><td/><td/><td/><td/><td>Guid</td><td/><td>A string GUID unique to this component, version, and language.</td></row>
		<row><td>Component</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>A conditional statement that will disable this component if the specified condition evaluates to the 'True' state. If a component is disabled, it will not be installed, regardless of the 'Action' state associated with the component.</td></row>
		<row><td>Component</td><td>Directory_</td><td>N</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Required key of a Directory table record. This is actually a property name whose value contains the actual path, set either by the AppSearch action or with the default setting obtained from the Directory table.</td></row>
		<row><td>Component</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties of a component.</td></row>
		<row><td>Component</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>User Comments.</td></row>
		<row><td>Component</td><td>ISDotNetInstallerArgsCommit</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Arguments passed to the key file of the component if if implements the .NET Installer class</td></row>
		<row><td>Component</td><td>ISDotNetInstallerArgsInstall</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Arguments passed to the key file of the component if if implements the .NET Installer class</td></row>
		<row><td>Component</td><td>ISDotNetInstallerArgsRollback</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Arguments passed to the key file of the component if if implements the .NET Installer class</td></row>
		<row><td>Component</td><td>ISDotNetInstallerArgsUninstall</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Arguments passed to the key file of the component if if implements the .NET Installer class</td></row>
		<row><td>Component</td><td>ISRegFileToMergeAtBuild</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Path and File name of a .REG file to merge into the component at build time.</td></row>
		<row><td>Component</td><td>ISScanAtBuildFile</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>File used by the Dot Net scanner to populate dependant assemblies' File_Application field.</td></row>
		<row><td>Component</td><td>KeyPath</td><td>Y</td><td/><td/><td>File;ODBCDataSource;Registry</td><td>1</td><td>Identifier</td><td/><td>Either the primary key into the File table, Registry table, or ODBCDataSource table. This extract path is stored when the component is installed, and is used to detect the presence of the component and to return the path to it.</td></row>
		<row><td>Condition</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Expression evaluated to determine if Level in the Feature table is to change.</td></row>
		<row><td>Condition</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Reference to a Feature entry in Feature table.</td></row>
		<row><td>Condition</td><td>Level</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>New selection Level to set in Feature table if Condition evaluates to TRUE.</td></row>
		<row><td>Control</td><td>Attributes</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>A 32-bit word that specifies the attribute flags to be applied to this control.</td></row>
		<row><td>Control</td><td>Binary_</td><td>Y</td><td/><td/><td>Binary</td><td>1</td><td>Identifier</td><td/><td>External key to the Binary table.</td></row>
		<row><td>Control</td><td>Control</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the control. This name must be unique within a dialog, but can repeat on different dialogs.</td></row>
		<row><td>Control</td><td>Control_Next</td><td>Y</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>The name of an other control on the same dialog. This link defines the tab order of the controls. The links have to form one or more cycles!</td></row>
		<row><td>Control</td><td>Dialog_</td><td>N</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>External key to the Dialog table, name of the dialog.</td></row>
		<row><td>Control</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Height of the bounding rectangle of the control.</td></row>
		<row><td>Control</td><td>Help</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The help strings used with the button. The text is optional.</td></row>
		<row><td>Control</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to .rtf file for scrollable text control</td></row>
		<row><td>Control</td><td>ISControlId</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A number used to represent the control ID of the Control, Used in Dialog export</td></row>
		<row><td>Control</td><td>ISWindowStyle</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>A 32-bit word that specifies non-MSI window styles to be applied to this control.</td></row>
		<row><td>Control</td><td>Property</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The name of a defined property to be linked to this control.</td></row>
		<row><td>Control</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>A string used to set the initial text contained within a control (if appropriate).</td></row>
		<row><td>Control</td><td>Type</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The type of the control.</td></row>
		<row><td>Control</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Width of the bounding rectangle of the control.</td></row>
		<row><td>Control</td><td>X</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Horizontal coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>Control</td><td>Y</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Vertical coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>ControlCondition</td><td>Action</td><td>N</td><td/><td/><td/><td/><td/><td>Default;Disable;Enable;Hide;Show</td><td>The desired action to be taken on the specified control.</td></row>
		<row><td>ControlCondition</td><td>Condition</td><td>N</td><td/><td/><td/><td/><td>Condition</td><td/><td>A standard conditional statement that specifies under which conditions the action should be triggered.</td></row>
		<row><td>ControlCondition</td><td>Control_</td><td>N</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>A foreign key to the Control table, name of the control.</td></row>
		<row><td>ControlCondition</td><td>Dialog_</td><td>N</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>A foreign key to the Dialog table, name of the dialog.</td></row>
		<row><td>ControlEvent</td><td>Argument</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>A value to be used as a modifier when triggering a particular event.</td></row>
		<row><td>ControlEvent</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>A standard conditional statement that specifies under which conditions an event should be triggered.</td></row>
		<row><td>ControlEvent</td><td>Control_</td><td>N</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>A foreign key to the Control table, name of the control</td></row>
		<row><td>ControlEvent</td><td>Dialog_</td><td>N</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>A foreign key to the Dialog table, name of the dialog.</td></row>
		<row><td>ControlEvent</td><td>Event</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>An identifier that specifies the type of the event that should take place when the user interacts with control specified by the first two entries.</td></row>
		<row><td>ControlEvent</td><td>Ordering</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>An integer used to order several events tied to the same control. Can be left blank.</td></row>
		<row><td>CreateFolder</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table.</td></row>
		<row><td>CreateFolder</td><td>Directory_</td><td>N</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Primary key, could be foreign key into the Directory table.</td></row>
		<row><td>CustomAction</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, name of action, normally appears in sequence table unless private use.</td></row>
		<row><td>CustomAction</td><td>ExtendedType</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The numeric custom action type info flags.</td></row>
		<row><td>CustomAction</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments for this custom action.</td></row>
		<row><td>CustomAction</td><td>Source</td><td>Y</td><td/><td/><td/><td/><td>CustomSource</td><td/><td>The table reference of the source of the code.</td></row>
		<row><td>CustomAction</td><td>Target</td><td>Y</td><td/><td/><td>ISDLLWrapper;ISInstallScriptAction</td><td>1</td><td>Formatted</td><td/><td>Excecution parameter, depends on the type of custom action</td></row>
		<row><td>CustomAction</td><td>Type</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>The numeric custom action type, consisting of source location, code type, entry, option flags.</td></row>
		<row><td>Dialog</td><td>Attributes</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>A 32-bit word that specifies the attribute flags to be applied to this dialog.</td></row>
		<row><td>Dialog</td><td>Control_Cancel</td><td>Y</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>Defines the cancel control. Hitting escape or clicking on the close icon on the dialog is equivalent to pushing this button.</td></row>
		<row><td>Dialog</td><td>Control_Default</td><td>Y</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>Defines the default control. Hitting return is equivalent to pushing this button.</td></row>
		<row><td>Dialog</td><td>Control_First</td><td>N</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>Defines the control that has the focus when the dialog is created.</td></row>
		<row><td>Dialog</td><td>Dialog</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the dialog.</td></row>
		<row><td>Dialog</td><td>HCentering</td><td>N</td><td>0</td><td>100</td><td/><td/><td/><td/><td>Horizontal position of the dialog on a 0-100 scale. 0 means left end, 100 means right end of the screen, 50 center.</td></row>
		<row><td>Dialog</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Height of the bounding rectangle of the dialog.</td></row>
		<row><td>Dialog</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments for this dialog.</td></row>
		<row><td>Dialog</td><td>ISResourceId</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A Number the Specifies the Dialog ID to be used in Dialog Export</td></row>
		<row><td>Dialog</td><td>ISWindowStyle</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A 32-bit word that specifies non-MSI window styles to be applied to this control. This is only used in Script Based Setups.</td></row>
		<row><td>Dialog</td><td>TextStyle_</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign Key into TextStyle table, only used in Script Based Projects.</td></row>
		<row><td>Dialog</td><td>Title</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>A text string specifying the title to be displayed in the title bar of the dialog's window.</td></row>
		<row><td>Dialog</td><td>VCentering</td><td>N</td><td>0</td><td>100</td><td/><td/><td/><td/><td>Vertical position of the dialog on a 0-100 scale. 0 means top end, 100 means bottom end of the screen, 50 center.</td></row>
		<row><td>Dialog</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Width of the bounding rectangle of the dialog.</td></row>
		<row><td>Directory</td><td>DefaultDir</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The default sub-path under parent's path.</td></row>
		<row><td>Directory</td><td>Directory</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Unique identifier for directory entry, primary key. If a property by this name is defined, it contains the full path to the directory.</td></row>
		<row><td>Directory</td><td>Directory_Parent</td><td>Y</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Reference to the entry in this table specifying the default parent directory. A record parented to itself or with a Null parent represents a root of the install tree.</td></row>
		<row><td>Directory</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1;2;3;4;5;6;7</td><td>This is used to store Installshield custom properties of a directory.  Currently the only one is Shortcut.</td></row>
		<row><td>Directory</td><td>ISDescription</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Description of folder</td></row>
		<row><td>Directory</td><td>ISFolderName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>This is used in Pro projects because the pro identifier used in the tree wasn't necessarily unique.</td></row>
		<row><td>DrLocator</td><td>Depth</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The depth below the path to which the Signature_ is recursively searched. If absent, the depth is assumed to be 0.</td></row>
		<row><td>DrLocator</td><td>Parent</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The parent file signature. It is also a foreign key in the Signature table. If null and the Path column does not expand to a full path, then all the fixed drives of the user system are searched using the Path.</td></row>
		<row><td>DrLocator</td><td>Path</td><td>Y</td><td/><td/><td/><td/><td>AnyPath</td><td/><td>The path on the user system. This is a either a subpath below the value of the Parent or a full path. The path may contain properties enclosed within [ ] that will be expanded.</td></row>
		<row><td>DrLocator</td><td>Signature_</td><td>N</td><td/><td/><td>Signature</td><td>1</td><td>Identifier</td><td/><td>The Signature_ represents a unique file signature and is also the foreign key in the Signature table.</td></row>
		<row><td>DuplicateFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the duplicate file.</td></row>
		<row><td>DuplicateFile</td><td>DestFolder</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of a property whose value is assumed to resolve to the full pathname to a destination folder.</td></row>
		<row><td>DuplicateFile</td><td>DestName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Filename to be given to the duplicate file.</td></row>
		<row><td>DuplicateFile</td><td>FileKey</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular file entry</td></row>
		<row><td>DuplicateFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing the source file to be duplicated.</td></row>
		<row><td>Environment</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table referencing component that controls the installing of the environmental value.</td></row>
		<row><td>Environment</td><td>Environment</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Unique identifier for the environmental variable setting</td></row>
		<row><td>Environment</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the environmental value.</td></row>
		<row><td>Environment</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value to set in the environmental settings.</td></row>
		<row><td>Error</td><td>Error</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Integer error number, obtained from header file IError(...) macros.</td></row>
		<row><td>Error</td><td>Message</td><td>Y</td><td/><td/><td/><td/><td>Template</td><td/><td>Error formatting template, obtained from user ed. or localizers.</td></row>
		<row><td>EventMapping</td><td>Attribute</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The name of the control attribute, that is set when this event is received.</td></row>
		<row><td>EventMapping</td><td>Control_</td><td>N</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>A foreign key to the Control table, name of the control.</td></row>
		<row><td>EventMapping</td><td>Dialog_</td><td>N</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>A foreign key to the Dialog table, name of the Dialog.</td></row>
		<row><td>EventMapping</td><td>Event</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>An identifier that specifies the type of the event that the control subscribes to.</td></row>
		<row><td>Extension</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table, specifying the component for which to return a path when called through LocateComponent.</td></row>
		<row><td>Extension</td><td>Extension</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The extension associated with the table row.</td></row>
		<row><td>Extension</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Feature Table, specifying the feature to validate or install in order for the CLSID factory to be operational.</td></row>
		<row><td>Extension</td><td>MIME_</td><td>Y</td><td/><td/><td>MIME</td><td>1</td><td>Text</td><td/><td>Optional Context identifier, typically "type/format" associated with the extension</td></row>
		<row><td>Extension</td><td>ProgId_</td><td>Y</td><td/><td/><td>ProgId</td><td>1</td><td>Text</td><td/><td>Optional ProgId associated with this extension.</td></row>
		<row><td>Feature</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td>0;1;2;4;5;6;8;9;10;16;17;18;20;21;22;24;25;26;32;33;34;36;37;38;48;49;50;52;53;54</td><td>Feature attributes</td></row>
		<row><td>Feature</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Longer descriptive text describing a visible feature item.</td></row>
		<row><td>Feature</td><td>Directory_</td><td>Y</td><td/><td/><td>Directory</td><td>1</td><td>UpperCase</td><td/><td>The name of the Directory that can be configured by the UI. A non-null value will enable the browse button.</td></row>
		<row><td>Feature</td><td>Display</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Numeric sort order, used to force a specific display ordering.</td></row>
		<row><td>Feature</td><td>Feature</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular feature record.</td></row>
		<row><td>Feature</td><td>Feature_Parent</td><td>Y</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Optional key of a parent record in the same table. If the parent is not selected, then the record will not be installed. Null indicates a root item.</td></row>
		<row><td>Feature</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Comments</td></row>
		<row><td>Feature</td><td>ISFeatureCabName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Name of CAB used when compressing CABs by Feature. Used to override build generated name for CAB file.</td></row>
		<row><td>Feature</td><td>ISProFeatureName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the feature used by pro projects.  This doesn't have to be unique.</td></row>
		<row><td>Feature</td><td>ISReleaseFlags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Release Flags that specify whether this  feature will be built in a particular release.</td></row>
		<row><td>Feature</td><td>Level</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The install level at which record will be initially selected. An install level of 0 will disable an item and prevent its display.</td></row>
		<row><td>Feature</td><td>Title</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Short text identifying a visible feature item.</td></row>
		<row><td>FeatureComponents</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Component table.</td></row>
		<row><td>FeatureComponents</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>File</td><td>Attributes</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Integer containing bit flags representing file attributes (with the decimal value of each bit position in parentheses)</td></row>
		<row><td>File</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the file.</td></row>
		<row><td>File</td><td>File</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token, must match identifier in cabinet.  For uncompressed files, this field is ignored.</td></row>
		<row><td>File</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>File name used for installation.  This may contain a "short name|long name" pair.  It may be just a long name, hence it cannot be of the Filename data type.</td></row>
		<row><td>File</td><td>FileSize</td><td>N</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Size of file in bytes (long integer).</td></row>
		<row><td>File</td><td>ISAttributes</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>This field contains the following attributes: UseSystemSettings(0x1)</td></row>
		<row><td>File</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path, the category is of Text instead of Path because of potential use of path variables.</td></row>
		<row><td>File</td><td>ISComponentSubFolder_</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key referencing component subfolder containing this file.  Only for Pro.</td></row>
		<row><td>File</td><td>Language</td><td>Y</td><td/><td/><td/><td/><td>Language</td><td/><td>List of decimal language Ids, comma-separated if more than one.</td></row>
		<row><td>File</td><td>Sequence</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>Sequence with respect to the media images; order must track cabinet order.</td></row>
		<row><td>File</td><td>Version</td><td>Y</td><td/><td/><td>File</td><td>1</td><td>Version</td><td/><td>Version string for versioned files;  Blank for unversioned files.</td></row>
		<row><td>FileSFPCatalog</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>File associated with the catalog</td></row>
		<row><td>FileSFPCatalog</td><td>SFPCatalog_</td><td>N</td><td/><td/><td>SFPCatalog</td><td>1</td><td>Text</td><td/><td>Catalog associated with the file</td></row>
		<row><td>Font</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Primary key, foreign key into File table referencing font file.</td></row>
		<row><td>Font</td><td>FontTitle</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Font name.</td></row>
		<row><td>ISAssistantTag</td><td>Data</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISAssistantTag</td><td>Tag</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Color</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Duration</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Effect</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Font</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>ISBillboard</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Origin</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Sequence</td><td>N</td><td>-32767</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Style</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Target</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Title</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>X</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISBillBoard</td><td>Y</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td/></row>
		<row><td>ISChainPackage</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Display name for the chained package. Used only in the IDE.</td></row>
		<row><td>ISChainPackage</td><td>ISReleaseFlags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISChainPackage</td><td>InstallCondition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>InstallProperties</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>Options</td><td>N</td><td/><td/><td/><td/><td>Integer</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>Order</td><td>N</td><td/><td/><td/><td/><td>Integer</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>Package</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>ProductCode</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISChainPackage</td><td>RemoveCondition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>RemoveProperties</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td/></row>
		<row><td>ISChainPackage</td><td>SourcePath</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISChainPackageData</td><td>Data</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>Binary stream. The binary icon data in PE (.DLL or .EXE) or icon (.ICO) format.</td></row>
		<row><td>ISChainPackageData</td><td>File</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td/></row>
		<row><td>ISChainPackageData</td><td>FilePath</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td/></row>
		<row><td>ISChainPackageData</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to the ICO or EXE file.</td></row>
		<row><td>ISChainPackageData</td><td>Options</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISChainPackageData</td><td>Package_</td><td>N</td><td/><td/><td>ISChainPackage</td><td>1</td><td>Identifier</td><td/><td/></row>
		<row><td>ISClrWrap</td><td>Action_</td><td>N</td><td/><td/><td>CustomAction</td><td>1</td><td>Identifier</td><td/><td>Foreign key into CustomAction table</td></row>
		<row><td>ISClrWrap</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Property associated with this Action</td></row>
		<row><td>ISClrWrap</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value associated with this Property</td></row>
		<row><td>ISComCatalogAttribute</td><td>ISComCatalogObject_</td><td>N</td><td/><td/><td>ISComCatalogObject</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComCatalogObject table.</td></row>
		<row><td>ISComCatalogAttribute</td><td>ItemName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The named attribute for a catalog object.</td></row>
		<row><td>ISComCatalogAttribute</td><td>ItemValue</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>A value associated with the attribute defined in the ItemName column.</td></row>
		<row><td>ISComCatalogCollection</td><td>CollectionName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>A catalog collection name.</td></row>
		<row><td>ISComCatalogCollection</td><td>ISComCatalogCollection</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique key for the ISComCatalogCollection table.</td></row>
		<row><td>ISComCatalogCollection</td><td>ISComCatalogObject_</td><td>N</td><td/><td/><td>ISComCatalogObject</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComCatalogObject table.</td></row>
		<row><td>ISComCatalogCollectionObjects</td><td>ISComCatalogCollection_</td><td>N</td><td/><td/><td>ISComCatalogCollection</td><td>1</td><td>Identifier</td><td/><td>A unique key for the ISComCatalogCollection table.</td></row>
		<row><td>ISComCatalogCollectionObjects</td><td>ISComCatalogObject_</td><td>N</td><td/><td/><td>ISComCatalogObject</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComCatalogObject table.</td></row>
		<row><td>ISComCatalogObject</td><td>DisplayName</td><td>N</td><td/><td/><td/><td/><td/><td/><td>The display name of a catalog object.</td></row>
		<row><td>ISComCatalogObject</td><td>ISComCatalogObject</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique key for the ISComCatalogObject table.</td></row>
		<row><td>ISComPlusApplication</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table that a COM+ application belongs to.</td></row>
		<row><td>ISComPlusApplication</td><td>ComputerName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Computer name that a COM+ application belongs to.</td></row>
		<row><td>ISComPlusApplication</td><td>DepFiles</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>List of the dependent files.</td></row>
		<row><td>ISComPlusApplication</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>InstallShield custom attributes associated with a COM+ application.</td></row>
		<row><td>ISComPlusApplication</td><td>ISComCatalogObject_</td><td>N</td><td/><td/><td>ISComCatalogObject</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComCatalogObject table.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>AlterDLL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Alternate filename of the COM+ application component. Will be used for a .NET serviced component.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>CLSID</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>CLSID of the COM+ application component.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>DLL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Filename of the COM+ application component.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>ISComCatalogObject_</td><td>N</td><td/><td/><td>ISComCatalogObject</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComCatalogObject table.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>ISComPlusApplicationDLL</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique key for the ISComPlusApplicationDLL table.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>ISComPlusApplication_</td><td>N</td><td/><td/><td>ISComPlusApplication</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplication table.</td></row>
		<row><td>ISComPlusApplicationDLL</td><td>ProgId</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>ProgId of the COM+ application component.</td></row>
		<row><td>ISComPlusProxy</td><td>Component_</td><td>Y</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table that a COM+ application proxy belongs to.</td></row>
		<row><td>ISComPlusProxy</td><td>DepFiles</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>List of the dependent files.</td></row>
		<row><td>ISComPlusProxy</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>InstallShield custom attributes associated with a COM+ application proxy.</td></row>
		<row><td>ISComPlusProxy</td><td>ISComPlusApplication_</td><td>N</td><td/><td/><td>ISComPlusApplication</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplication table that a COM+ application proxy belongs to.</td></row>
		<row><td>ISComPlusProxy</td><td>ISComPlusProxy</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique key for the ISComPlusProxy table.</td></row>
		<row><td>ISComPlusProxyDepFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table.</td></row>
		<row><td>ISComPlusProxyDepFile</td><td>ISComPlusApplication_</td><td>N</td><td/><td/><td>ISComPlusApplication</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplication table.</td></row>
		<row><td>ISComPlusProxyDepFile</td><td>ISPath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path of the dependent file.</td></row>
		<row><td>ISComPlusProxyFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table.</td></row>
		<row><td>ISComPlusProxyFile</td><td>ISComPlusApplicationDLL_</td><td>N</td><td/><td/><td>ISComPlusApplicationDLL</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplicationDLL table.</td></row>
		<row><td>ISComPlusServerDepFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table.</td></row>
		<row><td>ISComPlusServerDepFile</td><td>ISComPlusApplication_</td><td>N</td><td/><td/><td>ISComPlusApplication</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplication table.</td></row>
		<row><td>ISComPlusServerDepFile</td><td>ISPath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path of the dependent file.</td></row>
		<row><td>ISComPlusServerFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table.</td></row>
		<row><td>ISComPlusServerFile</td><td>ISComPlusApplicationDLL_</td><td>N</td><td/><td/><td>ISComPlusApplicationDLL</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISComPlusApplicationDLL table.</td></row>
		<row><td>ISComponentExtended</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Primary key used to identify a particular component record.</td></row>
		<row><td>ISComponentExtended</td><td>FTPLocation</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>FTP Location</td></row>
		<row><td>ISComponentExtended</td><td>FilterProperty</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property to set if you want to filter a component</td></row>
		<row><td>ISComponentExtended</td><td>HTTPLocation</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>HTTP Location</td></row>
		<row><td>ISComponentExtended</td><td>Language</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Language</td></row>
		<row><td>ISComponentExtended</td><td>Miscellaneous</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Miscellaneous</td></row>
		<row><td>ISComponentExtended</td><td>OS</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>bitwise addition of OSs</td></row>
		<row><td>ISComponentExtended</td><td>Platforms</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>bitwise addition of Platforms.</td></row>
		<row><td>ISCustomActionReference</td><td>Action_</td><td>N</td><td/><td/><td>CustomAction</td><td>1</td><td>Identifier</td><td/><td>Foreign key into theICustomAction table.</td></row>
		<row><td>ISCustomActionReference</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Contents of the file speciifed in ISCAReferenceFilePath. This column is only used by MSI.</td></row>
		<row><td>ISCustomActionReference</td><td>FileType</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>file type of the file specified  ISCAReferenceFilePath. This column is only used by MSI.</td></row>
		<row><td>ISCustomActionReference</td><td>ISCAReferenceFilePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path, the category is of Text instead of Path because of potential use of path variables.  This column only exists in ISM.</td></row>
		<row><td>ISDIMDependency</td><td>ISDIMReference_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>This is the primary key to the ISDIMDependency table</td></row>
		<row><td>ISDIMDependency</td><td>RequiredBuildVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>the build version identifying the required DIM</td></row>
		<row><td>ISDIMDependency</td><td>RequiredMajorVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>the major version identifying the required DIM</td></row>
		<row><td>ISDIMDependency</td><td>RequiredMinorVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>the minor version identifying the required DIM</td></row>
		<row><td>ISDIMDependency</td><td>RequiredRevisionVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>the revision version identifying the required DIM</td></row>
		<row><td>ISDIMDependency</td><td>RequiredUUID</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>the UUID identifying the required DIM</td></row>
		<row><td>ISDIMReference</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path, the category is of Text instead of Path because of potential use of path variables.</td></row>
		<row><td>ISDIMReference</td><td>ISDIMReference</td><td>N</td><td/><td/><td>ISDIMDependency</td><td>1</td><td>Identifier</td><td/><td>This is the primary key to the ISDIMReference table</td></row>
		<row><td>ISDIMReferenceDependencies</td><td>ISDIMDependency_</td><td>N</td><td/><td/><td>ISDIMDependency</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISDIMDependency table.</td></row>
		<row><td>ISDIMReferenceDependencies</td><td>ISDIMReference_Parent</td><td>N</td><td/><td/><td>ISDIMReference</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISDIMReference table.</td></row>
		<row><td>ISDIMVariable</td><td>ISDIMReference_</td><td>N</td><td/><td/><td>ISDIMReference</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISDIMReference table.</td></row>
		<row><td>ISDIMVariable</td><td>ISDIMVariable</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>This is the primary key to the ISDIMVariable table</td></row>
		<row><td>ISDIMVariable</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of a variable defined in the .dim file</td></row>
		<row><td>ISDIMVariable</td><td>NewValue</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>New value that you want to override with</td></row>
		<row><td>ISDIMVariable</td><td>Type</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Type of the variable. 0: Build Variable, 1: Runtime Variable</td></row>
		<row><td>ISDLLWrapper</td><td>EntryPoint</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is a foreign key to the target column in the CustomAction table</td></row>
		<row><td>ISDLLWrapper</td><td>Source</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>This is column points to the source file for the DLLWrapper Custom Action</td></row>
		<row><td>ISDLLWrapper</td><td>Target</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The function signature</td></row>
		<row><td>ISDLLWrapper</td><td>Type</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Type</td></row>
		<row><td>ISDependency</td><td>Exclude</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISDependency</td><td>ISDependency</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISDisk1File</td><td>Disk</td><td>Y</td><td/><td/><td/><td/><td/><td>-1;0;1</td><td>Used to differentiate between disk1(1), last disk(-1), and other(0).</td></row>
		<row><td>ISDisk1File</td><td>ISBuildSourcePath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path of file to be copied to Disk1 folder</td></row>
		<row><td>ISDisk1File</td><td>ISDisk1File</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key for ISDisk1File table</td></row>
		<row><td>ISDynamicFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the file.</td></row>
		<row><td>ISDynamicFile</td><td>ExcludeFiles</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Wildcards for excluded files.</td></row>
		<row><td>ISDynamicFile</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1;2;3;4;5;6;7;8;9;10;11;12;13;14;15</td><td>This is used to store Installshield custom properties of a dynamic filet.  Currently the only one is SelfRegister.</td></row>
		<row><td>ISDynamicFile</td><td>IncludeFiles</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Wildcards for included files.</td></row>
		<row><td>ISDynamicFile</td><td>IncludeFlags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Include flags.</td></row>
		<row><td>ISDynamicFile</td><td>SourceFolder</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path, the category is of Text instead of Path because of potential use of path variables.</td></row>
		<row><td>ISFeatureDIMReferences</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>ISFeatureDIMReferences</td><td>ISDIMReference_</td><td>N</td><td/><td/><td>ISDIMReference</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISDIMReference table.</td></row>
		<row><td>ISFeatureMergeModuleExcludes</td><td>Feature_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>ISFeatureMergeModuleExcludes</td><td>Language</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Foreign key into ISMergeModule table.</td></row>
		<row><td>ISFeatureMergeModuleExcludes</td><td>ModuleID</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into ISMergeModule table.</td></row>
		<row><td>ISFeatureMergeModules</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>ISFeatureMergeModules</td><td>ISMergeModule_</td><td>N</td><td/><td/><td>ISMergeModule</td><td>1</td><td>Text</td><td/><td>Foreign key into ISMergeModule table.</td></row>
		<row><td>ISFeatureMergeModules</td><td>Language_</td><td>N</td><td/><td/><td>ISMergeModule</td><td>2</td><td/><td/><td>Foreign key into ISMergeModule table.</td></row>
		<row><td>ISFeatureSetupPrerequisites</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>ISFeatureSetupPrerequisites</td><td>ISSetupPrerequisites_</td><td>N</td><td/><td/><td>ISSetupPrerequisites</td><td>1</td><td/><td/><td/></row>
		<row><td>ISFileManifests</td><td>File_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into File table.</td></row>
		<row><td>ISFileManifests</td><td>Manifest_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into File table.</td></row>
		<row><td>ISIISItem</td><td>Component_</td><td>Y</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key to Component table.</td></row>
		<row><td>ISIISItem</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localizable Item Name.</td></row>
		<row><td>ISIISItem</td><td>ISIISItem</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key for each item.</td></row>
		<row><td>ISIISItem</td><td>ISIISItem_Parent</td><td>Y</td><td/><td/><td>ISIISItem</td><td>1</td><td>Identifier</td><td/><td>This record's parent record.</td></row>
		<row><td>ISIISItem</td><td>Type</td><td>N</td><td/><td/><td/><td/><td/><td/><td>IIS resource type.</td></row>
		<row><td>ISIISProperty</td><td>FriendlyName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>IIS property name.</td></row>
		<row><td>ISIISProperty</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Flags.</td></row>
		<row><td>ISIISProperty</td><td>ISIISItem_</td><td>N</td><td/><td/><td>ISIISItem</td><td>1</td><td>Identifier</td><td/><td>Primary key for table, foreign key into ISIISItem.</td></row>
		<row><td>ISIISProperty</td><td>ISIISProperty</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key for table.</td></row>
		<row><td>ISIISProperty</td><td>MetaDataAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>IIS property attributes.</td></row>
		<row><td>ISIISProperty</td><td>MetaDataProp</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>IIS property ID.</td></row>
		<row><td>ISIISProperty</td><td>MetaDataType</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>IIS property data type.</td></row>
		<row><td>ISIISProperty</td><td>MetaDataUserType</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>IIS property user data type.</td></row>
		<row><td>ISIISProperty</td><td>MetaDataValue</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>IIS property value.</td></row>
		<row><td>ISIISProperty</td><td>Order</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Order sequencing.</td></row>
		<row><td>ISIISProperty</td><td>Schema</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>IIS7 schema information.</td></row>
		<row><td>ISInstallScriptAction</td><td>EntryPoint</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is a foreign key to the target column in the CustomAction table</td></row>
		<row><td>ISInstallScriptAction</td><td>Source</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>This is column points to the source file for the DLLWrapper Custom Action</td></row>
		<row><td>ISInstallScriptAction</td><td>Target</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The function signature</td></row>
		<row><td>ISInstallScriptAction</td><td>Type</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Type</td></row>
		<row><td>ISLanguage</td><td>ISLanguage</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is the language ID.</td></row>
		<row><td>ISLanguage</td><td>Included</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1</td><td>Specify whether this language should be included.</td></row>
		<row><td>ISLinkerLibrary</td><td>ISLinkerLibrary</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Unique identifier for the link library.</td></row>
		<row><td>ISLinkerLibrary</td><td>Library</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path of the object library (.obl file).</td></row>
		<row><td>ISLinkerLibrary</td><td>Order</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Order of the Library</td></row>
		<row><td>ISLocalControl</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A 32-bit word that specifies the attribute flags to be applied to this control.</td></row>
		<row><td>ISLocalControl</td><td>Binary_</td><td>Y</td><td/><td/><td>Binary</td><td>1</td><td>Identifier</td><td/><td>External key to the Binary table.</td></row>
		<row><td>ISLocalControl</td><td>Control_</td><td>N</td><td/><td/><td>Control</td><td>2</td><td>Identifier</td><td/><td>Name of the control. This name must be unique within a dialog, but can repeat on different dialogs.</td></row>
		<row><td>ISLocalControl</td><td>Dialog_</td><td>N</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>External key to the Dialog table, name of the dialog.</td></row>
		<row><td>ISLocalControl</td><td>Height</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Height of the bounding rectangle of the control.</td></row>
		<row><td>ISLocalControl</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to .rtf file for scrollable text control</td></row>
		<row><td>ISLocalControl</td><td>ISLanguage_</td><td>N</td><td/><td/><td>ISLanguage</td><td>1</td><td>Text</td><td/><td>This is a foreign key to the ISLanguage table.</td></row>
		<row><td>ISLocalControl</td><td>Width</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Width of the bounding rectangle of the control.</td></row>
		<row><td>ISLocalControl</td><td>X</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Horizontal coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>ISLocalControl</td><td>Y</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Vertical coordinate of the upper left corner of the bounding rectangle of the control.</td></row>
		<row><td>ISLocalDialog</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A 32-bit word that specifies the attribute flags to be applied to this dialog.</td></row>
		<row><td>ISLocalDialog</td><td>Dialog_</td><td>Y</td><td/><td/><td>Dialog</td><td>1</td><td>Identifier</td><td/><td>Name of the dialog.</td></row>
		<row><td>ISLocalDialog</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Height of the bounding rectangle of the dialog.</td></row>
		<row><td>ISLocalDialog</td><td>ISLanguage_</td><td>Y</td><td/><td/><td>ISLanguage</td><td>1</td><td>Text</td><td/><td>This is a foreign key to the ISLanguage table.</td></row>
		<row><td>ISLocalDialog</td><td>TextStyle_</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign Key into TextStyle table, only used in Script Based Projects.</td></row>
		<row><td>ISLocalDialog</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Width of the bounding rectangle of the dialog.</td></row>
		<row><td>ISLocalRadioButton</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The height of the button.</td></row>
		<row><td>ISLocalRadioButton</td><td>ISLanguage_</td><td>N</td><td/><td/><td>ISLanguage</td><td>1</td><td>Text</td><td/><td>This is a foreign key to the ISLanguage table.</td></row>
		<row><td>ISLocalRadioButton</td><td>Order</td><td>N</td><td>1</td><td>32767</td><td>RadioButton</td><td>2</td><td/><td/><td>A positive integer used to determine the ordering of the items within one list..The integers do not have to be consecutive.</td></row>
		<row><td>ISLocalRadioButton</td><td>Property</td><td>N</td><td/><td/><td>RadioButton</td><td>1</td><td>Identifier</td><td/><td>A named property to be tied to this radio button. All the buttons tied to the same property become part of the same group.</td></row>
		<row><td>ISLocalRadioButton</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The width of the button.</td></row>
		<row><td>ISLocalRadioButton</td><td>X</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The horizontal coordinate of the upper left corner of the bounding rectangle of the radio button.</td></row>
		<row><td>ISLocalRadioButton</td><td>Y</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The vertical coordinate of the upper left corner of the bounding rectangle of the radio button.</td></row>
		<row><td>ISLockPermissions</td><td>Attributes</td><td>Y</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Permissions attributes mask, 1==Deny access; 2==No inherit, 4==Ignore apply failures, 8==Target object is 64-bit</td></row>
		<row><td>ISLockPermissions</td><td>Domain</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Domain name for user whose permissions are being set.</td></row>
		<row><td>ISLockPermissions</td><td>LockObject</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into CreateFolder, Registry, or File table</td></row>
		<row><td>ISLockPermissions</td><td>Permission</td><td>Y</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Permission Access mask.</td></row>
		<row><td>ISLockPermissions</td><td>Table</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td>CreateFolder;File;Registry</td><td>Reference to another table name</td></row>
		<row><td>ISLockPermissions</td><td>User</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>User for permissions to be set. This can be a property, hardcoded named, or SID string</td></row>
		<row><td>ISLogicalDisk</td><td>Cabinet</td><td>Y</td><td/><td/><td/><td/><td>Cabinet</td><td/><td>If some or all of the files stored on the media are compressed in a cabinet, the name of that cabinet.</td></row>
		<row><td>ISLogicalDisk</td><td>DiskId</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>Primary key, integer to determine sort order for table.</td></row>
		<row><td>ISLogicalDisk</td><td>DiskPrompt</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Disk name: the visible text actually printed on the disk.  This will be used to prompt the user when this disk needs to be inserted.</td></row>
		<row><td>ISLogicalDisk</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISLogicalDisk</td><td>ISRelease_</td><td>N</td><td/><td/><td>ISRelease</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISRelease table.</td></row>
		<row><td>ISLogicalDisk</td><td>LastSequence</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>File sequence number for the last file for this media.</td></row>
		<row><td>ISLogicalDisk</td><td>Source</td><td>Y</td><td/><td/><td/><td/><td>Property</td><td/><td>The property defining the location of the cabinet file.</td></row>
		<row><td>ISLogicalDisk</td><td>VolumeLabel</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The label attributed to the volume.</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>Feature_</td><td>Y</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Feature Table,</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties, like Compressed, etc.</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>ISLogicalDisk_</td><td>N</td><td>1</td><td>32767</td><td>ISLogicalDisk</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the ISLogicalDisk table.</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>ISRelease_</td><td>N</td><td/><td/><td>ISRelease</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISRelease table.</td></row>
		<row><td>ISLogicalDiskFeatures</td><td>Sequence</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>File sequence number for the file for this media.</td></row>
		<row><td>ISMergeModule</td><td>Destination</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Destination.</td></row>
		<row><td>ISMergeModule</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties of a merge module.</td></row>
		<row><td>ISMergeModule</td><td>ISMergeModule</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The GUID identifying the merge module.</td></row>
		<row><td>ISMergeModule</td><td>Language</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Default decimal language of module.</td></row>
		<row><td>ISMergeModule</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the merge module.</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Attributes (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>ContextData</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>ContextData  (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>DefaultValue</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>DefaultValue  (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Description (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>DisplayName (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Format</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Format (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>HelpKeyword</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>HelpKeyword (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>HelpLocation</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>HelpLocation (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>ISMergeModule_</td><td>N</td><td/><td/><td>ISMergeModule</td><td>1</td><td>Text</td><td/><td>The module signature, a foreign key into the ISMergeModule table</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Language_</td><td>N</td><td/><td/><td>ISMergeModule</td><td>2</td><td/><td/><td>Default decimal language of module.</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>ModuleConfiguration_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Identifier, foreign key into ModuleConfiguration table (ModuleConfiguration.Name)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Type</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Type (from configurable merge module)</td></row>
		<row><td>ISMergeModuleCfgValues</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value for this item.</td></row>
		<row><td>ISObject</td><td>Language</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>ISObject</td><td>ObjectName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>ISObjectProperty</td><td>IncludeInBuild</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Boolean, 0 for false non 0 for true</td></row>
		<row><td>ISObjectProperty</td><td>ObjectName</td><td>Y</td><td/><td/><td>ISObject</td><td>1</td><td>Text</td><td/><td/></row>
		<row><td>ISObjectProperty</td><td>Property</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>ISObjectProperty</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>ISPatchConfigImage</td><td>PatchConfiguration_</td><td>Y</td><td/><td/><td>ISPatchConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key to the ISPatchConfigurationTable</td></row>
		<row><td>ISPatchConfigImage</td><td>UpgradedImage_</td><td>N</td><td/><td/><td>ISUpgradedImage</td><td>1</td><td>Text</td><td/><td>Foreign key to the ISUpgradedImageTable</td></row>
		<row><td>ISPatchConfiguration</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>PatchConfiguration attributes</td></row>
		<row><td>ISPatchConfiguration</td><td>CanPCDiffer</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether Product Codes may differ</td></row>
		<row><td>ISPatchConfiguration</td><td>CanPVDiffer</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether the Major Product Version may differ</td></row>
		<row><td>ISPatchConfiguration</td><td>EnablePatchCache</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether to Enable Patch cacheing</td></row>
		<row><td>ISPatchConfiguration</td><td>Flags</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Patching API Flags</td></row>
		<row><td>ISPatchConfiguration</td><td>IncludeWholeFiles</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether to build a binary level patch</td></row>
		<row><td>ISPatchConfiguration</td><td>LeaveDecompressed</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether to leave intermediate files devcompressed when finished</td></row>
		<row><td>ISPatchConfiguration</td><td>MinMsiVersion</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Minimum Required MSI Version</td></row>
		<row><td>ISPatchConfiguration</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the Patch Configuration</td></row>
		<row><td>ISPatchConfiguration</td><td>OptimizeForSize</td><td>N</td><td/><td/><td/><td/><td/><td/><td>This is determine whether to Optimize for large files</td></row>
		<row><td>ISPatchConfiguration</td><td>OutputPath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Build Location</td></row>
		<row><td>ISPatchConfiguration</td><td>PatchCacheDir</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Directory to recieve the Patch Cache information</td></row>
		<row><td>ISPatchConfiguration</td><td>PatchGuid</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Unique Patch Identifier</td></row>
		<row><td>ISPatchConfiguration</td><td>PatchGuidsToReplace</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>List Of Patch Guids to unregister</td></row>
		<row><td>ISPatchConfiguration</td><td>TargetProductCodes</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>List Of target Product Codes</td></row>
		<row><td>ISPatchConfigurationProperty</td><td>ISPatchConfiguration_</td><td>Y</td><td/><td/><td>ISPatchConfiguration</td><td>1</td><td>Text</td><td/><td>Name of the Patch Configuration</td></row>
		<row><td>ISPatchConfigurationProperty</td><td>Property</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the Patch Configuration Property value</td></row>
		<row><td>ISPatchConfigurationProperty</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value of the Patch Configuration Property</td></row>
		<row><td>ISPatchExternalFile</td><td>FileKey</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Filekey</td></row>
		<row><td>ISPatchExternalFile</td><td>FilePath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Filepath</td></row>
		<row><td>ISPatchExternalFile</td><td>ISUpgradedImage_</td><td>N</td><td/><td/><td>ISUpgradedImage</td><td>1</td><td>Text</td><td/><td>Foreign key to the isupgraded image table</td></row>
		<row><td>ISPatchExternalFile</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Uniqu name to identify this record.</td></row>
		<row><td>ISPatchWholeFile</td><td>Component</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Component containing file key</td></row>
		<row><td>ISPatchWholeFile</td><td>FileKey</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Key of file to be included as whole</td></row>
		<row><td>ISPatchWholeFile</td><td>UpgradedImage</td><td>N</td><td/><td/><td>ISUpgradedImage</td><td>1</td><td>Text</td><td/><td>Foreign key to ISUpgradedImage Table</td></row>
		<row><td>ISPathVariable</td><td>ISPathVariable</td><td>N</td><td/><td/><td/><td/><td/><td/><td>The name of the path variable.</td></row>
		<row><td>ISPathVariable</td><td>TestValue</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The test value of the path variable.</td></row>
		<row><td>ISPathVariable</td><td>Type</td><td>N</td><td/><td/><td/><td/><td/><td>1;2;4;8</td><td>The type of the path variable.</td></row>
		<row><td>ISPathVariable</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The value of the path variable.</td></row>
		<row><td>ISProductConfiguration</td><td>GeneratePackageCode</td><td>Y</td><td/><td/><td/><td/><td>Number</td><td>0;1</td><td>Indicates whether or not to generate a package code.</td></row>
		<row><td>ISProductConfiguration</td><td>ISProductConfiguration</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the product configuration.</td></row>
		<row><td>ISProductConfiguration</td><td>ProductConfigurationFlags</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Product configuration (release) flags.</td></row>
		<row><td>ISProductConfigurationInstance</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISProductConfigurationInstance</td><td>InstanceId</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Identifies the instance number of this instance. This value is stored in the Property InstanceId.</td></row>
		<row><td>ISProductConfigurationInstance</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Product Congiuration property name</td></row>
		<row><td>ISProductConfigurationInstance</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>String value for property.</td></row>
		<row><td>ISProductConfigurationProperty</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISProductConfigurationProperty</td><td>Property</td><td>N</td><td/><td/><td>Property</td><td>1</td><td>Text</td><td/><td>Product Congiuration property name</td></row>
		<row><td>ISProductConfigurationProperty</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>String value for property. Never null or empty.</td></row>
		<row><td>ISRelease</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Bitfield holding boolean values for various release attributes.</td></row>
		<row><td>ISRelease</td><td>BuildLocation</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Build location.</td></row>
		<row><td>ISRelease</td><td>CDBrowser</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Demoshield browser location.</td></row>
		<row><td>ISRelease</td><td>DefaultLanguage</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Default language for setup.</td></row>
		<row><td>ISRelease</td><td>DigitalPVK</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Digital signing private key (.pvk) file.</td></row>
		<row><td>ISRelease</td><td>DigitalSPC</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Digital signing Software Publisher Certificate (.spc) file.</td></row>
		<row><td>ISRelease</td><td>DigitalURL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Digital signing URL.</td></row>
		<row><td>ISRelease</td><td>DiskClusterSize</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Disk cluster size.</td></row>
		<row><td>ISRelease</td><td>DiskSize</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Disk size.</td></row>
		<row><td>ISRelease</td><td>DiskSizeUnit</td><td>N</td><td/><td/><td/><td/><td/><td>0;1;2</td><td>Disk size units (KB or MB).</td></row>
		<row><td>ISRelease</td><td>DiskSpanning</td><td>N</td><td/><td/><td/><td/><td/><td>0;1;2</td><td>Disk spanning (automatic, enforce size, etc.).</td></row>
		<row><td>ISRelease</td><td>DotNetBuildConfiguration</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Build Configuration for .NET solutions.</td></row>
		<row><td>ISRelease</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISRelease</td><td>ISRelease</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the release.</td></row>
		<row><td>ISRelease</td><td>ISSetupPrerequisiteLocation</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1;2;3</td><td>Location the Setup Prerequisites will be placed in</td></row>
		<row><td>ISRelease</td><td>MediaLocation</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Media location on disk.</td></row>
		<row><td>ISRelease</td><td>MsiCommandLine</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Command line passed to the msi package from setup.exe</td></row>
		<row><td>ISRelease</td><td>MsiSourceType</td><td>N</td><td>-1</td><td>4</td><td/><td/><td/><td/><td>MSI media source type.</td></row>
		<row><td>ISRelease</td><td>PackageName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Package name.</td></row>
		<row><td>ISRelease</td><td>Password</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Password.</td></row>
		<row><td>ISRelease</td><td>Platforms</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Platforms supported (Intel, Alpha, etc.).</td></row>
		<row><td>ISRelease</td><td>ReleaseFlags</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Release flags.</td></row>
		<row><td>ISRelease</td><td>ReleaseType</td><td>N</td><td/><td/><td/><td/><td/><td>1;2;4</td><td>Release type (single, uncompressed, etc.).</td></row>
		<row><td>ISRelease</td><td>SupportedLanguagesData</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Languages supported (for component filtering).</td></row>
		<row><td>ISRelease</td><td>SupportedLanguagesUI</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>UI languages supported.</td></row>
		<row><td>ISRelease</td><td>SupportedOSs</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Indicate which operating systmes are supported.</td></row>
		<row><td>ISRelease</td><td>SynchMsi</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>MSI file to synchronize file keys and other data with (patch-like functionality).</td></row>
		<row><td>ISRelease</td><td>Type</td><td>N</td><td>0</td><td>6</td><td/><td/><td/><td/><td>Release type (CDROM, Network, etc.).</td></row>
		<row><td>ISRelease</td><td>URLLocation</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Media location via URL.</td></row>
		<row><td>ISRelease</td><td>VersionCopyright</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Version stamp information.</td></row>
		<row><td>ISReleaseASPublishInfo</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISReleaseASPublishInfo</td><td>ISRelease_</td><td>N</td><td/><td/><td>ISRelease</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISRelease table.</td></row>
		<row><td>ISReleaseASPublishInfo</td><td>Property</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>AS Repository property name</td></row>
		<row><td>ISReleaseASPublishInfo</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>AS Repository property value</td></row>
		<row><td>ISReleaseExtended</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Bitfield holding boolean values for various release attributes.</td></row>
		<row><td>ISReleaseExtended</td><td>CertPassword</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Digital certificate password</td></row>
		<row><td>ISReleaseExtended</td><td>DigitalCertificateDBaseNS</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Path to cerificate database for Netscape digital  signature</td></row>
		<row><td>ISReleaseExtended</td><td>DigitalCertificateIdNS</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Path to cerificate ID for Netscape digital  signature</td></row>
		<row><td>ISReleaseExtended</td><td>DigitalCertificatePasswordNS</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Password for Netscape digital  signature</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetBaseLanguage</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Base Languge of .NET Redist</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetFxCmdLine</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Command Line to pass to DotNetFx.exe</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetLangPackCmdLine</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Command Line to pass to LangPack.exe</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetLangaugePacks</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>.NET Redist language packs to include</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetRedistLocation</td><td>Y</td><td>0</td><td>3</td><td/><td/><td/><td/><td>Location of .NET framework Redist (Web, SetupExe, Source, None)</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetRedistURL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to .NET framework Redist</td></row>
		<row><td>ISReleaseExtended</td><td>DotNetVersion</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>Version of .NET framework Redist (1.0, 1.1)</td></row>
		<row><td>ISReleaseExtended</td><td>EngineLocation</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>Location of msi engine (Web, SetupExe...)</td></row>
		<row><td>ISReleaseExtended</td><td>ISEngineLocation</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>Location of ISScript  engine (Web, SetupExe...)</td></row>
		<row><td>ISReleaseExtended</td><td>ISEngineURL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to InstallShield scripting engine</td></row>
		<row><td>ISReleaseExtended</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISReleaseExtended</td><td>ISRelease_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the release.</td></row>
		<row><td>ISReleaseExtended</td><td>JSharpCmdLine</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Command Line to pass to vjredist.exe</td></row>
		<row><td>ISReleaseExtended</td><td>JSharpRedistLocation</td><td>Y</td><td>0</td><td>3</td><td/><td/><td/><td/><td>Location of J# framework Redist (Web, SetupExe, Source, None)</td></row>
		<row><td>ISReleaseExtended</td><td>MsiEngineVersion</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Bitfield holding selected MSI engine versions included in this release</td></row>
		<row><td>ISReleaseExtended</td><td>OneClickCabName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>File name of generated cabfile</td></row>
		<row><td>ISReleaseExtended</td><td>OneClickHtmlName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>File name of generated html page</td></row>
		<row><td>ISReleaseExtended</td><td>OneClickTargetBrowser</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>Target browser (IE, Netscape, both...)</td></row>
		<row><td>ISReleaseExtended</td><td>WebCabSize</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Size of the cabfile</td></row>
		<row><td>ISReleaseExtended</td><td>WebLocalCachePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Directory to cache downloaded package</td></row>
		<row><td>ISReleaseExtended</td><td>WebType</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>Type of web install (One Executable, Downloader...)</td></row>
		<row><td>ISReleaseExtended</td><td>WebURL</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to .msi package</td></row>
		<row><td>ISReleaseExtended</td><td>Win9xMsiUrl</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to Ansi MSI engine</td></row>
		<row><td>ISReleaseExtended</td><td>WinMsi30Url</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to MSI 3.0 engine</td></row>
		<row><td>ISReleaseExtended</td><td>WinNTMsiUrl</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>URL to Unicode MSI engine</td></row>
		<row><td>ISReleaseProperty</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key into ISProductConfiguration table.</td></row>
		<row><td>ISReleaseProperty</td><td>ISRelease_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key into ISRelease table.</td></row>
		<row><td>ISReleaseProperty</td><td>Name</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property name</td></row>
		<row><td>ISReleaseProperty</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISReleasePublishInfo</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Repository item description</td></row>
		<row><td>ISReleasePublishInfo</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Repository item display name</td></row>
		<row><td>ISReleasePublishInfo</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Bitfield holding various attributes</td></row>
		<row><td>ISReleasePublishInfo</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td>ISProductConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key into the ISProductConfiguration table.</td></row>
		<row><td>ISReleasePublishInfo</td><td>ISRelease_</td><td>N</td><td/><td/><td>ISRelease</td><td>1</td><td>Text</td><td/><td>The name of the release.</td></row>
		<row><td>ISReleasePublishInfo</td><td>Publisher</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Repository item publisher</td></row>
		<row><td>ISReleasePublishInfo</td><td>Repository</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Repository which to  publish the built merge module</td></row>
		<row><td>ISSQLConnection</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>Authentication</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>BatchSeparator</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>CmdTimeout</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>Comments</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>Database</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>ISSQLConnection</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular ISSQLConnection record.</td></row>
		<row><td>ISSQLConnection</td><td>Order</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>Password</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>ScriptVersion_Column</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>ScriptVersion_Table</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>Server</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnection</td><td>UserName</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnectionDBServer</td><td>ISSQLConnectionDBServer</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular ISSQLConnectionDBServer record.</td></row>
		<row><td>ISSQLConnectionDBServer</td><td>ISSQLConnection_</td><td>N</td><td/><td/><td>ISSQLConnection</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLConnection table.</td></row>
		<row><td>ISSQLConnectionDBServer</td><td>ISSQLDBMetaData_</td><td>N</td><td/><td/><td>ISSQLDBMetaData</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLDBMetaData table.</td></row>
		<row><td>ISSQLConnectionDBServer</td><td>Order</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLConnectionScript</td><td>ISSQLConnection_</td><td>N</td><td/><td/><td>ISSQLConnection</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLConnection table.</td></row>
		<row><td>ISSQLConnectionScript</td><td>ISSQLScriptFile_</td><td>N</td><td/><td/><td>ISSQLScriptFile</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLScriptFile table.</td></row>
		<row><td>ISSQLConnectionScript</td><td>Order</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnAdditional</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnDatabase</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnDriver</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnNetLibrary</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnPassword</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnPort</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnServer</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnUserID</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoCxnWindowsSecurity</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>AdoDriverName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>CreateDbCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>CreateTableCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>DsnODBCName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>ISSQLDBMetaData</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular ISSQLDBMetaData record.</td></row>
		<row><td>ISSQLDBMetaData</td><td>InsertRecordCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>LocalInstanceNames</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>QueryDatabasesCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>ScriptVersion_Column</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>ScriptVersion_ColumnType</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>ScriptVersion_Table</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>SelectTableCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>SwitchDbCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>TestDatabaseCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>TestTableCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>TestTableCmd2</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>VersionBeginToken</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>VersionEndToken</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>VersionInfoCmd</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLDBMetaData</td><td>WinAuthentUserId</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLRequirement</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLRequirement</td><td>ISSQLConnectionDBServer_</td><td>Y</td><td/><td/><td>ISSQLConnectionDBServer</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLConnectionDBServer table.</td></row>
		<row><td>ISSQLRequirement</td><td>ISSQLConnection_</td><td>N</td><td/><td/><td>ISSQLConnection</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLConnection table.</td></row>
		<row><td>ISSQLRequirement</td><td>ISSQLRequirement</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular ISSQLRequirement record.</td></row>
		<row><td>ISSQLRequirement</td><td>MajorVersion</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLRequirement</td><td>ServicePackLevel</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptError</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptError</td><td>ErrHandling</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptError</td><td>ErrNumber</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptError</td><td>ISSQLScriptFile_</td><td>Y</td><td/><td/><td>ISSQLScriptFile</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLScriptFile table</td></row>
		<row><td>ISSQLScriptError</td><td>Message</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Custom end-user message. Reserved for future use.</td></row>
		<row><td>ISSQLScriptFile</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptFile</td><td>Comments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Comments</td></row>
		<row><td>ISSQLScriptFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the SQL script.</td></row>
		<row><td>ISSQLScriptFile</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>A conditional statement that will disable this script if the specified condition evaluates to the 'False' state. If a script is disabled, it will not be installed regardless of the 'Action' state associated with the component.</td></row>
		<row><td>ISSQLScriptFile</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Display name for the SQL script file.</td></row>
		<row><td>ISSQLScriptFile</td><td>ErrorHandling</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptFile</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path, the category is of Text instead of Path because of potential use of path variables.</td></row>
		<row><td>ISSQLScriptFile</td><td>ISSQLScriptFile</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>This is the primary key to the ISSQLScriptFile table</td></row>
		<row><td>ISSQLScriptFile</td><td>InstallText</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Feedback end-user text at install</td></row>
		<row><td>ISSQLScriptFile</td><td>Scheduling</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptFile</td><td>UninstallText</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Feedback end-user text at Uninstall</td></row>
		<row><td>ISSQLScriptFile</td><td>Version</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Schema Version (#####.#####.#####.#####)</td></row>
		<row><td>ISSQLScriptImport</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>Authentication</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>Database</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>ExcludeTables</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>ISSQLScriptFile_</td><td>N</td><td/><td/><td>ISSQLScriptFile</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLScriptFile table.</td></row>
		<row><td>ISSQLScriptImport</td><td>IncludeTables</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>Password</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>Server</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptImport</td><td>UserName</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptReplace</td><td>Attributes</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptReplace</td><td>ISSQLScriptFile_</td><td>N</td><td/><td/><td>ISSQLScriptFile</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSQLScriptFile table.</td></row>
		<row><td>ISSQLScriptReplace</td><td>ISSQLScriptReplace</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular ISSQLScriptReplace record.</td></row>
		<row><td>ISSQLScriptReplace</td><td>Replace</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSQLScriptReplace</td><td>Search</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISScriptFile</td><td>ISScriptFile</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is the full path of the script file. The path portion may be expressed in path variable form.</td></row>
		<row><td>ISSelfReg</td><td>CmdLine</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSelfReg</td><td>Cost</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSelfReg</td><td>FileKey</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key to the file table</td></row>
		<row><td>ISSelfReg</td><td>Order</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSetupFile</td><td>FileName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>This is the file name to use when streaming the file to the support files location</td></row>
		<row><td>ISSetupFile</td><td>ISSetupFile</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>This is the primary key to the ISSetupFile table</td></row>
		<row><td>ISSetupFile</td><td>Language</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Four digit language identifier.  0 for Language Neutral</td></row>
		<row><td>ISSetupFile</td><td>Path</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Link to the source file on the build machine</td></row>
		<row><td>ISSetupFile</td><td>Splash</td><td>Y</td><td/><td/><td/><td/><td>Short</td><td/><td>Boolean value indication whether his setup file entry belongs in the Splasc Screen section</td></row>
		<row><td>ISSetupFile</td><td>Stream</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>Binary stream. The bits to stream to the support location</td></row>
		<row><td>ISSetupPrerequisites</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSetupPrerequisites</td><td>ISReleaseFlags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Release Flags that specify whether this prereq  will be included in a particular release.</td></row>
		<row><td>ISSetupPrerequisites</td><td>ISSetupLocation</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1;2</td><td/></row>
		<row><td>ISSetupPrerequisites</td><td>ISSetupPrerequisites</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSetupPrerequisites</td><td>Order</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISSetupType</td><td>Comments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>User Comments.</td></row>
		<row><td>ISSetupType</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Longer descriptive text describing a visible feature item.</td></row>
		<row><td>ISSetupType</td><td>Display</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Numeric sort order, used to force a specific display ordering.</td></row>
		<row><td>ISSetupType</td><td>Display_Name</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>A string used to set the initial text contained within a control (if appropriate).</td></row>
		<row><td>ISSetupType</td><td>ISSetupType</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular feature record.</td></row>
		<row><td>ISSetupTypeFeatures</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>ISSetupTypeFeatures</td><td>ISSetupType_</td><td>N</td><td/><td/><td>ISSetupType</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISSetupType table.</td></row>
		<row><td>ISStorages</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Path to the file to stream into sub-storage</td></row>
		<row><td>ISStorages</td><td>Name</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Name of the sub-storage key</td></row>
		<row><td>ISString</td><td>Comment</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Comment</td></row>
		<row><td>ISString</td><td>Encoded</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Encoding for multi-byte strings.</td></row>
		<row><td>ISString</td><td>ISLanguage_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is a foreign key to the ISLanguage table.</td></row>
		<row><td>ISString</td><td>ISString</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>String id.</td></row>
		<row><td>ISString</td><td>TimeStamp</td><td>Y</td><td/><td/><td/><td/><td>Time/Date</td><td/><td>Time Stamp. MSI's Time/Date column type is just an int, with bits packed in a certain order.</td></row>
		<row><td>ISString</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>real string value.</td></row>
		<row><td>ISSwidtagProperty</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISSwidtagProperty</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Property value</td></row>
		<row><td>ISTargetImage</td><td>Flags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>relative order of the target image</td></row>
		<row><td>ISTargetImage</td><td>IgnoreMissingFiles</td><td>N</td><td/><td/><td/><td/><td/><td/><td>If true, ignore missing source files when creating patch</td></row>
		<row><td>ISTargetImage</td><td>MsiPath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Path to the target image</td></row>
		<row><td>ISTargetImage</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the TargetImage</td></row>
		<row><td>ISTargetImage</td><td>Order</td><td>N</td><td/><td/><td/><td/><td/><td/><td>relative order of the target image</td></row>
		<row><td>ISTargetImage</td><td>UpgradedImage_</td><td>N</td><td/><td/><td>ISUpgradedImage</td><td>1</td><td>Text</td><td/><td>foreign key to the upgraded Image table</td></row>
		<row><td>ISUpgradeMsiItem</td><td>ISAttributes</td><td>N</td><td/><td/><td/><td/><td/><td>0;1</td><td/></row>
		<row><td>ISUpgradeMsiItem</td><td>ISReleaseFlags</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>ISUpgradeMsiItem</td><td>ObjectSetupPath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The path to the setup you want to upgrade.</td></row>
		<row><td>ISUpgradeMsiItem</td><td>UpgradeItem</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the Upgrade Item.</td></row>
		<row><td>ISUpgradedImage</td><td>Family</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the image family</td></row>
		<row><td>ISUpgradedImage</td><td>MsiPath</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Path to the upgraded image</td></row>
		<row><td>ISUpgradedImage</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the UpgradedImage</td></row>
		<row><td>ISVirtualDirectory</td><td>Directory_</td><td>N</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Directory table.</td></row>
		<row><td>ISVirtualDirectory</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISVirtualDirectory</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISVirtualFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into File  table.</td></row>
		<row><td>ISVirtualFile</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISVirtualFile</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISVirtualPackage</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISVirtualPackage</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISVirtualRegistry</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISVirtualRegistry</td><td>Registry_</td><td>N</td><td/><td/><td>Registry</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Registry table.</td></row>
		<row><td>ISVirtualRegistry</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISVirtualRelease</td><td>ISProductConfiguration_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key into ISProductConfiguration table.</td></row>
		<row><td>ISVirtualRelease</td><td>ISRelease_</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key into ISRelease table.</td></row>
		<row><td>ISVirtualRelease</td><td>Name</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property name</td></row>
		<row><td>ISVirtualRelease</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISVirtualShortcut</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Property name</td></row>
		<row><td>ISVirtualShortcut</td><td>Shortcut_</td><td>N</td><td/><td/><td>Shortcut</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Shortcut table.</td></row>
		<row><td>ISVirtualShortcut</td><td>Value</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Property value</td></row>
		<row><td>ISWSEWrap</td><td>Action_</td><td>N</td><td/><td/><td>CustomAction</td><td>1</td><td>Identifier</td><td/><td>Foreign key into CustomAction table</td></row>
		<row><td>ISWSEWrap</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Property associated with this Action</td></row>
		<row><td>ISWSEWrap</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value associated with this Property</td></row>
		<row><td>ISXmlElement</td><td>Content</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Element contents</td></row>
		<row><td>ISXmlElement</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td>Number</td><td/><td>Internal XML element attributes</td></row>
		<row><td>ISXmlElement</td><td>ISXmlElement</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized, internal token for Xml element</td></row>
		<row><td>ISXmlElement</td><td>ISXmlElement_Parent</td><td>Y</td><td/><td/><td>ISXmlElement</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISXMLElement table.</td></row>
		<row><td>ISXmlElement</td><td>ISXmlFile_</td><td>N</td><td/><td/><td>ISXmlFile</td><td>1</td><td>Identifier</td><td/><td>Foreign key into XmlFile table.</td></row>
		<row><td>ISXmlElement</td><td>XPath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>XPath fragment including any operators</td></row>
		<row><td>ISXmlElementAttrib</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td>Number</td><td/><td>Internal XML elementattib attributes</td></row>
		<row><td>ISXmlElementAttrib</td><td>ISXmlElementAttrib</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized, internal token for Xml element attribute</td></row>
		<row><td>ISXmlElementAttrib</td><td>ISXmlElement_</td><td>N</td><td/><td/><td>ISXmlElement</td><td>1</td><td>Identifier</td><td/><td>Foreign key into ISXMLElement table.</td></row>
		<row><td>ISXmlElementAttrib</td><td>Name</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized attribute name</td></row>
		<row><td>ISXmlElementAttrib</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized attribute value</td></row>
		<row><td>ISXmlFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Component table.</td></row>
		<row><td>ISXmlFile</td><td>Directory</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into Directory table.</td></row>
		<row><td>ISXmlFile</td><td>Encoding</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>XML File Encoding</td></row>
		<row><td>ISXmlFile</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized XML file name</td></row>
		<row><td>ISXmlFile</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td>Number</td><td/><td>Internal XML file attributes</td></row>
		<row><td>ISXmlFile</td><td>ISXmlFile</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized,internal token for Xml file</td></row>
		<row><td>ISXmlFile</td><td>SelectionNamespaces</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Selection namespaces</td></row>
		<row><td>ISXmlLocator</td><td>Attribute</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>The name of an attribute within the XML element.</td></row>
		<row><td>ISXmlLocator</td><td>Element</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>XPath query that will locate an element in an XML file.</td></row>
		<row><td>ISXmlLocator</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td>0;1;2</td><td/></row>
		<row><td>ISXmlLocator</td><td>Parent</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The parent file signature. It is also a foreign key in the Signature table.</td></row>
		<row><td>ISXmlLocator</td><td>Signature_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The Signature_ represents a unique file signature and is also the foreign key in the Signature,  RegLocator, IniLocator, ISXmlLocator, CompLocator and the DrLocator tables.</td></row>
		<row><td>Icon</td><td>Data</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>Binary stream. The binary icon data in PE (.DLL or .EXE) or icon (.ICO) format.</td></row>
		<row><td>Icon</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to the ICO or EXE file.</td></row>
		<row><td>Icon</td><td>ISIconIndex</td><td>Y</td><td>-32767</td><td>32767</td><td/><td/><td/><td/><td>Optional icon index to be extracted.</td></row>
		<row><td>Icon</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key. Name of the icon file.</td></row>
		<row><td>IniFile</td><td>Action</td><td>N</td><td/><td/><td/><td/><td/><td>0;1;3</td><td>The type of modification to be made, one of iifEnum</td></row>
		<row><td>IniFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table referencing component that controls the installing of the .INI value.</td></row>
		<row><td>IniFile</td><td>DirProperty</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into the Directory table denoting the directory where the .INI file is.</td></row>
		<row><td>IniFile</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The .INI file name in which to write the information</td></row>
		<row><td>IniFile</td><td>IniFile</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>IniFile</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The .INI file key below Section.</td></row>
		<row><td>IniFile</td><td>Section</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The .INI file Section.</td></row>
		<row><td>IniFile</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value to be written.</td></row>
		<row><td>IniLocator</td><td>Field</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The field in the .INI line. If Field is null or 0 the entire line is read.</td></row>
		<row><td>IniLocator</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The .INI file name.</td></row>
		<row><td>IniLocator</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Key value (followed by an equals sign in INI file).</td></row>
		<row><td>IniLocator</td><td>Section</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Section name within in file (within square brackets in INI file).</td></row>
		<row><td>IniLocator</td><td>Signature_</td><td>N</td><td/><td/><td>Signature</td><td>1</td><td>Identifier</td><td/><td>The table key. The Signature_ represents a unique file signature and is also the foreign key in the Signature table.</td></row>
		<row><td>IniLocator</td><td>Type</td><td>Y</td><td>0</td><td>2</td><td/><td/><td/><td/><td>An integer value that determines if the .INI value read is a filename or a directory location or to be used as is w/o interpretation.</td></row>
		<row><td>InstallExecuteSequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>InstallExecuteSequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>InstallExecuteSequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>InstallExecuteSequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>InstallExecuteSequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>InstallShield</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of property, uppercase if settable by launcher or loader.</td></row>
		<row><td>InstallShield</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>String value for property.</td></row>
		<row><td>InstallUISequence</td><td>Action</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of action to invoke, either in the engine or the handler DLL.</td></row>
		<row><td>InstallUISequence</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td>Optional expression which skips the action if evaluates to expFalse.If the expression syntax is invalid, the engine will terminate, returning iesBadActionData.</td></row>
		<row><td>InstallUISequence</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store MM Custom Action Types</td></row>
		<row><td>InstallUISequence</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Sequence.</td></row>
		<row><td>InstallUISequence</td><td>Sequence</td><td>Y</td><td>-4</td><td>32767</td><td/><td/><td/><td/><td>Number that determines the sort order in which the actions are to be executed.  Leave blank to suppress action.</td></row>
		<row><td>IsolatedComponent</td><td>Component_Application</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Key to Component table item for application</td></row>
		<row><td>IsolatedComponent</td><td>Component_Shared</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Key to Component table item to be isolated</td></row>
		<row><td>LaunchCondition</td><td>Condition</td><td>N</td><td/><td/><td/><td/><td>Condition</td><td/><td>Expression which must evaluate to TRUE in order for install to commence.</td></row>
		<row><td>LaunchCondition</td><td>Description</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Localizable text to display when condition fails and install must abort.</td></row>
		<row><td>ListBox</td><td>Order</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>A positive integer used to determine the ordering of the items within one list..The integers do not have to be consecutive.</td></row>
		<row><td>ListBox</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A named property to be tied to this item. All the items tied to the same property become part of the same listbox.</td></row>
		<row><td>ListBox</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The visible text to be assigned to the item. Optional. If this entry or the entire column is missing, the text is the same as the value.</td></row>
		<row><td>ListBox</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value string associated with this item. Selecting the line will set the associated property to this value.</td></row>
		<row><td>ListView</td><td>Binary_</td><td>Y</td><td/><td/><td>Binary</td><td>1</td><td>Identifier</td><td/><td>The name of the icon to be displayed with the icon. The binary information is looked up from the Binary Table.</td></row>
		<row><td>ListView</td><td>Order</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>A positive integer used to determine the ordering of the items within one list..The integers do not have to be consecutive.</td></row>
		<row><td>ListView</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A named property to be tied to this item. All the items tied to the same property become part of the same listview.</td></row>
		<row><td>ListView</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The visible text to be assigned to the item. Optional. If this entry or the entire column is missing, the text is the same as the value.</td></row>
		<row><td>ListView</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The value string associated with this item. Selecting the line will set the associated property to this value.</td></row>
		<row><td>LockPermissions</td><td>Domain</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Domain name for user whose permissions are being set. (usually a property)</td></row>
		<row><td>LockPermissions</td><td>LockObject</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into Registry or File table</td></row>
		<row><td>LockPermissions</td><td>Permission</td><td>Y</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Permission Access mask.  Full Control = 268435456 (GENERIC_ALL = 0x10000000)</td></row>
		<row><td>LockPermissions</td><td>Table</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td>Directory;File;Registry</td><td>Reference to another table name</td></row>
		<row><td>LockPermissions</td><td>User</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>User for permissions to be set.  (usually a property)</td></row>
		<row><td>MIME</td><td>CLSID</td><td>Y</td><td/><td/><td>Class</td><td>1</td><td>Guid</td><td/><td>Optional associated CLSID.</td></row>
		<row><td>MIME</td><td>ContentType</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Primary key. Context identifier, typically "type/format".</td></row>
		<row><td>MIME</td><td>Extension_</td><td>N</td><td/><td/><td>Extension</td><td>1</td><td>Text</td><td/><td>Optional associated extension (without dot)</td></row>
		<row><td>Media</td><td>Cabinet</td><td>Y</td><td/><td/><td/><td/><td>Cabinet</td><td/><td>If some or all of the files stored on the media are compressed in a cabinet, the name of that cabinet.</td></row>
		<row><td>Media</td><td>DiskId</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>Primary key, integer to determine sort order for table.</td></row>
		<row><td>Media</td><td>DiskPrompt</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Disk name: the visible text actually printed on the disk.  This will be used to prompt the user when this disk needs to be inserted.</td></row>
		<row><td>Media</td><td>LastSequence</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>File sequence number for the last file for this media.</td></row>
		<row><td>Media</td><td>Source</td><td>Y</td><td/><td/><td/><td/><td>Property</td><td/><td>The property defining the location of the cabinet file.</td></row>
		<row><td>Media</td><td>VolumeLabel</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The label attributed to the volume.</td></row>
		<row><td>MoveFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>If this component is not "selected" for installation or removal, no action will be taken on the associated MoveFile entry</td></row>
		<row><td>MoveFile</td><td>DestFolder</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of a property whose value is assumed to resolve to the full path to the destination directory</td></row>
		<row><td>MoveFile</td><td>DestName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Name to be given to the original file after it is moved or copied.  If blank, the destination file will be given the same name as the source file</td></row>
		<row><td>MoveFile</td><td>FileKey</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key that uniquely identifies a particular MoveFile record</td></row>
		<row><td>MoveFile</td><td>Options</td><td>N</td><td>0</td><td>1</td><td/><td/><td/><td/><td>Integer value specifying the MoveFile operating mode, one of imfoEnum</td></row>
		<row><td>MoveFile</td><td>SourceFolder</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of a property whose value is assumed to resolve to the full path to the source directory</td></row>
		<row><td>MoveFile</td><td>SourceName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the source file(s) to be moved or copied.  Can contain the '*' or '?' wildcards.</td></row>
		<row><td>MsiAssembly</td><td>Attributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Assembly attributes</td></row>
		<row><td>MsiAssembly</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Component table.</td></row>
		<row><td>MsiAssembly</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Feature table.</td></row>
		<row><td>MsiAssembly</td><td>File_Application</td><td>Y</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into File table, denoting the application context for private assemblies. Null for global assemblies.</td></row>
		<row><td>MsiAssembly</td><td>File_Manifest</td><td>Y</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table denoting the manifest file for the assembly.</td></row>
		<row><td>MsiAssemblyName</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into Component table.</td></row>
		<row><td>MsiAssemblyName</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name part of the name-value pairs for the assembly name.</td></row>
		<row><td>MsiAssemblyName</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The value part of the name-value pairs for the assembly name.</td></row>
		<row><td>MsiDigitalCertificate</td><td>CertData</td><td>N</td><td/><td/><td/><td/><td>Binary</td><td/><td>A certificate context blob for a signer certificate</td></row>
		<row><td>MsiDigitalCertificate</td><td>DigitalCertificate</td><td>N</td><td/><td/><td>MsiPackageCertificate</td><td>2</td><td>Identifier</td><td/><td>A unique identifier for the row</td></row>
		<row><td>MsiDigitalSignature</td><td>DigitalCertificate_</td><td>N</td><td/><td/><td>MsiDigitalCertificate</td><td>1</td><td>Identifier</td><td/><td>Foreign key to MsiDigitalCertificate table identifying the signer certificate</td></row>
		<row><td>MsiDigitalSignature</td><td>Hash</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>The encoded hash blob from the digital signature</td></row>
		<row><td>MsiDigitalSignature</td><td>SignObject</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Foreign key to Media table</td></row>
		<row><td>MsiDigitalSignature</td><td>Table</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Reference to another table name (only Media table is supported)</td></row>
		<row><td>MsiDriverPackages</td><td>Component</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Primary key used to identify a particular component record.</td></row>
		<row><td>MsiDriverPackages</td><td>Flags</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Driver package flags</td></row>
		<row><td>MsiDriverPackages</td><td>ReferenceComponents</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>MsiDriverPackages</td><td>Sequence</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>Installation sequence number</td></row>
		<row><td>MsiEmbeddedChainer</td><td>CommandLine</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td/></row>
		<row><td>MsiEmbeddedChainer</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Condition</td><td/><td/></row>
		<row><td>MsiEmbeddedChainer</td><td>MsiEmbeddedChainer</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td/></row>
		<row><td>MsiEmbeddedChainer</td><td>Source</td><td>N</td><td/><td/><td/><td/><td>CustomSource</td><td/><td/></row>
		<row><td>MsiEmbeddedChainer</td><td>Type</td><td>Y</td><td/><td/><td/><td/><td>Integer</td><td>2;18;50</td><td/></row>
		<row><td>MsiEmbeddedUI</td><td>Attributes</td><td>N</td><td>0</td><td>3</td><td/><td/><td>Integer</td><td/><td>Information about the data in the Data column.</td></row>
		<row><td>MsiEmbeddedUI</td><td>Data</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>This column contains binary information.</td></row>
		<row><td>MsiEmbeddedUI</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Filename</td><td/><td>The name of the file that receives the binary information in the Data column.</td></row>
		<row><td>MsiEmbeddedUI</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>MsiEmbeddedUI</td><td>MessageFilter</td><td>Y</td><td>0</td><td>234913791</td><td/><td/><td>Integer</td><td/><td>Specifies the types of messages that are sent to the user interface DLL. This column is only relevant for rows with the msidbEmbeddedUI attribute.</td></row>
		<row><td>MsiEmbeddedUI</td><td>MsiEmbeddedUI</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The primary key for the table.</td></row>
		<row><td>MsiFileHash</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Primary key, foreign key into File table referencing file with this hash</td></row>
		<row><td>MsiFileHash</td><td>HashPart1</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Size of file in bytes (long integer).</td></row>
		<row><td>MsiFileHash</td><td>HashPart2</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Size of file in bytes (long integer).</td></row>
		<row><td>MsiFileHash</td><td>HashPart3</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Size of file in bytes (long integer).</td></row>
		<row><td>MsiFileHash</td><td>HashPart4</td><td>N</td><td/><td/><td/><td/><td/><td/><td>Size of file in bytes (long integer).</td></row>
		<row><td>MsiFileHash</td><td>Options</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Various options and attributes for this hash.</td></row>
		<row><td>MsiLockPermissionsEx</td><td>Condition</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Expression which must evaluate to TRUE in order for this set of permissions to be applied</td></row>
		<row><td>MsiLockPermissionsEx</td><td>LockObject</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into Registry, File, CreateFolder, or ServiceInstall table</td></row>
		<row><td>MsiLockPermissionsEx</td><td>MsiLockPermissionsEx</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token</td></row>
		<row><td>MsiLockPermissionsEx</td><td>SDDLText</td><td>N</td><td/><td/><td/><td/><td>FormattedSDDLText</td><td/><td>String to indicate permissions to be applied to the LockObject</td></row>
		<row><td>MsiLockPermissionsEx</td><td>Table</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td>CreateFolder;File;Registry;ServiceInstall</td><td>Reference to another table name</td></row>
		<row><td>MsiPackageCertificate</td><td>DigitalCertificate_</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A foreign key to the digital certificate table</td></row>
		<row><td>MsiPackageCertificate</td><td>PackageCertificate</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique identifier for the row</td></row>
		<row><td>MsiPatchCertificate</td><td>DigitalCertificate_</td><td>N</td><td/><td/><td>MsiDigitalCertificate</td><td>1</td><td>Identifier</td><td/><td>A foreign key to the digital certificate table</td></row>
		<row><td>MsiPatchCertificate</td><td>PatchCertificate</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique identifier for the row</td></row>
		<row><td>MsiPatchMetadata</td><td>Company</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Optional company name</td></row>
		<row><td>MsiPatchMetadata</td><td>PatchConfiguration_</td><td>N</td><td/><td/><td>ISPatchConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key to the ISPatchConfiguration table</td></row>
		<row><td>MsiPatchMetadata</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the metadata</td></row>
		<row><td>MsiPatchMetadata</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value of the metadata</td></row>
		<row><td>MsiPatchOldAssemblyFile</td><td>Assembly_</td><td>Y</td><td/><td/><td>MsiPatchOldAssemblyName</td><td>1</td><td/><td/><td/></row>
		<row><td>MsiPatchOldAssemblyFile</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td/><td/><td/></row>
		<row><td>MsiPatchOldAssemblyName</td><td>Assembly</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>MsiPatchOldAssemblyName</td><td>Name</td><td>N</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>MsiPatchOldAssemblyName</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td/><td/><td/></row>
		<row><td>MsiPatchSequence</td><td>PatchConfiguration_</td><td>N</td><td/><td/><td>ISPatchConfiguration</td><td>1</td><td>Text</td><td/><td>Foreign key to the patch configuration table</td></row>
		<row><td>MsiPatchSequence</td><td>PatchFamily</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the family to which this patch belongs</td></row>
		<row><td>MsiPatchSequence</td><td>Sequence</td><td>N</td><td/><td/><td/><td/><td>Version</td><td/><td>The version of this patch in this family</td></row>
		<row><td>MsiPatchSequence</td><td>Supersede</td><td>N</td><td/><td/><td/><td/><td>Integer</td><td/><td>Supersede</td></row>
		<row><td>MsiPatchSequence</td><td>Target</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Target product codes for this patch family</td></row>
		<row><td>MsiServiceConfig</td><td>Argument</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Argument(s) for service configuration. Value depends on the content of the ConfigType field</td></row>
		<row><td>MsiServiceConfig</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table that controls the configuration of the service</td></row>
		<row><td>MsiServiceConfig</td><td>ConfigType</td><td>N</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Service Configuration Option</td></row>
		<row><td>MsiServiceConfig</td><td>Event</td><td>N</td><td>0</td><td>7</td><td/><td/><td/><td/><td>Bit field:   0x1 = Install, 0x2 = Uninstall, 0x4 = Reinstall</td></row>
		<row><td>MsiServiceConfig</td><td>MsiServiceConfig</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>MsiServiceConfig</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Name of a service. /, \, comma and space are invalid</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>Actions</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>A list of integer actions separated by [~] delimiters: 0 = SC_ACTION_NONE, 1 = SC_ACTION_RESTART, 2 = SC_ACTION_REBOOT, 3 = SC_ACTION_RUN_COMMAND. Terminate with [~][~]</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>Command</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Command line of the process to CreateProcess function to execute</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table that controls the configuration of the service</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>DelayActions</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>A list of delays (time in milli-seconds), separated by [~] delmiters, to wait before taking the corresponding Action. Terminate with [~][~]</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>Event</td><td>N</td><td>0</td><td>7</td><td/><td/><td/><td/><td>Bit field:   0x1 = Install, 0x2 = Uninstall, 0x4 = Reinstall</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>MsiServiceConfigFailureActions</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Name of a service. /, \, comma and space are invalid</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>RebootMessage</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Message to be broadcast to server users before rebooting</td></row>
		<row><td>MsiServiceConfigFailureActions</td><td>ResetPeriod</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Time in seconds after which to reset the failure count to zero. Leave blank if it should never be reset</td></row>
		<row><td>MsiShortcutProperty</td><td>MsiShortcutProperty</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token</td></row>
		<row><td>MsiShortcutProperty</td><td>PropVariantValue</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>String representation of the value in the property</td></row>
		<row><td>MsiShortcutProperty</td><td>PropertyKey</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Canonical string representation of the Property Key being set</td></row>
		<row><td>MsiShortcutProperty</td><td>Shortcut_</td><td>N</td><td/><td/><td>Shortcut</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Shortcut table</td></row>
		<row><td>ODBCAttribute</td><td>Attribute</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of ODBC driver attribute</td></row>
		<row><td>ODBCAttribute</td><td>Driver_</td><td>N</td><td/><td/><td>ODBCDriver</td><td>1</td><td>Identifier</td><td/><td>Reference to ODBC driver in ODBCDriver table</td></row>
		<row><td>ODBCAttribute</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value for ODBC driver attribute</td></row>
		<row><td>ODBCDataSource</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Reference to associated component</td></row>
		<row><td>ODBCDataSource</td><td>DataSource</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized.internal token for data source</td></row>
		<row><td>ODBCDataSource</td><td>Description</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Text used as registered name for data source</td></row>
		<row><td>ODBCDataSource</td><td>DriverDescription</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Reference to driver description, may be existing driver</td></row>
		<row><td>ODBCDataSource</td><td>Registration</td><td>N</td><td>0</td><td>1</td><td/><td/><td/><td/><td>Registration option: 0=machine, 1=user, others t.b.d.</td></row>
		<row><td>ODBCDriver</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Reference to associated component</td></row>
		<row><td>ODBCDriver</td><td>Description</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Text used as registered name for driver, non-localized</td></row>
		<row><td>ODBCDriver</td><td>Driver</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized.internal token for driver</td></row>
		<row><td>ODBCDriver</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Reference to key driver file</td></row>
		<row><td>ODBCDriver</td><td>File_Setup</td><td>Y</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Optional reference to key driver setup DLL</td></row>
		<row><td>ODBCSourceAttribute</td><td>Attribute</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of ODBC data source attribute</td></row>
		<row><td>ODBCSourceAttribute</td><td>DataSource_</td><td>N</td><td/><td/><td>ODBCDataSource</td><td>1</td><td>Identifier</td><td/><td>Reference to ODBC data source in ODBCDataSource table</td></row>
		<row><td>ODBCSourceAttribute</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Value for ODBC data source attribute</td></row>
		<row><td>ODBCTranslator</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Reference to associated component</td></row>
		<row><td>ODBCTranslator</td><td>Description</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>Text used as registered name for translator</td></row>
		<row><td>ODBCTranslator</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Reference to key translator file</td></row>
		<row><td>ODBCTranslator</td><td>File_Setup</td><td>Y</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Optional reference to key translator setup DLL</td></row>
		<row><td>ODBCTranslator</td><td>Translator</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized.internal token for translator</td></row>
		<row><td>Patch</td><td>Attributes</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Integer containing bit flags representing patch attributes</td></row>
		<row><td>Patch</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Primary key, non-localized token, foreign key to File table, must match identifier in cabinet.</td></row>
		<row><td>Patch</td><td>Header</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>Binary stream. The patch header, used for patch validation.</td></row>
		<row><td>Patch</td><td>ISBuildSourcePath</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Full path to patch header.</td></row>
		<row><td>Patch</td><td>PatchSize</td><td>N</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Size of patch in bytes (long integer).</td></row>
		<row><td>Patch</td><td>Sequence</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Primary key, sequence with respect to the media images; order must track cabinet order.</td></row>
		<row><td>Patch</td><td>StreamRef_</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>External key into the MsiPatchHeaders table specifying the row that contains the patch header stream.</td></row>
		<row><td>PatchPackage</td><td>Media_</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Foreign key to DiskId column of Media table. Indicates the disk containing the patch package.</td></row>
		<row><td>PatchPackage</td><td>PatchId</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>A unique string GUID representing this patch.</td></row>
		<row><td>ProgId</td><td>Class_</td><td>Y</td><td/><td/><td>Class</td><td>1</td><td>Guid</td><td/><td>The CLSID of an OLE factory corresponding to the ProgId.</td></row>
		<row><td>ProgId</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Localized description for the Program identifier.</td></row>
		<row><td>ProgId</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties of a component, like ExtractIcon, etc.</td></row>
		<row><td>ProgId</td><td>IconIndex</td><td>Y</td><td>-32767</td><td>32767</td><td/><td/><td/><td/><td>Optional icon index.</td></row>
		<row><td>ProgId</td><td>Icon_</td><td>Y</td><td/><td/><td>Icon</td><td>1</td><td>Identifier</td><td/><td>Optional foreign key into the Icon Table, specifying the icon file associated with this ProgId. Will be written under the DefaultIcon key.</td></row>
		<row><td>ProgId</td><td>ProgId</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The Program Identifier. Primary key.</td></row>
		<row><td>ProgId</td><td>ProgId_Parent</td><td>Y</td><td/><td/><td>ProgId</td><td>1</td><td>Text</td><td/><td>The Parent Program Identifier. If specified, the ProgId column becomes a version independent prog id.</td></row>
		<row><td>Property</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>User Comments.</td></row>
		<row><td>Property</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of property, uppercase if settable by launcher or loader.</td></row>
		<row><td>Property</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>String value for property.</td></row>
		<row><td>PublishComponent</td><td>AppData</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>This is localisable Application specific data that can be associated with a Qualified Component.</td></row>
		<row><td>PublishComponent</td><td>ComponentId</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>A string GUID that represents the component id that will be requested by the alien product.</td></row>
		<row><td>PublishComponent</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table.</td></row>
		<row><td>PublishComponent</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Feature table.</td></row>
		<row><td>PublishComponent</td><td>Qualifier</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>This is defined only when the ComponentId column is an Qualified Component Id. This is the Qualifier for ProvideComponentIndirect.</td></row>
		<row><td>RadioButton</td><td>Height</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The height of the button.</td></row>
		<row><td>RadioButton</td><td>Help</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The help strings used with the button. The text is optional.</td></row>
		<row><td>RadioButton</td><td>ISControlId</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>A number used to represent the control ID of the Control, Used in Dialog export</td></row>
		<row><td>RadioButton</td><td>Order</td><td>N</td><td>1</td><td>32767</td><td/><td/><td/><td/><td>A positive integer used to determine the ordering of the items within one list..The integers do not have to be consecutive.</td></row>
		<row><td>RadioButton</td><td>Property</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A named property to be tied to this radio button. All the buttons tied to the same property become part of the same group.</td></row>
		<row><td>RadioButton</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The visible title to be assigned to the radio button.</td></row>
		<row><td>RadioButton</td><td>Value</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value string associated with this button. Selecting the button will set the associated property to this value.</td></row>
		<row><td>RadioButton</td><td>Width</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The width of the button.</td></row>
		<row><td>RadioButton</td><td>X</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The horizontal coordinate of the upper left corner of the bounding rectangle of the radio button.</td></row>
		<row><td>RadioButton</td><td>Y</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The vertical coordinate of the upper left corner of the bounding rectangle of the radio button.</td></row>
		<row><td>RegLocator</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>RegPath</td><td/><td>The key for the registry value.</td></row>
		<row><td>RegLocator</td><td>Name</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The registry value name.</td></row>
		<row><td>RegLocator</td><td>Root</td><td>N</td><td>0</td><td>3</td><td/><td/><td/><td/><td>The predefined root key for the registry value, one of rrkEnum.</td></row>
		<row><td>RegLocator</td><td>Signature_</td><td>N</td><td/><td/><td>Signature</td><td>1</td><td>Identifier</td><td/><td>The table key. The Signature_ represents a unique file signature and is also the foreign key in the Signature table. If the type is 0, the registry values refers a directory, and _Signature is not a foreign key.</td></row>
		<row><td>RegLocator</td><td>Type</td><td>Y</td><td>0</td><td>18</td><td/><td/><td/><td/><td>An integer value that determines if the registry value is a filename or a directory location or to be used as is w/o interpretation.</td></row>
		<row><td>Registry</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table referencing component that controls the installing of the registry value.</td></row>
		<row><td>Registry</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties of a registry item.  Currently the only one is Automatic.</td></row>
		<row><td>Registry</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>RegPath</td><td/><td>The key for the registry value.</td></row>
		<row><td>Registry</td><td>Name</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The registry value name.</td></row>
		<row><td>Registry</td><td>Registry</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>Registry</td><td>Root</td><td>N</td><td>-1</td><td>3</td><td/><td/><td/><td/><td>The predefined root key for the registry value, one of rrkEnum.</td></row>
		<row><td>Registry</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The registry value.</td></row>
		<row><td>RemoveFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key referencing Component that controls the file to be removed.</td></row>
		<row><td>RemoveFile</td><td>DirProperty</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of a property whose value is assumed to resolve to the full pathname to the folder of the file to be removed.</td></row>
		<row><td>RemoveFile</td><td>FileKey</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key used to identify a particular file entry</td></row>
		<row><td>RemoveFile</td><td>FileName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Name of the file to be removed.</td></row>
		<row><td>RemoveFile</td><td>InstallMode</td><td>N</td><td/><td/><td/><td/><td/><td>1;2;3</td><td>Installation option, one of iimEnum.</td></row>
		<row><td>RemoveIniFile</td><td>Action</td><td>N</td><td/><td/><td/><td/><td/><td>2;4</td><td>The type of modification to be made, one of iifEnum.</td></row>
		<row><td>RemoveIniFile</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table referencing component that controls the deletion of the .INI value.</td></row>
		<row><td>RemoveIniFile</td><td>DirProperty</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Foreign key into the Directory table denoting the directory where the .INI file is.</td></row>
		<row><td>RemoveIniFile</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The .INI file name in which to delete the information</td></row>
		<row><td>RemoveIniFile</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The .INI file key below Section.</td></row>
		<row><td>RemoveIniFile</td><td>RemoveIniFile</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>RemoveIniFile</td><td>Section</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The .INI file Section.</td></row>
		<row><td>RemoveIniFile</td><td>Value</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The value to be deleted. The value is required when Action is iifIniRemoveTag</td></row>
		<row><td>RemoveRegistry</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table referencing component that controls the deletion of the registry value.</td></row>
		<row><td>RemoveRegistry</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>RegPath</td><td/><td>The key for the registry value.</td></row>
		<row><td>RemoveRegistry</td><td>Name</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The registry value name.</td></row>
		<row><td>RemoveRegistry</td><td>RemoveRegistry</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>RemoveRegistry</td><td>Root</td><td>N</td><td>-1</td><td>3</td><td/><td/><td/><td/><td>The predefined root key for the registry value, one of rrkEnum</td></row>
		<row><td>ReserveCost</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Reserve a specified amount of space if this component is to be installed.</td></row>
		<row><td>ReserveCost</td><td>ReserveFolder</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of a property whose value is assumed to resolve to the full path to the destination directory</td></row>
		<row><td>ReserveCost</td><td>ReserveKey</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key that uniquely identifies a particular ReserveCost record</td></row>
		<row><td>ReserveCost</td><td>ReserveLocal</td><td>N</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Disk space to reserve if linked component is installed locally.</td></row>
		<row><td>ReserveCost</td><td>ReserveSource</td><td>N</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>Disk space to reserve if linked component is installed to run from the source location.</td></row>
		<row><td>SFPCatalog</td><td>Catalog</td><td>Y</td><td/><td/><td/><td/><td>Binary</td><td/><td>SFP Catalog</td></row>
		<row><td>SFPCatalog</td><td>Dependency</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Parent catalog - only used by SFP</td></row>
		<row><td>SFPCatalog</td><td>SFPCatalog</td><td>N</td><td/><td/><td/><td/><td>Filename</td><td/><td>File name for the catalog.</td></row>
		<row><td>SelfReg</td><td>Cost</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The cost of registering the module.</td></row>
		<row><td>SelfReg</td><td>File_</td><td>N</td><td/><td/><td>File</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table denoting the module that needs to be registered.</td></row>
		<row><td>ServiceControl</td><td>Arguments</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Arguments for the service.  Separate by [~].</td></row>
		<row><td>ServiceControl</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table that controls the startup of the service</td></row>
		<row><td>ServiceControl</td><td>Event</td><td>N</td><td>0</td><td>187</td><td/><td/><td/><td/><td>Bit field:  Install:  0x1 = Start, 0x2 = Stop, 0x8 = Delete, Uninstall: 0x10 = Start, 0x20 = Stop, 0x80 = Delete</td></row>
		<row><td>ServiceControl</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Name of a service. /, \, comma and space are invalid</td></row>
		<row><td>ServiceControl</td><td>ServiceControl</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>ServiceControl</td><td>Wait</td><td>Y</td><td>0</td><td>1</td><td/><td/><td/><td/><td>Boolean for whether to wait for the service to fully start</td></row>
		<row><td>ServiceInstall</td><td>Arguments</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Arguments to include in every start of the service, passed to WinMain</td></row>
		<row><td>ServiceInstall</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table that controls the startup of the service</td></row>
		<row><td>ServiceInstall</td><td>Dependencies</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Other services this depends on to start.  Separate by [~], and end with [~][~]</td></row>
		<row><td>ServiceInstall</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Description of service.</td></row>
		<row><td>ServiceInstall</td><td>DisplayName</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>External Name of the Service</td></row>
		<row><td>ServiceInstall</td><td>ErrorControl</td><td>N</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Severity of error if service fails to start</td></row>
		<row><td>ServiceInstall</td><td>LoadOrderGroup</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>LoadOrderGroup</td></row>
		<row><td>ServiceInstall</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Internal Name of the Service</td></row>
		<row><td>ServiceInstall</td><td>Password</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>password to run service with.  (with StartName)</td></row>
		<row><td>ServiceInstall</td><td>ServiceInstall</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>ServiceInstall</td><td>ServiceType</td><td>N</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Type of the service</td></row>
		<row><td>ServiceInstall</td><td>StartName</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>User or object name to run service as</td></row>
		<row><td>ServiceInstall</td><td>StartType</td><td>N</td><td>0</td><td>4</td><td/><td/><td/><td/><td>Type of the service</td></row>
		<row><td>Shortcut</td><td>Arguments</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The command-line arguments for the shortcut.</td></row>
		<row><td>Shortcut</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Component table denoting the component whose selection gates the the shortcut creation/deletion.</td></row>
		<row><td>Shortcut</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The description for the shortcut.</td></row>
		<row><td>Shortcut</td><td>DescriptionResourceDLL</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>This field contains a Formatted string value for the full path to the language neutral file that contains the MUI manifest.</td></row>
		<row><td>Shortcut</td><td>DescriptionResourceId</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The description name index for the shortcut.</td></row>
		<row><td>Shortcut</td><td>Directory_</td><td>N</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the Directory table denoting the directory where the shortcut file is created.</td></row>
		<row><td>Shortcut</td><td>DisplayResourceDLL</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>This field contains a Formatted string value for the full path to the language neutral file that contains the MUI manifest.</td></row>
		<row><td>Shortcut</td><td>DisplayResourceId</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The display name index for the shortcut.</td></row>
		<row><td>Shortcut</td><td>Hotkey</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The hotkey for the shortcut. It has the virtual-key code for the key in the low-order byte, and the modifier flags in the high-order byte.</td></row>
		<row><td>Shortcut</td><td>ISAttributes</td><td>Y</td><td/><td/><td/><td/><td/><td/><td>This is used to store Installshield custom properties of a shortcut.  Mainly used in pro project types.</td></row>
		<row><td>Shortcut</td><td>ISComments</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Author’s comments on this Shortcut.</td></row>
		<row><td>Shortcut</td><td>ISShortcutName</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>A non-unique name for the shortcut.  Mainly used by pro pro project types.</td></row>
		<row><td>Shortcut</td><td>IconIndex</td><td>Y</td><td>-32767</td><td>32767</td><td/><td/><td/><td/><td>The icon index for the shortcut.</td></row>
		<row><td>Shortcut</td><td>Icon_</td><td>Y</td><td/><td/><td>Icon</td><td>1</td><td>Identifier</td><td/><td>Foreign key into the File table denoting the external icon file for the shortcut.</td></row>
		<row><td>Shortcut</td><td>Name</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the shortcut to be created.</td></row>
		<row><td>Shortcut</td><td>Shortcut</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Primary key, non-localized token.</td></row>
		<row><td>Shortcut</td><td>ShowCmd</td><td>Y</td><td/><td/><td/><td/><td/><td>1;3;7</td><td>The show command for the application window.The following values may be used.</td></row>
		<row><td>Shortcut</td><td>Target</td><td>N</td><td/><td/><td/><td/><td>Shortcut</td><td/><td>The shortcut target. This is usually a property that is expanded to a file or a folder that the shortcut points to.</td></row>
		<row><td>Shortcut</td><td>WkDir</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of property defining location of working directory.</td></row>
		<row><td>Signature</td><td>FileName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The name of the file. This may contain a "short name|long name" pair.</td></row>
		<row><td>Signature</td><td>Languages</td><td>Y</td><td/><td/><td/><td/><td>Language</td><td/><td>The languages supported by the file.</td></row>
		<row><td>Signature</td><td>MaxDate</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The maximum creation date of the file.</td></row>
		<row><td>Signature</td><td>MaxSize</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The maximum size of the file.</td></row>
		<row><td>Signature</td><td>MaxVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The maximum version of the file.</td></row>
		<row><td>Signature</td><td>MinDate</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The minimum creation date of the file.</td></row>
		<row><td>Signature</td><td>MinSize</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The minimum size of the file.</td></row>
		<row><td>Signature</td><td>MinVersion</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The minimum version of the file.</td></row>
		<row><td>Signature</td><td>Signature</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>The table key. The Signature represents a unique file signature.</td></row>
		<row><td>TextStyle</td><td>Color</td><td>Y</td><td>0</td><td>16777215</td><td/><td/><td/><td/><td>A long integer indicating the color of the string in the RGB format (Red, Green, Blue each 0-255, RGB = R + 256*G + 256^2*B).</td></row>
		<row><td>TextStyle</td><td>FaceName</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>A string indicating the name of the font used. Required. The string must be at most 31 characters long.</td></row>
		<row><td>TextStyle</td><td>Size</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The size of the font used. This size is given in our units (1/12 of the system font height). Assuming that the system font is set to 12 point size, this is equivalent to the point size.</td></row>
		<row><td>TextStyle</td><td>StyleBits</td><td>Y</td><td>0</td><td>15</td><td/><td/><td/><td/><td>A combination of style bits.</td></row>
		<row><td>TextStyle</td><td>TextStyle</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of the style. The primary key of this table. This name is embedded in the texts to indicate a style change.</td></row>
		<row><td>TypeLib</td><td>Component_</td><td>N</td><td/><td/><td>Component</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Component Table, specifying the component for which to return a path when called through LocateComponent.</td></row>
		<row><td>TypeLib</td><td>Cost</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The cost associated with the registration of the typelib. This column is currently optional.</td></row>
		<row><td>TypeLib</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td/></row>
		<row><td>TypeLib</td><td>Directory_</td><td>Y</td><td/><td/><td>Directory</td><td>1</td><td>Identifier</td><td/><td>Optional. The foreign key into the Directory table denoting the path to the help file for the type library.</td></row>
		<row><td>TypeLib</td><td>Feature_</td><td>N</td><td/><td/><td>Feature</td><td>1</td><td>Identifier</td><td/><td>Required foreign key into the Feature Table, specifying the feature to validate or install in order for the type library to be operational.</td></row>
		<row><td>TypeLib</td><td>Language</td><td>N</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>The language of the library.</td></row>
		<row><td>TypeLib</td><td>LibID</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>The GUID that represents the library.</td></row>
		<row><td>TypeLib</td><td>Version</td><td>Y</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The version of the library. The major version is in the upper 8 bits of the short integer. The minor version is in the lower 8 bits.</td></row>
		<row><td>UIText</td><td>Key</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>A unique key that identifies the particular string.</td></row>
		<row><td>UIText</td><td>Text</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The localized version of the string.</td></row>
		<row><td>Upgrade</td><td>ActionProperty</td><td>N</td><td/><td/><td/><td/><td>UpperCase</td><td/><td>The property to set when a product in this set is found.</td></row>
		<row><td>Upgrade</td><td>Attributes</td><td>N</td><td>0</td><td>2147483647</td><td/><td/><td/><td/><td>The attributes of this product set.</td></row>
		<row><td>Upgrade</td><td>ISDisplayName</td><td>Y</td><td/><td/><td>ISUpgradeMsiItem</td><td>1</td><td/><td/><td/></row>
		<row><td>Upgrade</td><td>Language</td><td>Y</td><td/><td/><td/><td/><td>Language</td><td/><td>A comma-separated list of languages for either products in this set or products not in this set.</td></row>
		<row><td>Upgrade</td><td>Remove</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The list of features to remove when uninstalling a product from this set.  The default is "ALL".</td></row>
		<row><td>Upgrade</td><td>UpgradeCode</td><td>N</td><td/><td/><td/><td/><td>Guid</td><td/><td>The UpgradeCode GUID belonging to the products in this set.</td></row>
		<row><td>Upgrade</td><td>VersionMax</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The maximum ProductVersion of the products in this set.  The set may or may not include products with this particular version.</td></row>
		<row><td>Upgrade</td><td>VersionMin</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>The minimum ProductVersion of the products in this set.  The set may or may not include products with this particular version.</td></row>
		<row><td>Verb</td><td>Argument</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>Optional value for the command arguments.</td></row>
		<row><td>Verb</td><td>Command</td><td>Y</td><td/><td/><td/><td/><td>Formatted</td><td/><td>The command text.</td></row>
		<row><td>Verb</td><td>Extension_</td><td>N</td><td/><td/><td>Extension</td><td>1</td><td>Text</td><td/><td>The extension associated with the table row.</td></row>
		<row><td>Verb</td><td>Sequence</td><td>Y</td><td>0</td><td>32767</td><td/><td/><td/><td/><td>Order within the verbs for a particular extension. Also used simply to specify the default verb.</td></row>
		<row><td>Verb</td><td>Verb</td><td>N</td><td/><td/><td/><td/><td>Text</td><td/><td>The verb for the command.</td></row>
		<row><td>_Validation</td><td>Category</td><td>Y</td><td/><td/><td/><td/><td/><td>"Text";"Formatted";"Template";"Condition";"Guid";"Path";"Version";"Language";"Identifier";"Binary";"UpperCase";"LowerCase";"Filename";"Paths";"AnyPath";"WildCardFilename";"RegPath";"KeyFormatted";"CustomSource";"Property";"Cabinet";"Shortcut";"URL";"DefaultDir"</td><td>String category</td></row>
		<row><td>_Validation</td><td>Column</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of column</td></row>
		<row><td>_Validation</td><td>Description</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Description of column</td></row>
		<row><td>_Validation</td><td>KeyColumn</td><td>Y</td><td>1</td><td>32</td><td/><td/><td/><td/><td>Column to which foreign key connects</td></row>
		<row><td>_Validation</td><td>KeyTable</td><td>Y</td><td/><td/><td/><td/><td>Identifier</td><td/><td>For foreign key, Name of table to which data must link</td></row>
		<row><td>_Validation</td><td>MaxValue</td><td>Y</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Maximum value allowed</td></row>
		<row><td>_Validation</td><td>MinValue</td><td>Y</td><td>-2147483647</td><td>2147483647</td><td/><td/><td/><td/><td>Minimum value allowed</td></row>
		<row><td>_Validation</td><td>Nullable</td><td>N</td><td/><td/><td/><td/><td/><td>Y;N;@</td><td>Whether the column is nullable</td></row>
		<row><td>_Validation</td><td>Set</td><td>Y</td><td/><td/><td/><td/><td>Text</td><td/><td>Set of values that are permitted</td></row>
		<row><td>_Validation</td><td>Table</td><td>N</td><td/><td/><td/><td/><td>Identifier</td><td/><td>Name of table</td></row>
	</table>
</msi>
