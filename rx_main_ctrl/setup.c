// ****************************************************************************
//
//	DIGITAL PRINTING - setup.c
//
// ****************************************************************************
//
//	Copyright 2013 by Radex AG, Switzerland. All rights reserved.
//	Written by Marcel Galliker
//
// ****************************************************************************

#include "rx_common.h"
#include "rx_file.h"
#include "rx_hash.h"
#include "rx_setup_file.h"
#include "rx_setup_ink.h"
#include "setup.h"
#include "network.h"

//--- prototypes --------------------------------------------------------------

static void _head_pressure_out_override(SRxConfig *pcfg);

//--- setup_test --------------------------------------------------------------
void setup_test()
{
	HANDLE file;

	file = setup_create();
	setup_load(file, "D:\\temp\\setup.xml");

	//--- init network --------------------------------
	SRxNetwork network;

	memset(&network, 0, sizeof(network));

	network.item[0].macAddr		= 1000;
	network.item[0].deviceType	= dev_enc;
	network.item[0].deviceNo	= 0;

	network.item[1].macAddr		= 2000;
	network.item[1].deviceType	= dev_head;
	network.item[1].deviceNo	= 0;

	network.item[2].macAddr		= 2001;
	network.item[2].deviceType	= dev_head;
	network.item[2].deviceNo	= 1;

	network.item[9].macAddr	= 2009;
	network.item[9].deviceType	= dev_head;
	network.item[9].deviceNo	= 9;

	setup_network(file, &network, WRITE);

	setup_save(file, "D:\\temp\\setup1.xml");
	//-------------------------------------------------

	memset(&network, 0, sizeof(network));
	setup_load(file, "D:\\temp\\setup1.xml");
	setup_network(file, &network, READ);


	network.item[1].macAddr = 20323;

	network.item[7].macAddr		= 2007;
	network.item[7].deviceType	= dev_head;
	network.item[7].deviceNo	= 7;	

	setup_network(file, &network, WRITE);

	setup_save(file, "D:\\temp\\setup2.xml");
}

//--- setup_network -----------------------------------------------------------
int setup_network(HANDLE file, SRxNetwork *pnet, EN_setup_Action  action)
{
	int i;
	
	setup_chapter(file, "Network", -1, action);
	for (i = 0; i < SIZEOF(pnet->item); i++)
	{
		if (action==READ || pnet->item[i].macAddr)
		{
			if (setup_chapter(file, "item", i, action)==REPLY_OK)
			{
				setup_str     (file, "DevTypeStr",	action,  pnet->item[i].deviceTypeStr, sizeof(pnet->item[i].deviceTypeStr), "undef");
				setup_str     (file, "SerialNo",	action,  pnet->item[i].serialNo,	  sizeof(pnet->item[i].serialNo), "");
				setup_mac_addr(file, "MacAddr",		action, &pnet->item[i].macAddr,			0);
				setup_uchar   (file, "DevType",		action, &pnet->item[i].deviceType,		dev_undef);
				setup_uchar   (file, "DevNo",		action, &pnet->item[i].deviceNo,		0);
				if (pnet->item[i].deviceNo==255) pnet->item[i].deviceNo=0;
				setup_chapter (file, "..", i, action);
			}
		}
	}
	setup_chapter(file, "..", 0, action);
	return REPLY_OK;
}

//--- setup_config ----------------------------------------------------------------------------
int setup_config(const char *filepath, SRxConfig *pcfg, EN_setup_Action  action)
{
	int i, h;
	char path[MAX_PATH];
	HANDLE file = setup_create();

	setup_load(file, filepath);
	
	if (setup_chapter(file, "Configuration", -1, action)!=REPLY_OK) return REPLY_ERROR;

	setup_uchar(file, "Simulation", action, &pcfg->simulation, 0);
	setup_str  (file, "material", action,  pcfg->material,	sizeof(pcfg->material),	"");

	//--- printer ---
	if (setup_chapter(file, "Printer", -1, action)==REPLY_OK) 
	{
		setup_int32(file, "type", action, (int*)&pcfg->printer.type, printer_undef);
		setup_int32(file, "encoderType", action, (int*)&pcfg->printer.encoderType, enc_Balluff);

		setup_uint32(file, "overlap", action, &pcfg->printer.overlap, TRUE);
		if (pcfg->printer.type!=printer_TX801 && pcfg->printer.type!=printer_TX802 && pcfg->printer.type!=printer_test_table)	pcfg->printer.overlap = TRUE;
		setup_uint32(file, "externalData", action, &pcfg->externalData, FALSE);
		setup_chapter(file, "..", -1, action);
	}

	//--- offsets ---
	{
		if (setup_chapter(file, "Offsets", -1, action)==REPLY_OK)
		{
			setup_int32(file, "offset_angle",				action, (int*)&pcfg->printer.offset.angle, 0);
			setup_int32(file, "offset_step",				action, (int*)&pcfg->printer.offset.step, 0);
			setup_int32(file, "offset_incPerMeter",			action, &pcfg->printer.offset.incPerMeter[0], 0);
			setup_int32(file, "offset_incPerMeterVerso",	action, &pcfg->printer.offset.incPerMeter[1], 0);
			setup_uint32(file, "offsetVerso",				action,	&pcfg->printer.offset.versoDist, 0);
			setup_uint32(file, "manualFlightTimeComp",		action, &pcfg->printer.offset.manualFlightTimeComp, 0);
			setup_chapter(file, "..", -1, action);			
		}
	}
	
	//--- encoder ---
	if (setup_chapter(file, "Encoder", -1, action)==REPLY_OK)
	{
		setup_int32_arr(file, "corrRotPar",   action, pcfg->encoder[0].corrRotPar, SIZEOF(pcfg->encoder[0].corrRotPar), 0);
		setup_int32_arr(file, "corrRotPar1",  action, pcfg->encoder[1].corrRotPar, SIZEOF(pcfg->encoder[1].corrRotPar), 0);
		setup_chapter(file, "..", -1, action);
	}

	//--- stepper ---
	if (setup_chapter(file, "Stepper", -1, action)==REPLY_OK) 
	{
		setup_int32(file, "ref_height",		action, &pcfg->stepper.ref_height,		0);
		setup_int32(file, "print_height",	action, &pcfg->stepper.print_height,	0);
		setup_int32(file, "wipe_height",	action, &pcfg->stepper.wipe_height,		0);
		setup_int32(file, "wipe_delay",		action, &pcfg->stepper.wipe_delay,		0);
		setup_int32(file, "wipe_speed",		action, &pcfg->stepper.wipe_speed,		0);
		setup_int32(file, "cap_height",		action, &pcfg->stepper.cap_height,		0);
		setup_int32(file, "cap_pos",		action, &pcfg->stepper.cap_pos,			0);
		for (i=0; i<SIZEOF(pcfg->stepper.robot); i++)
		{
			if (setup_chapter(file, "robot", i, action)==REPLY_OK) 
			{	
				setup_int32(file, "ref_height",	  action, &pcfg->stepper.robot[i].ref_height,	0);
				setup_int32(file, "head_align",	  action, &pcfg->stepper.robot[i].head_align,	0);
				setup_int32(file, "ref_height_back", action, &pcfg->stepper.robot[i].ref_height_back, 0);
				setup_int32(file, "ref_height_front", action, &pcfg->stepper.robot[i].ref_height_front, 0);
				setup_int32(file, "cap_height", action, &pcfg->stepper.robot[i].cap_height, 0);
				setup_chapter(file, "..", -1, action);
			}				
		}
		setup_chapter(file, "..", -1, action);
	}
	setup_int32_arr(file, "Tara",  action, pcfg->scales.tara,  MAX_SCALES, 0);
	setup_int32_arr(file, "Calib", action, pcfg->scales.calib, MAX_SCALES, 0);

	//--- ink supply ---
	setup_uchar(file, "InkSupplyCnt", action, &pcfg->inkSupplyCnt, 1);
	setup_uchar(file, "HeadsPerColor", action, &pcfg->headsPerColor, 1);
	setup_uchar(file, "InkCylinderPerColorCnt", action, &pcfg->inkCylindersPerColor, 1); // if value is not available yet, the value is set to 1
	if(pcfg->inkCylindersPerColor != 0)
	{
		pcfg->colorCnt = pcfg->inkSupplyCnt / pcfg->inkCylindersPerColor;
	}
	else
	{
		pcfg->colorCnt = 1; // invalid config! -> set to 1 (Default)
	}
	
	for (i=0; i<pcfg->inkSupplyCnt; i++)
	{
		if (setup_chapter(file, "InkSupply", i, action)==REPLY_OK) 
		{
			setup_str(file, "InkName", action,  pcfg->inkSupply[i].inkFileName,	sizeof(pcfg->inkSupply[i].inkFileName),	"inkName");
			sprintf(path, "%s%s.wfd", PATH_WAVE_FORM, pcfg->inkSupply[i].inkFileName);
			setup_ink(path, &pcfg->inkSupply[i].ink, READ);
			setup_int32(file, "RectoVerso", action, (int*)&pcfg->inkSupply[i].rectoVerso, 0);
			setup_int32(file, "inkPressureSet", action, (int*)&pcfg->inkSupply[i].cylinderPresSet, 150);
			if (pcfg->inkSupply[i].cylinderPresSet==INVALID_VALUE) pcfg->inkSupply[i].cylinderPresSet=150; 

			int headsPerInkSupply = pcfg->headsPerColor;
			if (pcfg->inkCylindersPerColor) headsPerInkSupply /= pcfg->inkCylindersPerColor;
			setup_int32_arr(file, "headFpVoltage", action, &pcfg->headFpVoltage[i*pcfg->headsPerColor], pcfg->headsPerColor,  100);
			setup_int32_arr(file, "HeadDist",      action, &pcfg->headDist[i*headsPerInkSupply],	  pcfg->headsPerColor,	0);
			setup_int32_arr(file, "HeadDistBack",  action, &pcfg->headDistBack[i*headsPerInkSupply],  pcfg->headsPerColor,	0);
			setup_int32(file, "ColorOffset",	   action, &pcfg->colorOffset[i], 0);
			setup_str(file, "BarcodeScannerSN",	   action,  pcfg->inkSupply[i].scannerSN,	sizeof(pcfg->inkSupply[i].scannerSN),	"");
					
			setup_chapter(file, "..", -1, action);
		}
	}

	if (action==WRITE)
	{
		if (setup_chapter(file, "Density", -1, action)==REPLY_OK)
		{
			for (i=0; i<pcfg->colorCnt; i++)
			{
				if (setup_chapter(file, "Color", i, action)==REPLY_OK) 
				{
					for (h=0; h<pcfg->headsPerColor; h++)
					{
						int n=i*RX_Config.headsPerColor+h;
						int board = n/MAX_HEADS_BOARD;
						int head  = n%MAX_HEADS_BOARD;
						UCHAR	voltage;
						setup_chapter(file, "Head", h, action);
						if (RX_HBStatus[board].head[head].eeprom_mvt.voltage) voltage=RX_HBStatus[board].head[head].eeprom_mvt.voltage;
						else voltage=(UCHAR)RX_HBStatus[board].head[head].eeprom.voltage;
                        setup_uchar(file, "voltage", action, &voltage, 0);
						setup_int16_arr(file, "density",  action, RX_HBStatus[board].head[head].eeprom_mvt.densityValue, MAX_DENSITY_VALUES, 0);
						setup_chapter(file, "..", -1, action);
					}
					setup_chapter(file, "..", -1, action);
				}
			}
			setup_chapter(file, "..", -1, action);
		}
	}

	if (action==WRITE) setup_save(file, filepath);
	setup_destroy(file);

	if (action==READ) _head_pressure_out_override(pcfg);
	return REPLY_OK;
}

//--- _head_pressure_out_override ----------------------------------------------
static void _head_pressure_out_override(SRxConfig *pcfg)
{
	int i;
	HANDLE file = setup_create();
	if (setup_load(file, PATH_USER FILENAME_HEAD_PRESOUT)==REPLY_OK)
	{
		if (setup_chapter(file, "pressure", -1, READ)==REPLY_OK) 
		{		
			for (i=0; i<SIZEOF(pcfg->cond); i++)
			{
				if (setup_chapter(file, "head", i+1, READ)==REPLY_OK) 
				{    									
				//	setup_int16_arr(file, "disabledJets",  READ, pcfg->headDisabledJets[i],	sizeof(pcfg->headDisabledJets[i]),	0);					
					setup_chapter(file, "..", -1, READ);
				}	
   			}
   		}
	}
	setup_destroy(file);	
}

//--- setup_fluid_system --------------------------------------------
int setup_fluid_system	(const char *filepath,	UINT32	*pflushed,	EN_setup_Action  action)
{
	HANDLE file = setup_create();
	setup_load(file, filepath);
	if (setup_chapter(file, "FluidSystem", -1, action)==REPLY_OK)
	{
		setup_uint32(file, "flushed", action, pflushed, 0);
		setup_chapter(file, "..", -1, action);			
	}
	if (action==WRITE) setup_save(file, filepath);
	setup_destroy(file);
	return REPLY_OK;									
}

