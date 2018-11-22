
# (C) 2001-2015 Altera Corporation. All rights reserved.
# Your use of Altera Corporation's design tools, logic functions and 
# other software and tools, and its AMPP partner logic functions, and 
# any output files any of the foregoing (including device programming 
# or simulation files), and any associated documentation or information 
# are expressly subject to the terms and conditions of the Altera 
# Program License Subscription Agreement, Altera MegaCore Function 
# License Agreement, or other applicable license agreement, including, 
# without limitation, that your use is for the sole purpose of 
# programming logic devices manufactured by Altera and sold by Altera 
# or its authorized distributors. Please refer to the applicable 
# agreement for further details.

# ACDS 14.0 200 win32 2015.12.23.13:41:48

# ----------------------------------------
# vcsmx - auto-generated simulation script

# ----------------------------------------
# initialize variables
TOP_LEVEL_NAME="fluid_board_soc"
QSYS_SIMDIR="./../../"
QUARTUS_INSTALL_DIR="C:/altera/14.0/quartus/"
SKIP_FILE_COPY=0
SKIP_DEV_COM=0
SKIP_COM=0
SKIP_ELAB=0
SKIP_SIM=0
USER_DEFINED_ELAB_OPTIONS=""
USER_DEFINED_SIM_OPTIONS="+vcs+finish+100"

# ----------------------------------------
# overwrite variables - DO NOT MODIFY!
# This block evaluates each command line argument, typically used for 
# overwriting variables. An example usage:
#   sh <simulator>_setup.sh SKIP_ELAB=1 SKIP_SIM=1
for expression in "$@"; do
  eval $expression
  if [ $? -ne 0 ]; then
    echo "Error: This command line argument, \"$expression\", is/has an invalid expression." >&2
    exit $?
  fi
done

# ----------------------------------------
# initialize simulation properties - DO NOT MODIFY!
ELAB_OPTIONS=""
SIM_OPTIONS=""
if [[ `vcs -platform` != *"amd64"* ]]; then
  :
else
  :
fi

# ----------------------------------------
# create compilation libraries
mkdir -p ./libraries/work/
mkdir -p ./libraries/altera_common_sv_packages/
mkdir -p ./libraries/h2t_timing_adapter/
mkdir -p ./libraries/write_slave_to_source/
mkdir -p ./libraries/export_slave/
mkdir -p ./libraries/border/
mkdir -p ./libraries/rsp_mux/
mkdir -p ./libraries/rsp_demux/
mkdir -p ./libraries/cmd_mux/
mkdir -p ./libraries/cmd_demux/
mkdir -p ./libraries/router_004/
mkdir -p ./libraries/router_002/
mkdir -p ./libraries/router/
mkdir -p ./libraries/onchip_memory_nios_arm_s2_agent_rsp_fifo/
mkdir -p ./libraries/sld_hub_controller_system_0_s0_agent_rsp_fifo/
mkdir -p ./libraries/onchip_memory_nios_arm_s1_rsp_width_adapter/
mkdir -p ./libraries/rsp_mux_003/
mkdir -p ./libraries/rsp_mux_002/
mkdir -p ./libraries/rsp_demux_006/
mkdir -p ./libraries/rsp_demux_003/
mkdir -p ./libraries/rsp_demux_002/
mkdir -p ./libraries/rsp_demux_001/
mkdir -p ./libraries/cmd_mux_006/
mkdir -p ./libraries/cmd_mux_003/
mkdir -p ./libraries/cmd_mux_002/
mkdir -p ./libraries/cmd_mux_001/
mkdir -p ./libraries/cmd_demux_003/
mkdir -p ./libraries/cmd_demux_002/
mkdir -p ./libraries/sysid_control_slave_burst_adapter/
mkdir -p ./libraries/hps_0_h2f_lw_axi_master_wr_limiter/
mkdir -p ./libraries/router_015/
mkdir -p ./libraries/router_013/
mkdir -p ./libraries/router_011/
mkdir -p ./libraries/router_010/
mkdir -p ./libraries/router_009/
mkdir -p ./libraries/router_008/
mkdir -p ./libraries/router_007/
mkdir -p ./libraries/router_006/
mkdir -p ./libraries/router_005/
mkdir -p ./libraries/router_003/
mkdir -p ./libraries/i2c_master_is1_avalon_slave_0_agent_rdata_fifo/
mkdir -p ./libraries/i2c_master_is1_avalon_slave_0_agent_rsp_fifo/
mkdir -p ./libraries/onchip_memory_nios_arm_s1_agent_rdata_fifo/
mkdir -p ./libraries/onchip_memory_nios_arm_s1_agent_rsp_fifo/
mkdir -p ./libraries/sysid_control_slave_agent_rdata_fifo/
mkdir -p ./libraries/sysid_control_slave_agent_rsp_fifo/
mkdir -p ./libraries/sysid_control_slave_agent/
mkdir -p ./libraries/fpga_only_master_master_agent/
mkdir -p ./libraries/hps_0_h2f_lw_axi_master_agent/
mkdir -p ./libraries/sysid_control_slave_translator/
mkdir -p ./libraries/fpga_only_master_master_translator/
mkdir -p ./libraries/p2b_adapter/
mkdir -p ./libraries/b2p_adapter/
mkdir -p ./libraries/transacto/
mkdir -p ./libraries/p2b/
mkdir -p ./libraries/b2p/
mkdir -p ./libraries/fifo/
mkdir -p ./libraries/timing_adt/
mkdir -p ./libraries/jtag_phy_embedded_in_jtag_master/
mkdir -p ./libraries/avalon_st_adapter/
mkdir -p ./libraries/hub_controller/
mkdir -p ./libraries/fabric/
mkdir -p ./libraries/link/
mkdir -p ./libraries/hps_io/
mkdir -p ./libraries/fpga_interfaces/
mkdir -p ./libraries/rst_controller/
mkdir -p ./libraries/irq_mapper_002/
mkdir -p ./libraries/irq_mapper_001/
mkdir -p ./libraries/irq_mapper/
mkdir -p ./libraries/mm_interconnect_1/
mkdir -p ./libraries/mm_interconnect_0/
mkdir -p ./libraries/pio_reset_nios/
mkdir -p ./libraries/fpga_only_master/
mkdir -p ./libraries/jtag_uart/
mkdir -p ./libraries/onchip_memory_nios_arm/
mkdir -p ./libraries/pio_output/
mkdir -p ./libraries/pio_input/
mkdir -p ./libraries/onchip_memory_nios_cpu/
mkdir -p ./libraries/i2c_master_is1/
mkdir -p ./libraries/avalon_spi_AMC7891_0/
mkdir -p ./libraries/timer_0/
mkdir -p ./libraries/nios2_qsys_0/
mkdir -p ./libraries/sld_hub_controller_system_0/
mkdir -p ./libraries/pll_0/
mkdir -p ./libraries/sysid/
mkdir -p ./libraries/hps_0/
mkdir -p ./libraries/altera_ver/
mkdir -p ./libraries/lpm_ver/
mkdir -p ./libraries/sgate_ver/
mkdir -p ./libraries/altera_mf_ver/
mkdir -p ./libraries/altera_lnsim_ver/
mkdir -p ./libraries/cyclonev_ver/
mkdir -p ./libraries/cyclonev_hssi_ver/
mkdir -p ./libraries/cyclonev_pcie_hip_ver/
mkdir -p ./libraries/altera/
mkdir -p ./libraries/lpm/
mkdir -p ./libraries/sgate/
mkdir -p ./libraries/altera_mf/
mkdir -p ./libraries/altera_lnsim/
mkdir -p ./libraries/cyclonev/

# ----------------------------------------
# copy RAM/ROM files to simulation directory
if [ $SKIP_FILE_COPY -eq 0 ]; then
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_arm.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_cpu.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.dat ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.mif ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.dat ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.mif ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.dat ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.mif ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.dat ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.mif ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.dat ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.hex ./
  cp -f $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.mif ./
fi

# ----------------------------------------
# compile device library files
if [ $SKIP_DEV_COM -eq 0 ]; then
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives.v"                       -work altera_ver           
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/220model.v"                                -work lpm_ver              
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate.v"                                   -work sgate_ver            
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf.v"                               -work altera_mf_ver        
  vlogan +v2k -sverilog "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_lnsim.sv"                           -work altera_lnsim_ver     
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/synopsys/cyclonev_atoms_ncrypt.v"          -work cyclonev_ver         
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/synopsys/cyclonev_hmi_atoms_ncrypt.v"      -work cyclonev_ver         
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_atoms.v"                          -work cyclonev_ver         
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/synopsys/cyclonev_hssi_atoms_ncrypt.v"     -work cyclonev_hssi_ver    
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_hssi_atoms.v"                     -work cyclonev_hssi_ver    
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/synopsys/cyclonev_pcie_hip_atoms_ncrypt.v" -work cyclonev_pcie_hip_ver
  vlogan +v2k           "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_pcie_hip_atoms.v"                 -work cyclonev_pcie_hip_ver
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_syn_attributes.vhd"                 -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_standard_functions.vhd"             -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/alt_dspbuilder_package.vhd"                -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_europa_support_lib.vhd"             -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives_components.vhd"          -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives.vhd"                     -work altera               
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/220pack.vhd"                               -work lpm                  
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/220model.vhd"                              -work lpm                  
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate_pack.vhd"                            -work sgate                
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate.vhd"                                 -work sgate                
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf_components.vhd"                  -work altera_mf            
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf.vhd"                             -work altera_mf            
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_lnsim_components.vhd"               -work altera_lnsim         
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_atoms.vhd"                        -work cyclonev             
  vhdlan                "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_components.vhd"                   -work cyclonev             
fi

# ----------------------------------------
# compile design files in correct order
if [ $SKIP_COM -eq 0 ]; then
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/verbosity_pkg.sv"                                                                     -work altera_common_sv_packages                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/avalon_utilities_pkg.sv"                                                              -work altera_common_sv_packages                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/avalon_mm_pkg.sv"                                                                     -work altera_common_sv_packages                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_h2t_timing_adapter.sv"               -work h2t_timing_adapter                            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_h2t_timing_adapter_fifo.sv"          -work h2t_timing_adapter                            
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_write_slave_to_source.v"             -work write_slave_to_source                         
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_export_slave.vho"                    -work export_slave                                  
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_mm_slave_bfm.sv"                                                        -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_interrupt_sink.sv"                                                      -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_clock_source.sv"                                                        -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_reset_source.sv"                                                        -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border_memory.sv"                                        -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border_hps_io.sv"                                        -work border                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border.sv"                                               -work border                                        
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_rsp_mux.vho"                                        -work rsp_mux                                       
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_rsp_demux.vho"                                      -work rsp_demux                                     
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_cmd_mux.vho"                                        -work cmd_mux                                       
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_cmd_demux.vho"                                      -work cmd_demux                                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router_004.sv"                                      -work router_004                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router_002.sv"                                      -work router_002                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router.sv"                                          -work router                                        
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_onchip_memory_nios_arm_s2_agent_rsp_fifo.vho"       -work onchip_memory_nios_arm_s2_agent_rsp_fifo      
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_sld_hub_controller_system_0_s0_agent_rsp_fifo.vho"  -work sld_hub_controller_system_0_s0_agent_rsp_fifo 
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_width_adapter.sv"                                                       -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_burst_uncompressor.sv"                                                  -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux_003.vho"                                    -work rsp_mux_003                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux_002.vho"                                    -work rsp_mux_002                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux.vho"                                        -work rsp_mux                                       
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_006.vho"                                  -work rsp_demux_006                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_003.vho"                                  -work rsp_demux_003                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_002.vho"                                  -work rsp_demux_002                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_001.vho"                                  -work rsp_demux_001                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux.vho"                                      -work rsp_demux                                     
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_006.vho"                                    -work cmd_mux_006                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_003.vho"                                    -work cmd_mux_003                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_002.vho"                                    -work cmd_mux_002                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_001.vho"                                    -work cmd_mux_001                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux.vho"                                        -work cmd_mux                                       
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux_003.vho"                                  -work cmd_demux_003                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux_002.vho"                                  -work cmd_demux_002                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux.vho"                                      -work cmd_demux                                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter_13_1.sv"                                                  -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter.sv"                                                       -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter_new.sv"                                                   -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_wrap_burst_converter.sv"                                                       -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_incr_burst_converter.sv"                                                       -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_default_burst_converter.sv"                                                    -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_stage.sv"                                                   -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                     -work sysid_control_slave_burst_adapter             
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_traffic_limiter.sv"                                                     -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_reorder_memory.sv"                                                      -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_sc_fifo.v"                                                              -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                     -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_015.sv"                                      -work router_015                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_013.sv"                                      -work router_013                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_011.sv"                                      -work router_011                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_010.sv"                                      -work router_010                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_009.sv"                                      -work router_009                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_008.sv"                                      -work router_008                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_007.sv"                                      -work router_007                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_006.sv"                                      -work router_006                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_005.sv"                                      -work router_005                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_004.sv"                                      -work router_004                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_003.sv"                                      -work router_003                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_002.sv"                                      -work router_002                                    
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router.sv"                                          -work router                                        
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_i2c_master_is1_avalon_slave_0_agent_rdata_fifo.vho" -work i2c_master_is1_avalon_slave_0_agent_rdata_fifo
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_i2c_master_is1_avalon_slave_0_agent_rsp_fifo.vho"   -work i2c_master_is1_avalon_slave_0_agent_rsp_fifo  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_onchip_memory_nios_arm_s1_agent_rdata_fifo.vho"     -work onchip_memory_nios_arm_s1_agent_rdata_fifo    
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_onchip_memory_nios_arm_s1_agent_rsp_fifo.vho"       -work onchip_memory_nios_arm_s1_agent_rsp_fifo      
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_sysid_control_slave_agent_rdata_fifo.vho"           -work sysid_control_slave_agent_rdata_fifo          
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_sysid_control_slave_agent_rsp_fifo.vho"             -work sysid_control_slave_agent_rsp_fifo            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_slave_agent.sv"                                                         -work sysid_control_slave_agent                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_burst_uncompressor.sv"                                                  -work sysid_control_slave_agent                     
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_master_agent.sv"                                                        -work fpga_only_master_master_agent                 
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_axi_master_ni.sv"                                                       -work hps_0_h2f_lw_axi_master_agent                 
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -work hps_0_h2f_lw_axi_master_agent                 
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_slave_translator.sv"                                                    -work sysid_control_slave_translator                
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_merlin_master_translator.sv"                                                   -work fpga_only_master_master_translator            
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_p2b_adapter.sv"                                      -work p2b_adapter                                   
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_b2p_adapter.sv"                                      -work b2p_adapter                                   
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_transacto.vho"                                       -work transacto                                     
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_p2b.vho"                                             -work p2b                                           
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_b2p.vho"                                             -work b2p                                           
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_fifo.vho"                                            -work fifo                                          
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_timing_adt.sv"                                       -work timing_adt                                    
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_avalon_st_jtag_interface.v"                                                    -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_jtag_dc_streaming.v"                                                           -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_jtag_sld_node.v"                                                               -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_jtag_streaming.v"                                                              -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_pli_streaming.v"                                                               -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_avalon_st_clock_crosser.v"                                                     -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                     -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_avalon_st_idle_remover.v"                                                      -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_avalon_st_idle_inserter.v"                                                     -work jtag_phy_embedded_in_jtag_master              
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_stage.sv"                                                   -work jtag_phy_embedded_in_jtag_master              
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_avalon_st_adapter.vhd"                    -work avalon_st_adapter                             
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_hub_controller.vhd"                       -work hub_controller                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_fabric.vhd"                               -work fabric                                        
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link.vhd"                                 -work link                                          
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io.v"                                                       -work hps_io                                        
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_mm_slave_bfm.sv"                                                        -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/questa_mvc_svapi.svh"                                                                 -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/mgc_common_axi.sv"                                                                    -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/mgc_axi_master.sv"                                                                    -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/mgc_axi_slave.sv"                                                                     -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_interrupt_sink.sv"                                                      -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_clock_source.sv"                                                        -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/altera_avalon_reset_source.sv"                                                        -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_fpga_interfaces_uart1.sv"                                       -work fpga_interfaces                               
  vlogan +v2k -sverilog "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_fpga_interfaces.sv"                                             -work fpga_interfaces                               
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_reset_controller.v"                                                            -work rst_controller                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/altera_reset_synchronizer.v"                                                          -work rst_controller                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper_002.vho"                                                   -work irq_mapper_002                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper_001.vho"                                                   -work irq_mapper_001                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper.vho"                                                       -work irq_mapper                                    
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1.v"                                                  -work mm_interconnect_1                             
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0.v"                                                  -work mm_interconnect_0                             
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_reset_nios.v"                                                     -work pio_reset_nios                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master.vhd"                                                 -work fpga_only_master                              
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_jtag_uart.v"                                                          -work jtag_uart                                     
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_arm.v"                                             -work onchip_memory_nios_arm                        
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_output.v"                                                         -work pio_output                                    
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_input.v"                                                          -work pio_input                                     
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_cpu.v"                                             -work onchip_memory_nios_cpu                        
  vlogan +v2k           "$QSYS_SIMDIR/submodules/i2c_master.v"                                                                         -work i2c_master_is1                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/i2c_master_bit_ctrl.v"                                                                -work i2c_master_is1                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/i2c_master_byte_ctrl.v"                                                               -work i2c_master_is1                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/i2c_master_defines.v"                                                                 -work i2c_master_is1                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/i2c_master_top.v"                                                                     -work i2c_master_is1                                
  vlogan +v2k           "$QSYS_SIMDIR/submodules/timescale.v"                                                                          -work i2c_master_is1                                
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/avalon_spi_AMC7891.vhd"                                                               -work avalon_spi_AMC7891_0                          
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_timer_0.v"                                                            -work timer_0                                       
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0.vho"                                                     -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_sysclk.vhd"                            -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_tck.vhd"                               -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_wrapper.vhd"                           -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_mult_cell.vhd"                                           -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_oci_test_bench.vhd"                                      -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_test_bench.vhd"                                          -work nios2_qsys_0                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0.vhd"                                      -work sld_hub_controller_system_0                   
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_pll_0.vo"                                                             -work pll_0                                         
  vhdlan -xlrm          "$QSYS_SIMDIR/submodules/fluid_board_soc_sysid.vho"                                                            -work sysid                                         
  vlogan +v2k           "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0.v"                                                              -work hps_0                                         
  vhdlan -xlrm          "$QSYS_SIMDIR/fluid_board_soc.vhd"                                                                                                                                 
  vhdlan -xlrm          "$QSYS_SIMDIR/fluid_board_soc_rst_controller.vhd"                                                                                                                  
  vhdlan -xlrm          "$QSYS_SIMDIR/fluid_board_soc_rst_controller_001.vhd"                                                                                                              
  vhdlan -xlrm          "$QSYS_SIMDIR/fluid_board_soc_rst_controller_002.vhd"                                                                                                              
fi

# ----------------------------------------
# elaborate top level design
if [ $SKIP_ELAB -eq 0 ]; then
  vcs -lca -t ps $ELAB_OPTIONS $USER_DEFINED_ELAB_OPTIONS $TOP_LEVEL_NAME
fi

# ----------------------------------------
# simulate
if [ $SKIP_SIM -eq 0 ]; then
  ./simv $SIM_OPTIONS $USER_DEFINED_SIM_OPTIONS
fi
