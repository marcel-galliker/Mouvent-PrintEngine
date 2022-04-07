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
#include "rx_error.h"
#include "rx_setup_file.h"
#include "rx_setup_ink.h"
#include "setup.h"
#include "network.h"

// --- Communication with master default settings (hybrid machines)
#define MASTER_IP_ADDR_SERVER "172.17.6.10"
#define MASTER_IP_PORT_SERVER 55105
#define PREFIX_PLC "S71500ET200MP station_1.PLC_1"

//--- prototypes --------------------------------------------------------------

static void _head_pressure_out_override(SRxConfig *pcfg, EN_setup_Action action);
static int _setup_config(const char *filepath, SRxConfig *pcfg, EN_setup_Action action);

static BOOL _SaveConfig;


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

void setup_save_config()
{
	_SaveConfig = TRUE;
}

BOOL setup_write_config()
{
	if (_SaveConfig) 
	{
		// copy the original configuration 
		rx_file_copy(PATH_USER FILENAME_CFG, PATH_USER FILENAME_CFG ".save");
		_SaveConfig = FALSE;
		_setup_config(PATH_USER FILENAME_CFG, &RX_Config, WRITE);
		Error(LOG, 0, "Save new configuration");
		return TRUE;
	}
	return FALSE;
}

void setup_read_config()
{
	_setup_config(PATH_USER FILENAME_CFG, &RX_Config, READ);
	if (RX_Config.printer.type == printer_undef)
	{
		// try to retrieve the saved config
		_setup_config(PATH_USER FILENAME_CFG ".save", &RX_Config, READ);
		Error(WARN, 0, "Retrieve configuration from backup");
	}
	
}

//--- setup_config ----------------------------------------------------------------------------
int _setup_config(const char *filepath, SRxConfig *pcfg, EN_setup_Action  action)
{
	int i, h;
	char path[MAX_PATH];
	HANDLE file = setup_create();

	setup_load(file, filepath);

	if (setup_chapter(file, "Configuration", -1, action)!=REPLY_OK) 
	{
		if (action == READ) // if could not read configuration, set default values in config to avoid setting null values at first write
		{
			strcpy(pcfg->master_ip_address, MASTER_IP_ADDR_SERVER);
			pcfg->master_ip_port = MASTER_IP_PORT_SERVER;
			strcpy(pcfg->opcua_prefix, PREFIX_PLC);
			pcfg->lh702_protocol_version = 10; // OPCUA protocol version (TODO remove when only OPCUA)
			pcfg->print_queue_buffer = 64;
			pcfg->mark_reader_ignore_size = 80;
			pcfg->mark_reader_window_size = 25;
		}

		return REPLY_ERROR;
	}

	setup_uchar (file, "Simulation", action, &pcfg->simulation, 0);
	setup_str   (file, "material", action,  pcfg->material,	sizeof(pcfg->material),	"");
    setup_str   (file, "master_ip", action, pcfg->master_ip_address, sizeof(pcfg->master_ip_address), MASTER_IP_ADDR_SERVER);
    setup_uint32(file, "master_port", action, &pcfg->master_ip_port, MASTER_IP_PORT_SERVER);
	setup_str	(file, "opcua_prefix", action, pcfg->opcua_prefix, sizeof(pcfg->opcua_prefix), PREFIX_PLC);
	setup_uint32(file, "lh702_protocol_ver", action, &pcfg->lh702_protocol_version, 10);
	setup_uint32(file, "print_queue_buffer", action, &pcfg->print_queue_buffer, 64);
	setup_uint32(file, "mark_reader_ignore_size", action, &pcfg->mark_reader_ignore_size, 80);
	setup_uint32(file, "mark_reader_window_size", action, &pcfg->mark_reader_window_size, 25);

	//--- printer ---
	if (setup_chapter(file, "Printer", -1, action)==REPLY_OK) 
	{
		setup_int32(file, "type", action, (int*)&pcfg->printer.type, printer_undef);
		setup_int32(file, "encoderType", action, (int*)&pcfg->printer.encoderType, enc_Balluff);
		setup_int32(file, "hybrid", action, &pcfg->printer.hybrid, 0);

		setup_uint32(file, "overlap", action, &pcfg->printer.overlap, TRUE);
        if (!rx_def_is_tx(pcfg->printer.type) && pcfg->printer.type!=printer_test_table)	pcfg->printer.overlap = TRUE;
		setup_uint32(file, "externalData", action, &pcfg->externalData, FALSE);

		setup_uint32(file, "CanisterEmptyHandlingMode", action, &pcfg->canister_empty_handling_mode, err_handling_mode_default);

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
					setup_chapter(file, "Head", h, action);
					setup_uchar(file, "voltage", action, &pcfg->voltage[board][head], 0);
					setup_int16_arr(file, "density", action, pcfg->densityValue[board][head], MAX_DENSITY_VALUES, 0);
					setup_chapter(file, "..", -1, action);
				}
				setup_chapter(file, "..", -1, action);
			}
		}
		setup_chapter(file, "..", -1, action);
	}

    // iQ500 config --------------------------------- //
	if (setup_chapter(file, "iQ500", -1, action) == REPLY_OK) 
	{
		setup_int32(file, "HasInspectionCamera", action, &pcfg->iQ500Cfg.hasInspectionCamera, 0);
		setup_int32(file, "DistanceToCameraInMM", action, &pcfg->iQ500Cfg.distanceToCamera, 0);
        setup_int32(file, "MinNumberOfCopies", action, &pcfg->iQ500Cfg.minNumberOfCopies, 50);

        setup_chapter(file, "..", -1, action);
	}
    
	if (action==WRITE) setup_save(file, filepath);
	setup_destroy(file);

	_head_pressure_out_override(pcfg, action);
	return REPLY_OK;
}

//--- _head_pressure_out_override ----------------------------------------------
static void _head_pressure_out_override(SRxConfig *pcfg, EN_setup_Action action)
{
	int b, h;
	HANDLE file = setup_create();
	setup_load(file, PATH_USER FILENAME_HEAD_PRESOUT);
	if (action == READ) memset(pcfg->headDisabledJets, -1, sizeof(pcfg->headDisabledJets)); // init jet to -1 (0xffff)
	if (setup_chapter(file, "pressure", -1, action) == REPLY_OK) 
	{
		for (b = 0; b < pcfg->colorCnt; b++)
		{
			for (h = 0; h < pcfg->headsPerColor; h++)
			{
				int n = b * RX_Config.headsPerColor + h;
				if (setup_chapter(file, "head", n + 1, action) == REPLY_OK)
				{
					setup_int16_arr(file, "disabledJets", action, pcfg->headDisabledJets[n], MAX_DISABLED_JETS, -1);
					setup_chapter(file, "..", -1, action);
				}
			}	
		}
	}
	if (action == WRITE) setup_save(file, PATH_USER FILENAME_HEAD_PRESOUT);
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

//--- setup_recovery ----------------------------------------------------
int setup_recovery(const char* filepath, SRecoveryFct* data, EN_setup_Action  action)
{
	int i;
	HANDLE file = setup_create();
	if (!rx_file_exists(filepath)) action = WRITE;

	setup_load(file, filepath);

	if (setup_chapter(file, "RecoveryData", -1, action) == REPLY_OK)
	{

		for (i = 0; i < SIZEOF(data->freq_hz); i++)
		{
			if (setup_chapter(file, "PrintingFreq", i, action) == REPLY_OK)
			{
				setup_int32(file, "Hz", action, &data->freq_hz[i], 0);
				setup_chapter(file, "..", -1, action);
			}
		}

		for (i = 0; i < SIZEOF(data->printing_time_min); i++)
		{
			if (setup_chapter(file, "PrintingTime", i, action) == REPLY_OK)
			{
				setup_int32(file, "Min", action, &data->printing_time_min[i], 0);
				setup_chapter(file, "..", -1, action);
			}
		}

		if (setup_chapter(file, "PurgeTime", -1, action) == REPLY_OK)
		{
			setup_int32(file, "Sec", action, &data->purge_time_s, 0);
			setup_chapter(file, "..", -1, action);
		}

		if (setup_chapter(file, "Repetion", -1, action) == REPLY_OK)
		{
			setup_int32(file, "Rep", action, &data->repetion, 0);
			setup_chapter(file, "..", -1, action);
		}
	}

	if (action == WRITE) setup_save(file, filepath);
	setup_destroy(file);
	return REPLY_OK;

}