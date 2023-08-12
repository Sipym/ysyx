module RegisterFile #(ADDR_WIDTH = 5, DATA_WIDTH = 64, REG_DATA_WIDTH = 64) (
    input                           clk,
    input [DATA_WIDTH-1:0]          wdata,
    input [ADDR_WIDTH-1:0]          waddr,
    input [ADDR_WIDTH-1:0]          rs1,
    input                           wen,
    output reg [REG_DATA_WIDTH-1:0] rs1_data
);
  
    reg [DATA_WIDTH-1:0] rf [ADDR_WIDTH-1:0]; //寄存器
    integer i;
    initial begin
        for (i = 0; i < 32; i = i + 1)
            rf[i] = 64'b0;
    end

    always @(posedge clk) begin
        if (wen) rf[waddr] <= wdata;
        rf[0] <= 0;           //0号寄存器的特性
        rs1_data <= rf[rs1[4:0]]; //输出rs1的值
    end

     
endmodule
