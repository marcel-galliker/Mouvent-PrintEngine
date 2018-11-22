# TCL File Generated by Component Editor 14.0
# Tue Aug 25 17:22:49 CEST 2015
# DO NOT MODIFY


# 
# avalon_cs_reg_slave "Avalon Config Status Register Slave" v1.0
# stefan.brantschen@fhnw.ch 2015.08.25.17:22:49
# 
# 

# 
# request TCL package from ACDS 14.0
# 
package require -exact qsys 14.0


# 
# module avalon_cs_reg_slave
# 
set_module_property DESCRIPTION ""
set_module_property NAME avalon_cs_reg_slave
set_module_property VERSION 1.0
set_module_property INTERNAL false
set_module_property OPAQUE_ADDRESS_MAP true
set_module_property GROUP Other
set_module_property AUTHOR stefan.brantschen@fhnw.ch
set_module_property DISPLAY_NAME "Avalon Config Status Register Slave"
set_module_property INSTANTIATE_IN_SYSTEM_MODULE true
set_module_property EDITABLE true
set_module_property REPORT_TO_TALKBACK false
set_module_property ALLOW_GREYBOX_GENERATION false
set_module_property REPORT_HIERARCHY false


# 
# file sets
# 
add_fileset QUARTUS_SYNTH QUARTUS_SYNTH "" ""
set_fileset_property QUARTUS_SYNTH TOP_LEVEL avalon_cs_reg_slave
set_fileset_property QUARTUS_SYNTH ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property QUARTUS_SYNTH ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file avalon_cs_reg_slave_rtl.vhd VHDL PATH ../../2_hdldesigner/core_lib/hdl/avalon_cs_reg_slave_rtl.vhd TOP_LEVEL_FILE

add_fileset SIM_VHDL SIM_VHDL "" ""
set_fileset_property SIM_VHDL TOP_LEVEL avalon_cs_reg_slave
set_fileset_property SIM_VHDL ENABLE_RELATIVE_INCLUDE_PATHS false
set_fileset_property SIM_VHDL ENABLE_FILE_OVERWRITE_MODE false
add_fileset_file avalon_cs_reg_slave_rtl.vhd VHDL PATH ../../2_hdldesigner/core_lib/hdl/avalon_cs_reg_slave_rtl.vhd


# 
# parameters
# 
add_parameter g_data_width INTEGER 32
set_parameter_property g_data_width DEFAULT_VALUE 32
set_parameter_property g_data_width DISPLAY_NAME g_data_width
set_parameter_property g_data_width TYPE INTEGER
set_parameter_property g_data_width UNITS None
set_parameter_property g_data_width ALLOWED_RANGES -2147483648:2147483647
set_parameter_property g_data_width HDL_PARAMETER true
add_parameter g_addr_width INTEGER 16
set_parameter_property g_addr_width DEFAULT_VALUE 16
set_parameter_property g_addr_width DISPLAY_NAME g_addr_width
set_parameter_property g_addr_width TYPE INTEGER
set_parameter_property g_addr_width UNITS None
set_parameter_property g_addr_width ALLOWED_RANGES -2147483648:2147483647
set_parameter_property g_addr_width HDL_PARAMETER true


# 
# display items
# 


# 
# connection point clock
# 
add_interface clock clock end
set_interface_property clock clockRate 50000000
set_interface_property clock ENABLED true
set_interface_property clock EXPORT_OF ""
set_interface_property clock PORT_NAME_MAP ""
set_interface_property clock CMSIS_SVD_VARIABLES ""
set_interface_property clock SVD_ADDRESS_GROUP ""

add_interface_port clock csi_clk clk Input 1


# 
# connection point reset
# 
add_interface reset reset end
set_interface_property reset associatedClock clock
set_interface_property reset synchronousEdges DEASSERT
set_interface_property reset ENABLED true
set_interface_property reset EXPORT_OF ""
set_interface_property reset PORT_NAME_MAP ""
set_interface_property reset CMSIS_SVD_VARIABLES ""
set_interface_property reset SVD_ADDRESS_GROUP ""

add_interface_port reset rsi_reset_n reset_n Input 1


# 
# connection point avalon_slave
# 
add_interface avalon_slave avalon end
set_interface_property avalon_slave addressUnits WORDS
set_interface_property avalon_slave associatedClock clock
set_interface_property avalon_slave associatedReset reset
set_interface_property avalon_slave bitsPerSymbol 8
set_interface_property avalon_slave burstOnBurstBoundariesOnly false
set_interface_property avalon_slave burstcountUnits WORDS
set_interface_property avalon_slave explicitAddressSpan 0
set_interface_property avalon_slave holdTime 0
set_interface_property avalon_slave linewrapBursts false
set_interface_property avalon_slave maximumPendingReadTransactions 1
set_interface_property avalon_slave maximumPendingWriteTransactions 0
set_interface_property avalon_slave readLatency 0
set_interface_property avalon_slave readWaitTime 1
set_interface_property avalon_slave setupTime 0
set_interface_property avalon_slave timingUnits Cycles
set_interface_property avalon_slave writeWaitTime 0
set_interface_property avalon_slave ENABLED true
set_interface_property avalon_slave EXPORT_OF ""
set_interface_property avalon_slave PORT_NAME_MAP ""
set_interface_property avalon_slave CMSIS_SVD_VARIABLES ""
set_interface_property avalon_slave SVD_ADDRESS_GROUP ""

add_interface_port avalon_slave avs_write write Input 1
add_interface_port avalon_slave avs_writedata writedata Input g_data_width
add_interface_port avalon_slave avs_waitrequest waitrequest Output 1
add_interface_port avalon_slave avs_address address Input g_addr_width
add_interface_port avalon_slave avs_read read Input 1
add_interface_port avalon_slave avs_readdata readdata Output g_data_width
add_interface_port avalon_slave avs_readdatavalid readdatavalid Output 1
set_interface_assignment avalon_slave embeddedsw.configuration.isFlash 0
set_interface_assignment avalon_slave embeddedsw.configuration.isMemoryDevice 0
set_interface_assignment avalon_slave embeddedsw.configuration.isNonVolatileStorage 0
set_interface_assignment avalon_slave embeddedsw.configuration.isPrintableDevice 0


# 
# connection point conduit_end
# 
add_interface conduit_end conduit end
set_interface_property conduit_end associatedClock clock
set_interface_property conduit_end associatedReset reset
set_interface_property conduit_end ENABLED true
set_interface_property conduit_end EXPORT_OF ""
set_interface_property conduit_end PORT_NAME_MAP ""
set_interface_property conduit_end CMSIS_SVD_VARIABLES ""
set_interface_property conduit_end SVD_ADDRESS_GROUP ""

add_interface_port conduit_end coe_rd_data rd_data Input g_data_width
add_interface_port conduit_end coe_rd_data_valid rd_data_valid Input 1
add_interface_port conduit_end coe_addr addr Output g_addr_width
add_interface_port conduit_end coe_rd rd Output 1
add_interface_port conduit_end coe_wr wr Output 1
add_interface_port conduit_end coe_wr_data wr_data Output g_data_width

