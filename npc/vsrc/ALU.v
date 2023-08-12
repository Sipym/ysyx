//暂时只有加法功能
module ALU #(DATA_WITDH = 2) (
    input [DATA_WITDH-1:0]  in_x,
    input [DATA_WITDH-1:0]  in_y,
    input                   clk,
    output reg [DATA_WITDH-1:0] out_c,
    output                  carry
);
    always @(posedge clk) begin
        {carry,out_c} <= in_x + in_y;
    end

endmodule
