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
// output_name:                                       fluid_board_soc_hps_0_hps_io_border_memory
// role:width:direction:                              mem_a:15:Output,mem_ba:3:Output,mem_ck:1:Output,mem_ck_n:1:Output,mem_cke:1:Output,mem_cs_n:1:Output,mem_ras_n:1:Output,mem_cas_n:1:Output,mem_we_n:1:Output,mem_reset_n:1:Output,mem_dq:32:Bidir,mem_dqs:4:Bidir,mem_dqs_n:4:Bidir,mem_odt:1:Output,mem_dm:4:Output,oct_rzqin:1:Input
// 0
//-----------------------------------------------------------------------------
`timescale 1 ps / 1 ps

module fluid_board_soc_hps_0_hps_io_border_memory
(
   sig_mem_a,
   sig_mem_ba,
   sig_mem_ck,
   sig_mem_ck_n,
   sig_mem_cke,
   sig_mem_cs_n,
   sig_mem_ras_n,
   sig_mem_cas_n,
   sig_mem_we_n,
   sig_mem_reset_n,
   sig_mem_dq,
   sig_mem_dqs,
   sig_mem_dqs_n,
   sig_mem_odt,
   sig_mem_dm,
   sig_oct_rzqin
);

   //--------------------------------------------------------------------------
   // =head1 PINS 
   // =head2 User defined interface
   //--------------------------------------------------------------------------
   output [14 : 0] sig_mem_a;
   output [2 : 0] sig_mem_ba;
   output sig_mem_ck;
   output sig_mem_ck_n;
   output sig_mem_cke;
   output sig_mem_cs_n;
   output sig_mem_ras_n;
   output sig_mem_cas_n;
   output sig_mem_we_n;
   output sig_mem_reset_n;
   inout wire [31 : 0] sig_mem_dq;
   inout wire [3 : 0] sig_mem_dqs;
   inout wire [3 : 0] sig_mem_dqs_n;
   output sig_mem_odt;
   output [3 : 0] sig_mem_dm;
   input sig_oct_rzqin;

   // synthesis translate_off
   import verbosity_pkg::*;
   
   typedef logic [14 : 0] ROLE_mem_a_t;
   typedef logic [2 : 0] ROLE_mem_ba_t;
   typedef logic ROLE_mem_ck_t;
   typedef logic ROLE_mem_ck_n_t;
   typedef logic ROLE_mem_cke_t;
   typedef logic ROLE_mem_cs_n_t;
   typedef logic ROLE_mem_ras_n_t;
   typedef logic ROLE_mem_cas_n_t;
   typedef logic ROLE_mem_we_n_t;
   typedef logic ROLE_mem_reset_n_t;
   typedef logic [31 : 0] ROLE_mem_dq_t;
   typedef logic [3 : 0] ROLE_mem_dqs_t;
   typedef logic [3 : 0] ROLE_mem_dqs_n_t;
   typedef logic ROLE_mem_odt_t;
   typedef logic [3 : 0] ROLE_mem_dm_t;
   typedef logic ROLE_oct_rzqin_t;

   reg [14 : 0] mem_a_temp;
   reg [14 : 0] mem_a_out;
   reg [2 : 0] mem_ba_temp;
   reg [2 : 0] mem_ba_out;
   reg mem_ck_temp;
   reg mem_ck_out;
   reg mem_ck_n_temp;
   reg mem_ck_n_out;
   reg mem_cke_temp;
   reg mem_cke_out;
   reg mem_cs_n_temp;
   reg mem_cs_n_out;
   reg mem_ras_n_temp;
   reg mem_ras_n_out;
   reg mem_cas_n_temp;
   reg mem_cas_n_out;
   reg mem_we_n_temp;
   reg mem_we_n_out;
   reg mem_reset_n_temp;
   reg mem_reset_n_out;
   logic mem_dq_oe;
   logic mem_dq_oe_temp = 0;
   reg [31 : 0] mem_dq_temp;
   reg [31 : 0] mem_dq_out;
   logic [31 : 0] mem_dq_in;
   logic [31 : 0] mem_dq_local;
   logic mem_dqs_oe;
   logic mem_dqs_oe_temp = 0;
   reg [3 : 0] mem_dqs_temp;
   reg [3 : 0] mem_dqs_out;
   logic [3 : 0] mem_dqs_in;
   logic [3 : 0] mem_dqs_local;
   logic mem_dqs_n_oe;
   logic mem_dqs_n_oe_temp = 0;
   reg [3 : 0] mem_dqs_n_temp;
   reg [3 : 0] mem_dqs_n_out;
   logic [3 : 0] mem_dqs_n_in;
   logic [3 : 0] mem_dqs_n_local;
   reg mem_odt_temp;
   reg mem_odt_out;
   reg [3 : 0] mem_dm_temp;
   reg [3 : 0] mem_dm_out;
   logic [0 : 0] oct_rzqin_in;
   logic [0 : 0] oct_rzqin_local;

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
   
   event signal_input_mem_dq_change;
   event signal_input_mem_dqs_change;
   event signal_input_mem_dqs_n_change;
   event signal_input_oct_rzqin_change;
   
   function automatic string get_version();  // public
      // Return BFM version string. For example, version 9.1 sp1 is "9.1sp1" 
      string ret_version = "14.0";
      return ret_version;
   endfunction

   // -------------------------------------------------------
   // mem_a
   // -------------------------------------------------------

   function automatic void set_mem_a (
      ROLE_mem_a_t new_value
   );
      // Drive the new value to mem_a.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_a_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_ba
   // -------------------------------------------------------

   function automatic void set_mem_ba (
      ROLE_mem_ba_t new_value
   );
      // Drive the new value to mem_ba.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_ba_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_ck
   // -------------------------------------------------------

   function automatic void set_mem_ck (
      ROLE_mem_ck_t new_value
   );
      // Drive the new value to mem_ck.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_ck_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_ck_n
   // -------------------------------------------------------

   function automatic void set_mem_ck_n (
      ROLE_mem_ck_n_t new_value
   );
      // Drive the new value to mem_ck_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_ck_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_cke
   // -------------------------------------------------------

   function automatic void set_mem_cke (
      ROLE_mem_cke_t new_value
   );
      // Drive the new value to mem_cke.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_cke_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_cs_n
   // -------------------------------------------------------

   function automatic void set_mem_cs_n (
      ROLE_mem_cs_n_t new_value
   );
      // Drive the new value to mem_cs_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_cs_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_ras_n
   // -------------------------------------------------------

   function automatic void set_mem_ras_n (
      ROLE_mem_ras_n_t new_value
   );
      // Drive the new value to mem_ras_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_ras_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_cas_n
   // -------------------------------------------------------

   function automatic void set_mem_cas_n (
      ROLE_mem_cas_n_t new_value
   );
      // Drive the new value to mem_cas_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_cas_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_we_n
   // -------------------------------------------------------

   function automatic void set_mem_we_n (
      ROLE_mem_we_n_t new_value
   );
      // Drive the new value to mem_we_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_we_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_reset_n
   // -------------------------------------------------------

   function automatic void set_mem_reset_n (
      ROLE_mem_reset_n_t new_value
   );
      // Drive the new value to mem_reset_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_reset_n_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_dq
   // -------------------------------------------------------
   function automatic ROLE_mem_dq_t get_mem_dq();
   
      // Gets the mem_dq input value.
      $sformat(message, "%m: called get_mem_dq");
      print(VERBOSITY_DEBUG, message);
      return mem_dq_in;
      
   endfunction

   function automatic void set_mem_dq (
      ROLE_mem_dq_t new_value
   );
      // Drive the new value to mem_dq.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dq_temp = new_value;
   endfunction
   
   function automatic void set_mem_dq_oe (
      bit enable
   );
      // bidir port mem_dq will work as output port when set to 1.
      // bidir port mem_dq will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dq_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // mem_dqs
   // -------------------------------------------------------
   function automatic ROLE_mem_dqs_t get_mem_dqs();
   
      // Gets the mem_dqs input value.
      $sformat(message, "%m: called get_mem_dqs");
      print(VERBOSITY_DEBUG, message);
      return mem_dqs_in;
      
   endfunction

   function automatic void set_mem_dqs (
      ROLE_mem_dqs_t new_value
   );
      // Drive the new value to mem_dqs.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dqs_temp = new_value;
   endfunction
   
   function automatic void set_mem_dqs_oe (
      bit enable
   );
      // bidir port mem_dqs will work as output port when set to 1.
      // bidir port mem_dqs will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dqs_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // mem_dqs_n
   // -------------------------------------------------------
   function automatic ROLE_mem_dqs_n_t get_mem_dqs_n();
   
      // Gets the mem_dqs_n input value.
      $sformat(message, "%m: called get_mem_dqs_n");
      print(VERBOSITY_DEBUG, message);
      return mem_dqs_n_in;
      
   endfunction

   function automatic void set_mem_dqs_n (
      ROLE_mem_dqs_n_t new_value
   );
      // Drive the new value to mem_dqs_n.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dqs_n_temp = new_value;
   endfunction
   
   function automatic void set_mem_dqs_n_oe (
      bit enable
   );
      // bidir port mem_dqs_n will work as output port when set to 1.
      // bidir port mem_dqs_n will work as input port when set to 0.
      
      $sformat(message, "%m: method called arg0 %0d", enable); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dqs_n_oe_temp = enable;
   endfunction

   // -------------------------------------------------------
   // mem_odt
   // -------------------------------------------------------

   function automatic void set_mem_odt (
      ROLE_mem_odt_t new_value
   );
      // Drive the new value to mem_odt.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_odt_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // mem_dm
   // -------------------------------------------------------

   function automatic void set_mem_dm (
      ROLE_mem_dm_t new_value
   );
      // Drive the new value to mem_dm.
      
      $sformat(message, "%m: method called arg0 %0d", new_value); 
      print(VERBOSITY_DEBUG, message);
      
      mem_dm_temp = new_value;
   endfunction

   // -------------------------------------------------------
   // oct_rzqin
   // -------------------------------------------------------
   function automatic ROLE_oct_rzqin_t get_oct_rzqin();
   
      // Gets the oct_rzqin input value.
      $sformat(message, "%m: called get_oct_rzqin");
      print(VERBOSITY_DEBUG, message);
      return oct_rzqin_in;
      
   endfunction

   assign sig_mem_a = mem_a_temp;
   assign sig_mem_ba = mem_ba_temp;
   assign sig_mem_ck = mem_ck_temp;
   assign sig_mem_ck_n = mem_ck_n_temp;
   assign sig_mem_cke = mem_cke_temp;
   assign sig_mem_cs_n = mem_cs_n_temp;
   assign sig_mem_ras_n = mem_ras_n_temp;
   assign sig_mem_cas_n = mem_cas_n_temp;
   assign sig_mem_we_n = mem_we_n_temp;
   assign sig_mem_reset_n = mem_reset_n_temp;
   assign mem_dq_oe = mem_dq_oe_temp;
   assign sig_mem_dq = (mem_dq_oe == 1)? mem_dq_temp:'z;
   assign mem_dq_in = (mem_dq_oe == 0)? sig_mem_dq:'z;
   assign mem_dqs_oe = mem_dqs_oe_temp;
   assign sig_mem_dqs = (mem_dqs_oe == 1)? mem_dqs_temp:'z;
   assign mem_dqs_in = (mem_dqs_oe == 0)? sig_mem_dqs:'z;
   assign mem_dqs_n_oe = mem_dqs_n_oe_temp;
   assign sig_mem_dqs_n = (mem_dqs_n_oe == 1)? mem_dqs_n_temp:'z;
   assign mem_dqs_n_in = (mem_dqs_n_oe == 0)? sig_mem_dqs_n:'z;
   assign sig_mem_odt = mem_odt_temp;
   assign sig_mem_dm = mem_dm_temp;
   assign oct_rzqin_in = sig_oct_rzqin;


   always @(mem_dq_in) begin
      if (mem_dq_oe == 0) begin
         if (mem_dq_local != mem_dq_in)
            -> signal_input_mem_dq_change;
         mem_dq_local = mem_dq_in;
      end
   end
   
   always @(mem_dqs_in) begin
      if (mem_dqs_oe == 0) begin
         if (mem_dqs_local != mem_dqs_in)
            -> signal_input_mem_dqs_change;
         mem_dqs_local = mem_dqs_in;
      end
   end
   
   always @(mem_dqs_n_in) begin
      if (mem_dqs_n_oe == 0) begin
         if (mem_dqs_n_local != mem_dqs_n_in)
            -> signal_input_mem_dqs_n_change;
         mem_dqs_n_local = mem_dqs_n_in;
      end
   end
   
   always @(oct_rzqin_in) begin
      if (oct_rzqin_local != oct_rzqin_in)
         -> signal_input_oct_rzqin_change;
      oct_rzqin_local = oct_rzqin_in;
   end
   


// synthesis translate_on

endmodule

