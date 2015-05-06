`timescale 1ns / 1ps

// control register
module mem_sdram_interface (
			    // inputs:
			    clk,
			    reset_n,
			    writedata,
			    read,
			    write,
			    byteenable,
			    chipselect,
			    address,
			    mem_rst,
			    
			    // outputs:
			    readdata,
			    mem_splat,
			    cc);
   
   output  [ 31: 0] readdata;
   output 	    mem_splat;
   input 	    mem_rst;

   input [ 31: 0]   writedata;
   input [31:0]     cc;
   
   input [3:0] 	    byteenable;
   input 	    clk, reset_n, read, write, chipselect, address;
   
   wire [ 31: 0]    readdata;

   reg 		    state_splat = 0;
   reg [31:0] 	    regcc = 0;
   reg  	    wd;
   
   assign mem_splat = state_splat;

   wire 	    wr_strobe;
   assign wr_strobe = chipselect && write;

   wire rst = ~reset_n || mem_rst;

   always @(posedge mem_rst) begin
      regcc <= cc;
   end
   
   always @(posedge clk or posedge rst) begin
      if (rst == 1) begin
	 state_splat = 0;
	 wd = 0;
      end
      else if (clk) begin
	 if (wr_strobe) begin
	    state_splat <= writedata ? 1 : 0;
	    wd <= writedata[0];
	 end
      end
   end
   
   assign readdata[31] = reset_n;
   assign readdata[30] = mem_rst;
   assign readdata[29] = state_splat;
   assign readdata[28:1] = regcc[31:4];
   assign readdata[0] = wd;

endmodule
