
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

# ACDS 14.0 200 win32 2015.12.23.13:41:49

# ----------------------------------------
# Auto-generated simulation script

# ----------------------------------------
# Initialize variables
if ![info exists SYSTEM_INSTANCE_NAME] { 
  set SYSTEM_INSTANCE_NAME ""
} elseif { ![ string match "" $SYSTEM_INSTANCE_NAME ] } { 
  set SYSTEM_INSTANCE_NAME "/$SYSTEM_INSTANCE_NAME"
}

if ![info exists TOP_LEVEL_NAME] { 
  set TOP_LEVEL_NAME "fluid_board_soc"
}

if ![info exists QSYS_SIMDIR] { 
  set QSYS_SIMDIR "./../"
}

if ![info exists QUARTUS_INSTALL_DIR] { 
  set QUARTUS_INSTALL_DIR "C:/altera/14.0/quartus/"
}

# ----------------------------------------
# Initialize simulation properties - DO NOT MODIFY!
set ELAB_OPTIONS ""
set SIM_OPTIONS ""
if ![ string match "*-64 vsim*" [ vsim -version ] ] {
} else {
}

set Aldec "Riviera"
if { [ string match "*Active-HDL*" [ vsim -version ] ] } {
  set Aldec "Active"
}

if { [ string match "Active" $Aldec ] } {
  scripterconf -tcl
  createdesign "$TOP_LEVEL_NAME"  "."
  opendesign "$TOP_LEVEL_NAME"
}

# ----------------------------------------
# Copy ROM/RAM files to simulation directory
alias file_copy {
  echo "\[exec\] file_copy"
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_arm.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_cpu.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.dat ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_bht_ram.mif ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.dat ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ic_tag_ram.mif ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.dat ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_ociram_default_contents.mif ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.dat ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_a.mif ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.dat ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.hex ./
  file copy -force $QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_rf_ram_b.mif ./
}

# ----------------------------------------
# Create compilation libraries
proc ensure_lib { lib } { if ![file isdirectory $lib] { vlib $lib } }
ensure_lib      ./libraries     
ensure_lib      ./libraries/work
vmap       work ./libraries/work
ensure_lib                       ./libraries/altera_ver           
vmap       altera_ver            ./libraries/altera_ver           
ensure_lib                       ./libraries/lpm_ver              
vmap       lpm_ver               ./libraries/lpm_ver              
ensure_lib                       ./libraries/sgate_ver            
vmap       sgate_ver             ./libraries/sgate_ver            
ensure_lib                       ./libraries/altera_mf_ver        
vmap       altera_mf_ver         ./libraries/altera_mf_ver        
ensure_lib                       ./libraries/altera_lnsim_ver     
vmap       altera_lnsim_ver      ./libraries/altera_lnsim_ver     
ensure_lib                       ./libraries/cyclonev_ver         
vmap       cyclonev_ver          ./libraries/cyclonev_ver         
ensure_lib                       ./libraries/cyclonev_hssi_ver    
vmap       cyclonev_hssi_ver     ./libraries/cyclonev_hssi_ver    
ensure_lib                       ./libraries/cyclonev_pcie_hip_ver
vmap       cyclonev_pcie_hip_ver ./libraries/cyclonev_pcie_hip_ver
ensure_lib                       ./libraries/altera               
vmap       altera                ./libraries/altera               
ensure_lib                       ./libraries/lpm                  
vmap       lpm                   ./libraries/lpm                  
ensure_lib                       ./libraries/sgate                
vmap       sgate                 ./libraries/sgate                
ensure_lib                       ./libraries/altera_mf            
vmap       altera_mf             ./libraries/altera_mf            
ensure_lib                       ./libraries/altera_lnsim         
vmap       altera_lnsim          ./libraries/altera_lnsim         
ensure_lib                       ./libraries/cyclonev             
vmap       cyclonev              ./libraries/cyclonev             
ensure_lib                                                ./libraries/altera_common_sv_packages                     
vmap       altera_common_sv_packages                      ./libraries/altera_common_sv_packages                     
ensure_lib                                                ./libraries/h2t_timing_adapter                            
vmap       h2t_timing_adapter                             ./libraries/h2t_timing_adapter                            
ensure_lib                                                ./libraries/write_slave_to_source                         
vmap       write_slave_to_source                          ./libraries/write_slave_to_source                         
ensure_lib                                                ./libraries/export_slave                                  
vmap       export_slave                                   ./libraries/export_slave                                  
ensure_lib                                                ./libraries/border                                        
vmap       border                                         ./libraries/border                                        
ensure_lib                                                ./libraries/rsp_mux                                       
vmap       rsp_mux                                        ./libraries/rsp_mux                                       
ensure_lib                                                ./libraries/rsp_demux                                     
vmap       rsp_demux                                      ./libraries/rsp_demux                                     
ensure_lib                                                ./libraries/cmd_mux                                       
vmap       cmd_mux                                        ./libraries/cmd_mux                                       
ensure_lib                                                ./libraries/cmd_demux                                     
vmap       cmd_demux                                      ./libraries/cmd_demux                                     
ensure_lib                                                ./libraries/router_004                                    
vmap       router_004                                     ./libraries/router_004                                    
ensure_lib                                                ./libraries/router_002                                    
vmap       router_002                                     ./libraries/router_002                                    
ensure_lib                                                ./libraries/router                                        
vmap       router                                         ./libraries/router                                        
ensure_lib                                                ./libraries/onchip_memory_nios_arm_s2_agent_rsp_fifo      
vmap       onchip_memory_nios_arm_s2_agent_rsp_fifo       ./libraries/onchip_memory_nios_arm_s2_agent_rsp_fifo      
ensure_lib                                                ./libraries/sld_hub_controller_system_0_s0_agent_rsp_fifo 
vmap       sld_hub_controller_system_0_s0_agent_rsp_fifo  ./libraries/sld_hub_controller_system_0_s0_agent_rsp_fifo 
ensure_lib                                                ./libraries/onchip_memory_nios_arm_s1_rsp_width_adapter   
vmap       onchip_memory_nios_arm_s1_rsp_width_adapter    ./libraries/onchip_memory_nios_arm_s1_rsp_width_adapter   
ensure_lib                                                ./libraries/rsp_mux_003                                   
vmap       rsp_mux_003                                    ./libraries/rsp_mux_003                                   
ensure_lib                                                ./libraries/rsp_mux_002                                   
vmap       rsp_mux_002                                    ./libraries/rsp_mux_002                                   
ensure_lib                                                ./libraries/rsp_demux_006                                 
vmap       rsp_demux_006                                  ./libraries/rsp_demux_006                                 
ensure_lib                                                ./libraries/rsp_demux_003                                 
vmap       rsp_demux_003                                  ./libraries/rsp_demux_003                                 
ensure_lib                                                ./libraries/rsp_demux_002                                 
vmap       rsp_demux_002                                  ./libraries/rsp_demux_002                                 
ensure_lib                                                ./libraries/rsp_demux_001                                 
vmap       rsp_demux_001                                  ./libraries/rsp_demux_001                                 
ensure_lib                                                ./libraries/cmd_mux_006                                   
vmap       cmd_mux_006                                    ./libraries/cmd_mux_006                                   
ensure_lib                                                ./libraries/cmd_mux_003                                   
vmap       cmd_mux_003                                    ./libraries/cmd_mux_003                                   
ensure_lib                                                ./libraries/cmd_mux_002                                   
vmap       cmd_mux_002                                    ./libraries/cmd_mux_002                                   
ensure_lib                                                ./libraries/cmd_mux_001                                   
vmap       cmd_mux_001                                    ./libraries/cmd_mux_001                                   
ensure_lib                                                ./libraries/cmd_demux_003                                 
vmap       cmd_demux_003                                  ./libraries/cmd_demux_003                                 
ensure_lib                                                ./libraries/cmd_demux_002                                 
vmap       cmd_demux_002                                  ./libraries/cmd_demux_002                                 
ensure_lib                                                ./libraries/sysid_control_slave_burst_adapter             
vmap       sysid_control_slave_burst_adapter              ./libraries/sysid_control_slave_burst_adapter             
ensure_lib                                                ./libraries/hps_0_h2f_lw_axi_master_wr_limiter            
vmap       hps_0_h2f_lw_axi_master_wr_limiter             ./libraries/hps_0_h2f_lw_axi_master_wr_limiter            
ensure_lib                                                ./libraries/router_015                                    
vmap       router_015                                     ./libraries/router_015                                    
ensure_lib                                                ./libraries/router_013                                    
vmap       router_013                                     ./libraries/router_013                                    
ensure_lib                                                ./libraries/router_011                                    
vmap       router_011                                     ./libraries/router_011                                    
ensure_lib                                                ./libraries/router_010                                    
vmap       router_010                                     ./libraries/router_010                                    
ensure_lib                                                ./libraries/router_009                                    
vmap       router_009                                     ./libraries/router_009                                    
ensure_lib                                                ./libraries/router_008                                    
vmap       router_008                                     ./libraries/router_008                                    
ensure_lib                                                ./libraries/router_007                                    
vmap       router_007                                     ./libraries/router_007                                    
ensure_lib                                                ./libraries/router_006                                    
vmap       router_006                                     ./libraries/router_006                                    
ensure_lib                                                ./libraries/router_005                                    
vmap       router_005                                     ./libraries/router_005                                    
ensure_lib                                                ./libraries/router_003                                    
vmap       router_003                                     ./libraries/router_003                                    
ensure_lib                                                ./libraries/i2c_master_is1_avalon_slave_0_agent_rdata_fifo
vmap       i2c_master_is1_avalon_slave_0_agent_rdata_fifo ./libraries/i2c_master_is1_avalon_slave_0_agent_rdata_fifo
ensure_lib                                                ./libraries/i2c_master_is1_avalon_slave_0_agent_rsp_fifo  
vmap       i2c_master_is1_avalon_slave_0_agent_rsp_fifo   ./libraries/i2c_master_is1_avalon_slave_0_agent_rsp_fifo  
ensure_lib                                                ./libraries/onchip_memory_nios_arm_s1_agent_rdata_fifo    
vmap       onchip_memory_nios_arm_s1_agent_rdata_fifo     ./libraries/onchip_memory_nios_arm_s1_agent_rdata_fifo    
ensure_lib                                                ./libraries/onchip_memory_nios_arm_s1_agent_rsp_fifo      
vmap       onchip_memory_nios_arm_s1_agent_rsp_fifo       ./libraries/onchip_memory_nios_arm_s1_agent_rsp_fifo      
ensure_lib                                                ./libraries/sysid_control_slave_agent_rdata_fifo          
vmap       sysid_control_slave_agent_rdata_fifo           ./libraries/sysid_control_slave_agent_rdata_fifo          
ensure_lib                                                ./libraries/sysid_control_slave_agent_rsp_fifo            
vmap       sysid_control_slave_agent_rsp_fifo             ./libraries/sysid_control_slave_agent_rsp_fifo            
ensure_lib                                                ./libraries/sysid_control_slave_agent                     
vmap       sysid_control_slave_agent                      ./libraries/sysid_control_slave_agent                     
ensure_lib                                                ./libraries/fpga_only_master_master_agent                 
vmap       fpga_only_master_master_agent                  ./libraries/fpga_only_master_master_agent                 
ensure_lib                                                ./libraries/hps_0_h2f_lw_axi_master_agent                 
vmap       hps_0_h2f_lw_axi_master_agent                  ./libraries/hps_0_h2f_lw_axi_master_agent                 
ensure_lib                                                ./libraries/sysid_control_slave_translator                
vmap       sysid_control_slave_translator                 ./libraries/sysid_control_slave_translator                
ensure_lib                                                ./libraries/fpga_only_master_master_translator            
vmap       fpga_only_master_master_translator             ./libraries/fpga_only_master_master_translator            
ensure_lib                                                ./libraries/p2b_adapter                                   
vmap       p2b_adapter                                    ./libraries/p2b_adapter                                   
ensure_lib                                                ./libraries/b2p_adapter                                   
vmap       b2p_adapter                                    ./libraries/b2p_adapter                                   
ensure_lib                                                ./libraries/transacto                                     
vmap       transacto                                      ./libraries/transacto                                     
ensure_lib                                                ./libraries/p2b                                           
vmap       p2b                                            ./libraries/p2b                                           
ensure_lib                                                ./libraries/b2p                                           
vmap       b2p                                            ./libraries/b2p                                           
ensure_lib                                                ./libraries/fifo                                          
vmap       fifo                                           ./libraries/fifo                                          
ensure_lib                                                ./libraries/timing_adt                                    
vmap       timing_adt                                     ./libraries/timing_adt                                    
ensure_lib                                                ./libraries/jtag_phy_embedded_in_jtag_master              
vmap       jtag_phy_embedded_in_jtag_master               ./libraries/jtag_phy_embedded_in_jtag_master              
ensure_lib                                                ./libraries/avalon_st_adapter                             
vmap       avalon_st_adapter                              ./libraries/avalon_st_adapter                             
ensure_lib                                                ./libraries/hub_controller                                
vmap       hub_controller                                 ./libraries/hub_controller                                
ensure_lib                                                ./libraries/fabric                                        
vmap       fabric                                         ./libraries/fabric                                        
ensure_lib                                                ./libraries/link                                          
vmap       link                                           ./libraries/link                                          
ensure_lib                                                ./libraries/hps_io                                        
vmap       hps_io                                         ./libraries/hps_io                                        
ensure_lib                                                ./libraries/fpga_interfaces                               
vmap       fpga_interfaces                                ./libraries/fpga_interfaces                               
ensure_lib                                                ./libraries/rst_controller                                
vmap       rst_controller                                 ./libraries/rst_controller                                
ensure_lib                                                ./libraries/irq_mapper_002                                
vmap       irq_mapper_002                                 ./libraries/irq_mapper_002                                
ensure_lib                                                ./libraries/irq_mapper_001                                
vmap       irq_mapper_001                                 ./libraries/irq_mapper_001                                
ensure_lib                                                ./libraries/irq_mapper                                    
vmap       irq_mapper                                     ./libraries/irq_mapper                                    
ensure_lib                                                ./libraries/mm_interconnect_1                             
vmap       mm_interconnect_1                              ./libraries/mm_interconnect_1                             
ensure_lib                                                ./libraries/mm_interconnect_0                             
vmap       mm_interconnect_0                              ./libraries/mm_interconnect_0                             
ensure_lib                                                ./libraries/pio_reset_nios                                
vmap       pio_reset_nios                                 ./libraries/pio_reset_nios                                
ensure_lib                                                ./libraries/fpga_only_master                              
vmap       fpga_only_master                               ./libraries/fpga_only_master                              
ensure_lib                                                ./libraries/jtag_uart                                     
vmap       jtag_uart                                      ./libraries/jtag_uart                                     
ensure_lib                                                ./libraries/onchip_memory_nios_arm                        
vmap       onchip_memory_nios_arm                         ./libraries/onchip_memory_nios_arm                        
ensure_lib                                                ./libraries/pio_output                                    
vmap       pio_output                                     ./libraries/pio_output                                    
ensure_lib                                                ./libraries/pio_input                                     
vmap       pio_input                                      ./libraries/pio_input                                     
ensure_lib                                                ./libraries/onchip_memory_nios_cpu                        
vmap       onchip_memory_nios_cpu                         ./libraries/onchip_memory_nios_cpu                        
ensure_lib                                                ./libraries/i2c_master_is1                                
vmap       i2c_master_is1                                 ./libraries/i2c_master_is1                                
ensure_lib                                                ./libraries/avalon_spi_AMC7891_0                          
vmap       avalon_spi_AMC7891_0                           ./libraries/avalon_spi_AMC7891_0                          
ensure_lib                                                ./libraries/timer_0                                       
vmap       timer_0                                        ./libraries/timer_0                                       
ensure_lib                                                ./libraries/nios2_qsys_0                                  
vmap       nios2_qsys_0                                   ./libraries/nios2_qsys_0                                  
ensure_lib                                                ./libraries/sld_hub_controller_system_0                   
vmap       sld_hub_controller_system_0                    ./libraries/sld_hub_controller_system_0                   
ensure_lib                                                ./libraries/pll_0                                         
vmap       pll_0                                          ./libraries/pll_0                                         
ensure_lib                                                ./libraries/sysid                                         
vmap       sysid                                          ./libraries/sysid                                         
ensure_lib                                                ./libraries/hps_0                                         
vmap       hps_0                                          ./libraries/hps_0                                         

# ----------------------------------------
# Compile device library files
alias dev_com {
  echo "\[exec\] dev_com"
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives.v"                    -work altera_ver           
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/220model.v"                             -work lpm_ver              
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate.v"                                -work sgate_ver            
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf.v"                            -work altera_mf_ver        
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_lnsim.sv"                        -work altera_lnsim_ver     
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/aldec/cyclonev_atoms_ncrypt.v"          -work cyclonev_ver         
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/aldec/cyclonev_hmi_atoms_ncrypt.v"      -work cyclonev_ver         
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_atoms.v"                       -work cyclonev_ver         
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/aldec/cyclonev_hssi_atoms_ncrypt.v"     -work cyclonev_hssi_ver    
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_hssi_atoms.v"                  -work cyclonev_hssi_ver    
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/aldec/cyclonev_pcie_hip_atoms_ncrypt.v" -work cyclonev_pcie_hip_ver
  vlog  "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_pcie_hip_atoms.v"              -work cyclonev_pcie_hip_ver
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_syn_attributes.vhd"              -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_standard_functions.vhd"          -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/alt_dspbuilder_package.vhd"             -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_europa_support_lib.vhd"          -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives_components.vhd"       -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_primitives.vhd"                  -work altera               
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/220pack.vhd"                            -work lpm                  
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/220model.vhd"                           -work lpm                  
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate_pack.vhd"                         -work sgate                
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/sgate.vhd"                              -work sgate                
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf_components.vhd"               -work altera_mf            
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_mf.vhd"                          -work altera_mf            
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/altera_lnsim_components.vhd"            -work altera_lnsim         
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_atoms.vhd"                     -work cyclonev             
  vcom  "$QUARTUS_INSTALL_DIR/eda/sim_lib/cyclonev_components.vhd"                -work cyclonev             
}

# ----------------------------------------
# Compile the design files in correct order
alias com {
  echo "\[exec\] com"
  vlog  "$QSYS_SIMDIR/submodules/verbosity_pkg.sv"                                                                                                  -work altera_common_sv_packages                     
  vlog  "$QSYS_SIMDIR/submodules/avalon_utilities_pkg.sv"                                                                                           -work altera_common_sv_packages                     
  vlog  "$QSYS_SIMDIR/submodules/avalon_mm_pkg.sv"                                                                                                  -work altera_common_sv_packages                     
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_h2t_timing_adapter.sv"               -l altera_common_sv_packages -work h2t_timing_adapter                            
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_h2t_timing_adapter_fifo.sv"          -l altera_common_sv_packages -work h2t_timing_adapter                            
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_write_slave_to_source.v"                                          -work write_slave_to_source                         
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link_export_slave.vho"                                                 -work export_slave                                  
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_mm_slave_bfm.sv"                                                        -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_interrupt_sink.sv"                                                      -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_clock_source.sv"                                                        -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_reset_source.sv"                                                        -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border_memory.sv"                                        -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border_hps_io.sv"                                        -l altera_common_sv_packages -work border                                        
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io_border.sv"                                               -l altera_common_sv_packages -work border                                        
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_rsp_mux.vho"                                                                     -work rsp_mux                                       
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_rsp_demux.vho"                                                                   -work rsp_demux                                     
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_cmd_mux.vho"                                                                     -work cmd_mux                                       
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_cmd_demux.vho"                                                                   -work cmd_demux                                     
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router_004.sv"                                      -l altera_common_sv_packages -work router_004                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router_002.sv"                                      -l altera_common_sv_packages -work router_002                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_router.sv"                                          -l altera_common_sv_packages -work router                                        
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_onchip_memory_nios_arm_s2_agent_rsp_fifo.vho"                                    -work onchip_memory_nios_arm_s2_agent_rsp_fifo      
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1_sld_hub_controller_system_0_s0_agent_rsp_fifo.vho"                               -work sld_hub_controller_system_0_s0_agent_rsp_fifo 
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_width_adapter.sv"                                                       -l altera_common_sv_packages -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -l altera_common_sv_packages -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_burst_uncompressor.sv"                                                  -l altera_common_sv_packages -work onchip_memory_nios_arm_s1_rsp_width_adapter   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux_003.vho"                                                                 -work rsp_mux_003                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux_002.vho"                                                                 -work rsp_mux_002                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_mux.vho"                                                                     -work rsp_mux                                       
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_006.vho"                                                               -work rsp_demux_006                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_003.vho"                                                               -work rsp_demux_003                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_002.vho"                                                               -work rsp_demux_002                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux_001.vho"                                                               -work rsp_demux_001                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_rsp_demux.vho"                                                                   -work rsp_demux                                     
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_006.vho"                                                                 -work cmd_mux_006                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_003.vho"                                                                 -work cmd_mux_003                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_002.vho"                                                                 -work cmd_mux_002                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux_001.vho"                                                                 -work cmd_mux_001                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_mux.vho"                                                                     -work cmd_mux                                       
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux_003.vho"                                                               -work cmd_demux_003                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux_002.vho"                                                               -work cmd_demux_002                                 
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_cmd_demux.vho"                                                                   -work cmd_demux                                     
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter_13_1.sv"                                                  -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter.sv"                                                       -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_burst_adapter_new.sv"                                                   -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_wrap_burst_converter.sv"                                                       -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_incr_burst_converter.sv"                                                       -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_default_burst_converter.sv"                                                    -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_stage.sv"                                                   -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                     -l altera_common_sv_packages -work sysid_control_slave_burst_adapter             
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_traffic_limiter.sv"                                                     -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_reorder_memory.sv"                                                      -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_sc_fifo.v"                                                              -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                     -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_wr_limiter            
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_015.sv"                                      -l altera_common_sv_packages -work router_015                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_013.sv"                                      -l altera_common_sv_packages -work router_013                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_011.sv"                                      -l altera_common_sv_packages -work router_011                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_010.sv"                                      -l altera_common_sv_packages -work router_010                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_009.sv"                                      -l altera_common_sv_packages -work router_009                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_008.sv"                                      -l altera_common_sv_packages -work router_008                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_007.sv"                                      -l altera_common_sv_packages -work router_007                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_006.sv"                                      -l altera_common_sv_packages -work router_006                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_005.sv"                                      -l altera_common_sv_packages -work router_005                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_004.sv"                                      -l altera_common_sv_packages -work router_004                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_003.sv"                                      -l altera_common_sv_packages -work router_003                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router_002.sv"                                      -l altera_common_sv_packages -work router_002                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_router.sv"                                          -l altera_common_sv_packages -work router                                        
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_i2c_master_is1_avalon_slave_0_agent_rdata_fifo.vho"                              -work i2c_master_is1_avalon_slave_0_agent_rdata_fifo
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_i2c_master_is1_avalon_slave_0_agent_rsp_fifo.vho"                                -work i2c_master_is1_avalon_slave_0_agent_rsp_fifo  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_onchip_memory_nios_arm_s1_agent_rdata_fifo.vho"                                  -work onchip_memory_nios_arm_s1_agent_rdata_fifo    
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_onchip_memory_nios_arm_s1_agent_rsp_fifo.vho"                                    -work onchip_memory_nios_arm_s1_agent_rsp_fifo      
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_sysid_control_slave_agent_rdata_fifo.vho"                                        -work sysid_control_slave_agent_rdata_fifo          
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0_sysid_control_slave_agent_rsp_fifo.vho"                                          -work sysid_control_slave_agent_rsp_fifo            
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_slave_agent.sv"                                                         -l altera_common_sv_packages -work sysid_control_slave_agent                     
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_burst_uncompressor.sv"                                                  -l altera_common_sv_packages -work sysid_control_slave_agent                     
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_master_agent.sv"                                                        -l altera_common_sv_packages -work fpga_only_master_master_agent                 
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_axi_master_ni.sv"                                                       -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_agent                 
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_address_alignment.sv"                                                   -l altera_common_sv_packages -work hps_0_h2f_lw_axi_master_agent                 
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_slave_translator.sv"                                                    -l altera_common_sv_packages -work sysid_control_slave_translator                
  vlog  "$QSYS_SIMDIR/submodules/altera_merlin_master_translator.sv"                                                   -l altera_common_sv_packages -work fpga_only_master_master_translator            
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_p2b_adapter.sv"                                      -l altera_common_sv_packages -work p2b_adapter                                   
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_b2p_adapter.sv"                                      -l altera_common_sv_packages -work b2p_adapter                                   
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_transacto.vho"                                                                    -work transacto                                     
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_p2b.vho"                                                                          -work p2b                                           
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_b2p.vho"                                                                          -work b2p                                           
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_fifo.vho"                                                                         -work fifo                                          
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master_timing_adt.sv"                                       -l altera_common_sv_packages -work timing_adt                                    
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_jtag_interface.v"                                                                                 -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_jtag_dc_streaming.v"                                                                                        -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_jtag_sld_node.v"                                                                                            -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_jtag_streaming.v"                                                                                           -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_pli_streaming.v"                                                                                            -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_clock_crosser.v"                                                                                  -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_base.v"                                                                                  -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_idle_remover.v"                                                                                   -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_idle_inserter.v"                                                                                  -work jtag_phy_embedded_in_jtag_master              
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_st_pipeline_stage.sv"                                                   -l altera_common_sv_packages -work jtag_phy_embedded_in_jtag_master              
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_avalon_st_adapter.vhd"                                                 -work avalon_st_adapter                             
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_hub_controller.vhd"                                                    -work hub_controller                                
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_fabric.vhd"                                                            -work fabric                                        
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0_link.vhd"                                                              -work link                                          
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_hps_io.v"                                                                                    -work hps_io                                        
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_mm_slave_bfm.sv"                                                        -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/questa_mvc_svapi.svh"                                                                 -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/mgc_common_axi.sv"                                                                    -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/mgc_axi_master.sv"                                                                    -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/mgc_axi_slave.sv"                                                                     -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_interrupt_sink.sv"                                                      -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_clock_source.sv"                                                        -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/altera_avalon_reset_source.sv"                                                        -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_fpga_interfaces_uart1.sv"                                       -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0_fpga_interfaces.sv"                                             -l altera_common_sv_packages -work fpga_interfaces                               
  vlog  "$QSYS_SIMDIR/submodules/altera_reset_controller.v"                                                                                         -work rst_controller                                
  vlog  "$QSYS_SIMDIR/submodules/altera_reset_synchronizer.v"                                                                                       -work rst_controller                                
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper_002.vho"                                                                                -work irq_mapper_002                                
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper_001.vho"                                                                                -work irq_mapper_001                                
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_irq_mapper.vho"                                                                                    -work irq_mapper                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_1.v"                                                                               -work mm_interconnect_1                             
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_mm_interconnect_0.v"                                                                               -work mm_interconnect_0                             
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_reset_nios.v"                                                                                  -work pio_reset_nios                                
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_fpga_only_master.vhd"                                                                              -work fpga_only_master                              
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_jtag_uart.v"                                                                                       -work jtag_uart                                     
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_arm.v"                                                                          -work onchip_memory_nios_arm                        
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_output.v"                                                                                      -work pio_output                                    
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_pio_input.v"                                                                                       -work pio_input                                     
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_onchip_memory_nios_cpu.v"                                                                          -work onchip_memory_nios_cpu                        
  vlog  "$QSYS_SIMDIR/submodules/i2c_master.v"                                                                                                      -work i2c_master_is1                                
  vlog  "$QSYS_SIMDIR/submodules/i2c_master_bit_ctrl.v"                                                                                             -work i2c_master_is1                                
  vlog  "$QSYS_SIMDIR/submodules/i2c_master_byte_ctrl.v"                                                                                            -work i2c_master_is1                                
  vlog  "$QSYS_SIMDIR/submodules/i2c_master_defines.v"                                                                                              -work i2c_master_is1                                
  vlog  "$QSYS_SIMDIR/submodules/i2c_master_top.v"                                                                                                  -work i2c_master_is1                                
  vlog  "$QSYS_SIMDIR/submodules/timescale.v"                                                                                                       -work i2c_master_is1                                
  vcom  "$QSYS_SIMDIR/submodules/avalon_spi_AMC7891.vhd"                                                                                            -work avalon_spi_AMC7891_0                          
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_timer_0.v"                                                                                         -work timer_0                                       
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0.vho"                                                                                  -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_sysclk.vhd"                                                         -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_tck.vhd"                                                            -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_jtag_debug_module_wrapper.vhd"                                                        -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_mult_cell.vhd"                                                                        -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_oci_test_bench.vhd"                                                                   -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_nios2_qsys_0_test_bench.vhd"                                                                       -work nios2_qsys_0                                  
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sld_hub_controller_system_0.vhd"                                                                   -work sld_hub_controller_system_0                   
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_pll_0.vo"                                                                                          -work pll_0                                         
  vcom  "$QSYS_SIMDIR/submodules/fluid_board_soc_sysid.vho"                                                                                         -work sysid                                         
  vlog  "$QSYS_SIMDIR/submodules/fluid_board_soc_hps_0.v"                                                                                           -work hps_0                                         
  vcom  "$QSYS_SIMDIR/fluid_board_soc.vhd"                                                                                                                                                              
  vcom  "$QSYS_SIMDIR/fluid_board_soc_rst_controller.vhd"                                                                                                                                               
  vcom  "$QSYS_SIMDIR/fluid_board_soc_rst_controller_001.vhd"                                                                                                                                           
  vcom  "$QSYS_SIMDIR/fluid_board_soc_rst_controller_002.vhd"                                                                                                                                           
}

# ----------------------------------------
# Elaborate top level design
alias elab {
  echo "\[exec\] elab"
  eval vsim +access +r -t ps $ELAB_OPTIONS -L work -L altera_common_sv_packages -L h2t_timing_adapter -L write_slave_to_source -L export_slave -L border -L rsp_mux -L rsp_demux -L cmd_mux -L cmd_demux -L router_004 -L router_002 -L router -L onchip_memory_nios_arm_s2_agent_rsp_fifo -L sld_hub_controller_system_0_s0_agent_rsp_fifo -L onchip_memory_nios_arm_s1_rsp_width_adapter -L rsp_mux_003 -L rsp_mux_002 -L rsp_demux_006 -L rsp_demux_003 -L rsp_demux_002 -L rsp_demux_001 -L cmd_mux_006 -L cmd_mux_003 -L cmd_mux_002 -L cmd_mux_001 -L cmd_demux_003 -L cmd_demux_002 -L sysid_control_slave_burst_adapter -L hps_0_h2f_lw_axi_master_wr_limiter -L router_015 -L router_013 -L router_011 -L router_010 -L router_009 -L router_008 -L router_007 -L router_006 -L router_005 -L router_003 -L i2c_master_is1_avalon_slave_0_agent_rdata_fifo -L i2c_master_is1_avalon_slave_0_agent_rsp_fifo -L onchip_memory_nios_arm_s1_agent_rdata_fifo -L onchip_memory_nios_arm_s1_agent_rsp_fifo -L sysid_control_slave_agent_rdata_fifo -L sysid_control_slave_agent_rsp_fifo -L sysid_control_slave_agent -L fpga_only_master_master_agent -L hps_0_h2f_lw_axi_master_agent -L sysid_control_slave_translator -L fpga_only_master_master_translator -L p2b_adapter -L b2p_adapter -L transacto -L p2b -L b2p -L fifo -L timing_adt -L jtag_phy_embedded_in_jtag_master -L avalon_st_adapter -L hub_controller -L fabric -L link -L hps_io -L fpga_interfaces -L rst_controller -L irq_mapper_002 -L irq_mapper_001 -L irq_mapper -L mm_interconnect_1 -L mm_interconnect_0 -L pio_reset_nios -L fpga_only_master -L jtag_uart -L onchip_memory_nios_arm -L pio_output -L pio_input -L onchip_memory_nios_cpu -L i2c_master_is1 -L avalon_spi_AMC7891_0 -L timer_0 -L nios2_qsys_0 -L sld_hub_controller_system_0 -L pll_0 -L sysid -L hps_0 -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver -L altera -L lpm -L sgate -L altera_mf -L altera_lnsim -L cyclonev $TOP_LEVEL_NAME
}

# ----------------------------------------
# Elaborate the top level design with -dbg -O2 option
alias elab_debug {
  echo "\[exec\] elab_debug"
  eval vsim -dbg -O2 +access +r -t ps $ELAB_OPTIONS -L work -L altera_common_sv_packages -L h2t_timing_adapter -L write_slave_to_source -L export_slave -L border -L rsp_mux -L rsp_demux -L cmd_mux -L cmd_demux -L router_004 -L router_002 -L router -L onchip_memory_nios_arm_s2_agent_rsp_fifo -L sld_hub_controller_system_0_s0_agent_rsp_fifo -L onchip_memory_nios_arm_s1_rsp_width_adapter -L rsp_mux_003 -L rsp_mux_002 -L rsp_demux_006 -L rsp_demux_003 -L rsp_demux_002 -L rsp_demux_001 -L cmd_mux_006 -L cmd_mux_003 -L cmd_mux_002 -L cmd_mux_001 -L cmd_demux_003 -L cmd_demux_002 -L sysid_control_slave_burst_adapter -L hps_0_h2f_lw_axi_master_wr_limiter -L router_015 -L router_013 -L router_011 -L router_010 -L router_009 -L router_008 -L router_007 -L router_006 -L router_005 -L router_003 -L i2c_master_is1_avalon_slave_0_agent_rdata_fifo -L i2c_master_is1_avalon_slave_0_agent_rsp_fifo -L onchip_memory_nios_arm_s1_agent_rdata_fifo -L onchip_memory_nios_arm_s1_agent_rsp_fifo -L sysid_control_slave_agent_rdata_fifo -L sysid_control_slave_agent_rsp_fifo -L sysid_control_slave_agent -L fpga_only_master_master_agent -L hps_0_h2f_lw_axi_master_agent -L sysid_control_slave_translator -L fpga_only_master_master_translator -L p2b_adapter -L b2p_adapter -L transacto -L p2b -L b2p -L fifo -L timing_adt -L jtag_phy_embedded_in_jtag_master -L avalon_st_adapter -L hub_controller -L fabric -L link -L hps_io -L fpga_interfaces -L rst_controller -L irq_mapper_002 -L irq_mapper_001 -L irq_mapper -L mm_interconnect_1 -L mm_interconnect_0 -L pio_reset_nios -L fpga_only_master -L jtag_uart -L onchip_memory_nios_arm -L pio_output -L pio_input -L onchip_memory_nios_cpu -L i2c_master_is1 -L avalon_spi_AMC7891_0 -L timer_0 -L nios2_qsys_0 -L sld_hub_controller_system_0 -L pll_0 -L sysid -L hps_0 -L altera_ver -L lpm_ver -L sgate_ver -L altera_mf_ver -L altera_lnsim_ver -L cyclonev_ver -L cyclonev_hssi_ver -L cyclonev_pcie_hip_ver -L altera -L lpm -L sgate -L altera_mf -L altera_lnsim -L cyclonev $TOP_LEVEL_NAME
}

# ----------------------------------------
# Compile all the design files and elaborate the top level design
alias ld "
  dev_com
  com
  elab
"

# ----------------------------------------
# Compile all the design files and elaborate the top level design with -dbg -O2
alias ld_debug "
  dev_com
  com
  elab_debug
"

# ----------------------------------------
# Print out user commmand line aliases
alias h {
  echo "List Of Command Line Aliases"
  echo
  echo "file_copy                     -- Copy ROM/RAM files to simulation directory"
  echo
  echo "dev_com                       -- Compile device library files"
  echo
  echo "com                           -- Compile the design files in correct order"
  echo
  echo "elab                          -- Elaborate top level design"
  echo
  echo "elab_debug                    -- Elaborate the top level design with -dbg -O2 option"
  echo
  echo "ld                            -- Compile all the design files and elaborate the top level design"
  echo
  echo "ld_debug                      -- Compile all the design files and elaborate the top level design with -dbg -O2"
  echo
  echo 
  echo
  echo "List Of Variables"
  echo
  echo "TOP_LEVEL_NAME                -- Top level module name."
  echo
  echo "SYSTEM_INSTANCE_NAME          -- Instantiated system module name inside top level module."
  echo
  echo "QSYS_SIMDIR                   -- Qsys base simulation directory."
  echo
  echo "QUARTUS_INSTALL_DIR           -- Quartus installation directory."
}
file_copy
h
