// (C) 2001-2014 Altera Corporation. All rights reserved.
// Your use of Altera Corporation's design tools, logic functions and other 
// software and tools, and its AMPP partner logic functions, and any output 
// files any of the foregoing (including device programming or simulation 
// files), and any associated documentation or information are expressly subject 
// to the terms and conditions of the Altera Program License Subscription 
// Agreement, Altera MegaCore Function License Agreement, or other applicable 
// license agreement, including, without limitation, that your use is for the 
// sole purpose of programming logic devices manufactured by Altera and sold by 
// Altera or its authorized distributors.  Please refer to the applicable 
// agreement for further details.


// $Id: //acds/main/ip/sopc/components/verification/altera_tristate_conduit_bfm/altera_tristate_conduit_bfm.sv.terp#7 $
// $Revision: #7 $
// $Date: 2010/08/05 $
// $Author: klong $
//-----------------------------------------------------------------------------
// =head1 NAME
// altera_conduit_bfm
// =head1 SYNOPSIS
// Bus Functional Model (BFM) for a Standard Conduit BFM
//-----------------------------------------------------------------------------
// =head1 DESCRIPTION
// This is a Bus Functional Model (BFM) for a Standard Conduit Master.
// This BFM sampled the input/bidirection port value or driving user's value to 
// output ports when user call the API.  
// This BFM's HDL is been generated through terp file in Qsys/SOPC Builder.
// Generation parameters:
// output_name:                                       fluid_board_soc_hps_0_hps_io_border_hps_io
// role:width:direction:                              hps_io_emac0_inst_TX_CLK:1:output,hps_io_emac0_inst_TXD0:1:output,hps_io_emac0_inst_TXD1:1:output,hps_io_emac0_inst_TXD2:1:output,hps_io_emac0_inst_TXD3:1:output,hps_io_emac0_inst_RXD0:1:input,hps_io_emac0_inst_MDIO:1:bidir,hps_io_emac0_inst_MDC:1:output,hps_io_emac0_inst_RX_CTL:1:input,hps_io_emac0_inst_TX_CTL:1:output,hps_io_emac0_inst_RX_CLK:1:input,hps_io_emac0_inst_RXD1:1:input,hps_io_emac0_inst_RXD2:1:input,hps_io_emac0_inst_RXD3:1:input,hps_io_uart0_inst_RX:1:input,hps_io_uart0_inst_TX:1:output,hps_io_gpio_inst_GPIO26:1:bidir,hps_io_gpio_inst_GPIO28:1:bidir,hps_io_gpio_inst_GPIO59:1:bidir
// 0
//-----------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module fluid_board_soc_hps_0_hps_io_border_hps_io
(
   sig_hps_io_emac0_inst_TX_CLK,
   sig_hps_io_emac0_inst_TXD0,
   sig_hps_io_emac0_inst_TXD1,
   sig_hps_io_emac0_inst_TXD2,
   sig_hps_io_emac0_inst_TXD3,
   sig_hps_io_emac0_inst_RXD0,
   sig_hps_io_emac0_inst_MDIO,
   sig_hps_io_emac0_inst_MDC,
   sig_hps_io_emac0_inst_RX_CTL,
   sig_hps_io_emac0_inst_TX_CTL,
   sig_hps_io_emac0_inst_RX_CLK,
   sig_hps_io_emac0_inst_RXD1,
   sig_hps_io_emac0_inst_RXD2,
   sig_hps_io_emac0_inst_RXD3,
   sig_hps_io_uart0_inst_RX,
   sig_hps_io_uart0_inst_TX,
   sig_hps_io_gpio_inst_GPIO26,
   sig_hps_io_gpio_inst_GPIO28,
   sig_hps_io_gpio_inst_GPIO59
);

   //--------------------------------------------------------------------------
   // =head1 PINS 
   // =head2 User defined interface
   //--------------------------------------------------------------------------
   output sig_hps_io_emac0_inst_TX_CLK;
   output sig_hps_io_emac0_inst_TXD0;
   output sig_hps_io_emac0_inst_TXD1;
   output sig_hps_io_emac0_inst_TXD2;
   output sig_hps_io_emac0_inst_TXD3;
   input sig_hps_io_emac0_inst_RXD0;
   inout wire sig_hps_io_emac0_inst_MDIO;
   output sig_hps_io_emac0_inst_MDC;
   input sig_hps_io_emac0_inst_RX_CTL;
   output sig_hps_io_emac0_inst_TX_CTL;
   input sig_hps_io_emac0_inst_RX_CLK;
   input sig_hps_io_emac0_inst_RXD1;
   input sig_hps_io_emac0_inst_RXD2;
   input sig_hps_io_emac0_inst_RXD3;
   input sig_hps_io_uart0_inst_RX;
   output sig_hps_io_uart0_inst_TX;
   inout wire sig_hps_io_gpio_inst_GPIO26;
   inout wire sig_hps_io_gpio_inst_GPIO28;
   inout wire sig_hps_io_gpio_inst_GPIO59;

   // synthesis translate_off
   import verbosity_pkg::*;
   
   typedef logic ROLE_hps_io_emac0_inst_TX_CLK_t;
   typedef logic ROLE_hps_io_emac0_inst_TXD0_t;
   typedef logic ROLE_hps_io_emac0_inst_TXD1_t;
   typedef logic ROLE_hps_io_emac0_inst_TXD2_t;
   typedef logic ROLE_hps_io_emac0_inst_TXD3_t;
   typedef logic ROLE_hps_io_emac0_inst_RXD0_t;
   typedef logic ROLE_hps_io_emac0_inst_MDIO_t;
   typedef logic ROLE_hps_io_emac0_inst_MDC_t;
   typedef logic ROLE_hps_io_emac0_inst_RX_CTL_t;
   typedef logic ROLE_hps_io_emac0_inst_TX_CTL_t;
   typedef logic ROLE_hps_io_emac0_inst_RX_CLK_t;
   typedef logic ROLE_hps_io_emac0_inst_RXD1_t;
   typedef logic ROLE_hps_io_emac0_inst_RXD2_t;
   typedef logic ROLE_hps_io_emac0_inst_RXD3_t;
   typedef logic ROLE_hps_io_uart0_inst_RX_t;
   typedef logic ROLE_hps_io_uart0_inst_TX_t;
   typedef logic ROLE_hps_io_gpio_inst_GPIO26_t;
   typedef logic ROLE_hps_io_gpio_inst_GPIO28_t;
   typedef logic ROLE_hps_io_gpio_inst_GPIO59_t;

   reg hps_io_emac0_inst_TX_CLK_temp;
   reg hps_io_emac0_inst_TX_CLK_out;
   reg hps_io_emac0_inst_TXD0_temp;
   reg hps_io_emac0_inst_TXD0_out;
   reg hps_io_emac0_inst_TXD1_temp;
   reg hps_io_emac0_inst_TXD1_out;
   reg hps_io_emac0_inst_TXD2_temp;
   reg hps_io_emac0_inst_TXD2_out;
   reg hps_io_emac0_inst_TXD3_temp;
   reg hps_io_emac0_inst_TXD3_out;
   logic [0 : 0] hps_io_emac0_inst_RXD0_in;
   logic [0 : 0] hps_io_emac0_inst_RXD0_local;
   logic hps_io_emac0_inst_MDIO_oe;
   logic hps_io_emac0_inst_MDIO_oe_temp = 0;
   reg hps_io_emac0_inst_MDIO_temp;
   reg hps_io_emac0_inst_MDIO_out;
   logic [0 : 0] hps_io_emac0_inst_MDIO_in;
   logic [0 : 0] hps_io_emac0_inst_MDIO_local;
   reg hps_io_emac0_inst_MDC_temp;
   reg hps_io_emac0_inst_MDC_out;
   logic [0 : 0] hps_io_emac0_inst_RX_CTL_in;
   logic [0 : 0] hps_io_emac0_inst_RX_CTL_local;
   reg hps_io_emac0_inst_TX_CTL_temp;
   reg hps_io_emac0_inst_TX_CTL_out;
   logic [0 : 0] hps_io_emac0_inst_RX_CLK_in;
   logic [0 : 0] hps_io_emac0_inst_RX_CLK_local;
   logic [0 : 0] hps_io_emac0_inst_RXD1_in;
   logic [0 : 0] hps_io_emac0_inst_RXD1_local;
   logic [0 : 0] hps_io_emac0_inst_RXD2_in;
   logic [0 : 0] hps_io_emac0_inst_RXD2_local;
   logic [0 : 0] hps_io_emac0_inst_RXD3_in;
   logic [0 : 0] hps_io_emac0_inst_RXD3_local;
   logic [0 : 0] hps_io_uart0_inst_RX_in;
   logic [0 : 0] hps_io_uart0_inst_RX_local;
   reg hps_io_uart0_inst_TX_temp;
   reg hps_io_uart0_inst_TX_out;
   logic hps_io_gpio_inst_GPIO26_oe;
   logic hps_io_gpio_inst_GPIO26_oe_temp = 0;
   reg hps_io_gpio_inst_GPIO26_temp;
   reg hps_io_gpio_inst_GPIO26_out;
   logic [0 : 0] hps_io_gpio_inst_GPIO26_in;
   logic [0 : 0] hps_io_gpio_inst_GPIO26_local;
   logic hps_io_gpio_inst_GPIO28_oe;
   logic hps_io_gpio_inst_GPIO28_oe_temp = 0;
   reg hps_io_gpio_inst_GPIO28_temp;
   reg hps_io_gpio_inst_GPIO28_out;
   logic [0 : 0] hps_io_gpio_inst_GPIO28_in;
   logic [0 : 0] hps_io_gpio_inst_GPIO28_local;
   logic hps_io_gpio_inst_GPIO59_oe;
   logic hps_io_gpio_inst_GPIO59_oe_temp = 0;
   reg hps_io_gpio_inst_GPIO59_temp;
   reg hps_io_gpio_inst_GPIO59_out;
   logic [0 : 0] hps_io_gpio_inst_GPIO59_in;
   logic [0 : 0] hps_io_gpio_inst_GPIO59_local;

   //--------------------------------------------------------------------------
   // =head1 Public Methods API
   // =pod
   // This section describes the public methods in the application programming
   // interface (API). The application program interface provides methods for 
   // a testbench which instantiates, controls and queries state in this BFM 
   // component. Test programs must only use these public access methods and 
   // events to communicate with this BFM component. The API and module pins
   // are the only interfaces of this component that are guaranteed to be
   // stable. The API will be maintained for the life of the product. 
   // While we cannot prevent a test program from directly accessing internal
   // tasks, functions, or data private to the BFM, there is no guarantee that
   // these will be present in the future. In fact, it is best for the user
   // to assume that the underlying implementation of this component can 
   // and will change.
   // =cut
   //--------------------------------------------------------------------------
   
   event signal_input_hps_io_emac0_inst_RXD0_change;
   event signal_input_hps_io_emac0_inst_MDIO_change;
   event signal_input_hps_io_emac0_inst_RX_CTL_change;
   event signal_input_hps_io_emac0_inst_RX_CLK_change;
   event signal_input_hps_io_emac0_inst_RXD1_change;
   event signal_input_hps_io_emac0_inst_RXD2_change;
   event signal_input_hps_io_emac0_inst_RXD3_change;
   event signal_input_hps_io_uart0_inst_RX_change;
   event signal_input_hps_io_gpio_inst_GPIO26_change;
   event signal_input_hps_io_gpio_inst_GPIO28_change;
   event signal_input_hps_io_gpio_inst_GPIO59_change;
   
   function automatic string get_version();  // public
      // Return BFM version string. For example, version 9.1 sp1 is "9.1sp1" 
      string ret_version = "14.0";
      return ret_version;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TX_CLK
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TX_CLK (
      ROLE_hps_io_emac0_inst_TX_CLK_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TX_CLK.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TX_CLK_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TXD0
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TXD0 (
      ROLE_hps_io_emac0_inst_TXD0_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TXD0.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TXD0_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TXD1
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TXD1 (
      ROLE_hps_io_emac0_inst_TXD1_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TXD1.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TXD1_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TXD2
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TXD2 (
      ROLE_hps_io_emac0_inst_TXD2_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TXD2.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TXD2_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TXD3
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TXD3 (
      ROLE_hps_io_emac0_inst_TXD3_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TXD3.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TXD3_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RXD0
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RXD0_t get_hps_io_emac0_inst_RXD0();
   
      // Gets the hps_io_emac0_inst_RXD0 input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RXD0");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RXD0_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_MDIO
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_MDIO_t get_hps_io_emac0_inst_MDIO();
   
      // Gets the hps_io_emac0_inst_MDIO input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_MDIO");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_MDIO_in;
      
   endfunction

   function automatic void set_hps_io_emac0_inst_MDIO (
      ROLE_hps_io_emac0_inst_MDIO_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_MDIO.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_MDIO_temp = new_value;
   endfunction
   
   function automatic void set_hps_io_emac0_inst_MDIO_oe (
      bit enable
   );
      // bidir port hps_io_emac0_inst_MDIO will work as output port when set to 1.
      // bidir port hps_io_emac0_inst_MDIO will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_MDIO_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_MDC
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_MDC (
      ROLE_hps_io_emac0_inst_MDC_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_MDC.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_MDC_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RX_CTL
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RX_CTL_t get_hps_io_emac0_inst_RX_CTL();
   
      // Gets the hps_io_emac0_inst_RX_CTL input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RX_CTL");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RX_CTL_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_TX_CTL
   // -------------------------------------------------------

   function automatic void set_hps_io_emac0_inst_TX_CTL (
      ROLE_hps_io_emac0_inst_TX_CTL_t new_value
   );
      // Drive the new value to hps_io_emac0_inst_TX_CTL.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_emac0_inst_TX_CTL_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RX_CLK
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RX_CLK_t get_hps_io_emac0_inst_RX_CLK();
   
      // Gets the hps_io_emac0_inst_RX_CLK input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RX_CLK");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RX_CLK_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RXD1
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RXD1_t get_hps_io_emac0_inst_RXD1();
   
      // Gets the hps_io_emac0_inst_RXD1 input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RXD1");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RXD1_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RXD2
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RXD2_t get_hps_io_emac0_inst_RXD2();
   
      // Gets the hps_io_emac0_inst_RXD2 input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RXD2");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RXD2_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_emac0_inst_RXD3
   // -------------------------------------------------------
   function automatic ROLE_hps_io_emac0_inst_RXD3_t get_hps_io_emac0_inst_RXD3();
   
      // Gets the hps_io_emac0_inst_RXD3 input value.
      $sformat(message, "%m: called get_hps_io_emac0_inst_RXD3");
      print(VERBOSITY_DEBUG, message);
      return hps_io_emac0_inst_RXD3_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_uart0_inst_RX
   // -------------------------------------------------------
   function automatic ROLE_hps_io_uart0_inst_RX_t get_hps_io_uart0_inst_RX();
   
      // Gets the hps_io_uart0_inst_RX input value.
      $sformat(message, "%m: called get_hps_io_uart0_inst_RX");
      print(VERBOSITY_DEBUG, message);
      return hps_io_uart0_inst_RX_in;
      
   endfunction

   // -------------------------------------------------------
   // hps_io_uart0_inst_TX
   // -------------------------------------------------------

   function automatic void set_hps_io_uart0_inst_TX (
      ROLE_hps_io_uart0_inst_TX_t new_value
   );
      // Drive the new value to hps_io_uart0_inst_TX.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_uart0_inst_TX_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // hps_io_gpio_inst_GPIO26
   // -------------------------------------------------------
   function automatic ROLE_hps_io_gpio_inst_GPIO26_t get_hps_io_gpio_inst_GPIO26();
   
      // Gets the hps_io_gpio_inst_GPIO26 input value.
      $sformat(message, "%m: called get_hps_io_gpio_inst_GPIO26");
      print(VERBOSITY_DEBUG, message);
      return hps_io_gpio_inst_GPIO26_in;
      
   endfunction

   function automatic void set_hps_io_gpio_inst_GPIO26 (
      ROLE_hps_io_gpio_inst_GPIO26_t new_value
   );
      // Drive the new value to hps_io_gpio_inst_GPIO26.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO26_temp = new_value;
   endfunction
   
   function automatic void set_hps_io_gpio_inst_GPIO26_oe (
      bit enable
   );
      // bidir port hps_io_gpio_inst_GPIO26 will work as output port when set to 1.
      // bidir port hps_io_gpio_inst_GPIO26 will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO26_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // hps_io_gpio_inst_GPIO28
   // -------------------------------------------------------
   function automatic ROLE_hps_io_gpio_inst_GPIO28_t get_hps_io_gpio_inst_GPIO28();
   
      // Gets the hps_io_gpio_inst_GPIO28 input value.
      $sformat(message, "%m: called get_hps_io_gpio_inst_GPIO28");
      print(VERBOSITY_DEBUG, message);
      return hps_io_gpio_inst_GPIO28_in;
      
   endfunction

   function automatic void set_hps_io_gpio_inst_GPIO28 (
      ROLE_hps_io_gpio_inst_GPIO28_t new_value
   );
      // Drive the new value to hps_io_gpio_inst_GPIO28.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO28_temp = new_value;
   endfunction
   
   function automatic void set_hps_io_gpio_inst_GPIO28_oe (
      bit enable
   );
      // bidir port hps_io_gpio_inst_GPIO28 will work as output port when set to 1.
      // bidir port hps_io_gpio_inst_GPIO28 will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO28_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // hps_io_gpio_inst_GPIO59
   // -------------------------------------------------------
   function automatic ROLE_hps_io_gpio_inst_GPIO59_t get_hps_io_gpio_inst_GPIO59();
   
      // Gets the hps_io_gpio_inst_GPIO59 input value.
      $sformat(message, "%m: called get_hps_io_gpio_inst_GPIO59");
      print(VERBOSITY_DEBUG, message);
      return hps_io_gpio_inst_GPIO59_in;
      
   endfunction

   function automatic void set_hps_io_gpio_inst_GPIO59 (
      ROLE_hps_io_gpio_inst_GPIO59_t new_value
   );
      // Drive the new value to hps_io_gpio_inst_GPIO59.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO59_temp = new_value;
   endfunction
   
   function automatic void set_hps_io_gpio_inst_GPIO59_oe (
      bit enable
   );
      // bidir port hps_io_gpio_inst_GPIO59 will work as output port when set to 1.
      // bidir port hps_io_gpio_inst_GPIO59 will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      hps_io_gpio_inst_GPIO59_oe_temp = enable;
   endfunction

   assign sig_hps_io_emac0_inst_TX_CLK = hps_io_emac0_inst_TX_CLK_temp;
   assign sig_hps_io_emac0_inst_TXD0 = hps_io_emac0_inst_TXD0_temp;
   assign sig_hps_io_emac0_inst_TXD1 = hps_io_emac0_inst_TXD1_temp;
   assign sig_hps_io_emac0_inst_TXD2 = hps_io_emac0_inst_TXD2_temp;
   assign sig_hps_io_emac0_inst_TXD3 = hps_io_emac0_inst_TXD3_temp;
   assign hps_io_emac0_inst_RXD0_in = sig_hps_io_emac0_inst_RXD0;
   assign hps_io_emac0_inst_MDIO_oe = hps_io_emac0_inst_MDIO_oe_temp;
   assign sig_hps_io_emac0_inst_MDIO = (hps_io_emac0_inst_MDIO_oe == 1)? hps_io_emac0_inst_MDIO_temp:'z;
   assign hps_io_emac0_inst_MDIO_in = (hps_io_emac0_inst_MDIO_oe == 0)? sig_hps_io_emac0_inst_MDIO:'z;
   assign sig_hps_io_emac0_inst_MDC = hps_io_emac0_inst_MDC_temp;
   assign hps_io_emac0_inst_RX_CTL_in = sig_hps_io_emac0_inst_RX_CTL;
   assign sig_hps_io_emac0_inst_TX_CTL = hps_io_emac0_inst_TX_CTL_temp;
   assign hps_io_emac0_inst_RX_CLK_in = sig_hps_io_emac0_inst_RX_CLK;
   assign hps_io_emac0_inst_RXD1_in = sig_hps_io_emac0_inst_RXD1;
   assign hps_io_emac0_inst_RXD2_in = sig_hps_io_emac0_inst_RXD2;
   assign hps_io_emac0_inst_RXD3_in = sig_hps_io_emac0_inst_RXD3;
   assign hps_io_uart0_inst_RX_in = sig_hps_io_uart0_inst_RX;
   assign sig_hps_io_uart0_inst_TX = hps_io_uart0_inst_TX_temp;
   assign hps_io_gpio_inst_GPIO26_oe = hps_io_gpio_inst_GPIO26_oe_temp;
   assign sig_hps_io_gpio_inst_GPIO26 = (hps_io_gpio_inst_GPIO26_oe == 1)? hps_io_gpio_inst_GPIO26_temp:'z;
   assign hps_io_gpio_inst_GPIO26_in = (hps_io_gpio_inst_GPIO26_oe == 0)? sig_hps_io_gpio_inst_GPIO26:'z;
   assign hps_io_gpio_inst_GPIO28_oe = hps_io_gpio_inst_GPIO28_oe_temp;
   assign sig_hps_io_gpio_inst_GPIO28 = (hps_io_gpio_inst_GPIO28_oe == 1)? hps_io_gpio_inst_GPIO28_temp:'z;
   assign hps_io_gpio_inst_GPIO28_in = (hps_io_gpio_inst_GPIO28_oe == 0)? sig_hps_io_gpio_inst_GPIO28:'z;
   assign hps_io_gpio_inst_GPIO59_oe = hps_io_gpio_inst_GPIO59_oe_temp;
   assign sig_hps_io_gpio_inst_GPIO59 = (hps_io_gpio_inst_GPIO59_oe == 1)? hps_io_gpio_inst_GPIO59_temp:'z;
   assign hps_io_gpio_inst_GPIO59_in = (hps_io_gpio_inst_GPIO59_oe == 0)? sig_hps_io_gpio_inst_GPIO59:'z;


   always @(hps_io_emac0_inst_RXD0_in) begin
      if (hps_io_emac0_inst_RXD0_local != hps_io_emac0_inst_RXD0_in)
         -> signal_input_hps_io_emac0_inst_RXD0_change;
      hps_io_emac0_inst_RXD0_local = hps_io_emac0_inst_RXD0_in;
   end
   
   always @(hps_io_emac0_inst_MDIO_in) begin
      if (hps_io_emac0_inst_MDIO_oe == 0) begin
         if (hps_io_emac0_inst_MDIO_local != hps_io_emac0_inst_MDIO_in)
            -> signal_input_hps_io_emac0_inst_MDIO_change;
         hps_io_emac0_inst_MDIO_local = hps_io_emac0_inst_MDIO_in;
      end
   end
   
   always @(hps_io_emac0_inst_RX_CTL_in) begin
      if (hps_io_emac0_inst_RX_CTL_local != hps_io_emac0_inst_RX_CTL_in)
         -> signal_input_hps_io_emac0_inst_RX_CTL_change;
      hps_io_emac0_inst_RX_CTL_local = hps_io_emac0_inst_RX_CTL_in;
   end
   
   always @(hps_io_emac0_inst_RX_CLK_in) begin
      if (hps_io_emac0_inst_RX_CLK_local != hps_io_emac0_inst_RX_CLK_in)
         -> signal_input_hps_io_emac0_inst_RX_CLK_change;
      hps_io_emac0_inst_RX_CLK_local = hps_io_emac0_inst_RX_CLK_in;
   end
   
   always @(hps_io_emac0_inst_RXD1_in) begin
      if (hps_io_emac0_inst_RXD1_local != hps_io_emac0_inst_RXD1_in)
         -> signal_input_hps_io_emac0_inst_RXD1_change;
      hps_io_emac0_inst_RXD1_local = hps_io_emac0_inst_RXD1_in;
   end
   
   always @(hps_io_emac0_inst_RXD2_in) begin
      if (hps_io_emac0_inst_RXD2_local != hps_io_emac0_inst_RXD2_in)
         -> signal_input_hps_io_emac0_inst_RXD2_change;
      hps_io_emac0_inst_RXD2_local = hps_io_emac0_inst_RXD2_in;
   end
   
   always @(hps_io_emac0_inst_RXD3_in) begin
      if (hps_io_emac0_inst_RXD3_local != hps_io_emac0_inst_RXD3_in)
         -> signal_input_hps_io_emac0_inst_RXD3_change;
      hps_io_emac0_inst_RXD3_local = hps_io_emac0_inst_RXD3_in;
   end
   
   always @(hps_io_uart0_inst_RX_in) begin
      if (hps_io_uart0_inst_RX_local != hps_io_uart0_inst_RX_in)
         -> signal_input_hps_io_uart0_inst_RX_change;
      hps_io_uart0_inst_RX_local = hps_io_uart0_inst_RX_in;
   end
   
   always @(hps_io_gpio_inst_GPIO26_in) begin
      if (hps_io_gpio_inst_GPIO26_oe == 0) begin
         if (hps_io_gpio_inst_GPIO26_local != hps_io_gpio_inst_GPIO26_in)
            -> signal_input_hps_io_gpio_inst_GPIO26_change;
         hps_io_gpio_inst_GPIO26_local = hps_io_gpio_inst_GPIO26_in;
      end
   end
   
   always @(hps_io_gpio_inst_GPIO28_in) begin
      if (hps_io_gpio_inst_GPIO28_oe == 0) begin
         if (hps_io_gpio_inst_GPIO28_local != hps_io_gpio_inst_GPIO28_in)
            -> signal_input_hps_io_gpio_inst_GPIO28_change;
         hps_io_gpio_inst_GPIO28_local = hps_io_gpio_inst_GPIO28_in;
      end
   end
   
   always @(hps_io_gpio_inst_GPIO59_in) begin
      if (hps_io_gpio_inst_GPIO59_oe == 0) begin
         if (hps_io_gpio_inst_GPIO59_local != hps_io_gpio_inst_GPIO59_in)
            -> signal_input_hps_io_gpio_inst_GPIO59_change;
         hps_io_gpio_inst_GPIO59_local = hps_io_gpio_inst_GPIO59_in;
      end
   end
   


// synthesis translate_on

endmodule

