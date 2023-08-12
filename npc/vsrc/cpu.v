module cpu #() (
    input [31:0] pc,
    input clk
);
    reg [4:0]   rd,rs1;
    reg [11:0]  imm;
    reg [63:0]  rs1_data,rs2_data;
    reg [2:0]   func3;
    reg [6:0]   opcode;
    reg         en;
    InstructionMemory #(32,  1) i0 (
        .pc         (pc),
        .clk        (clk),
        .rd         (rd),
        .rs1        (rs1),
        .imm        (imm),
        .func3      (func3),
        .opcode     (opcode),
        .en         (en)
    );     
    RegisterFile #(5, 64, 64) i1 (
        .clk        (clk),
        .wdata      (0), //利用了0号寄存器的特性
        .waddr      (0),
        .rs1        (rs1),
        .rs1_data   (rs1_data),
        .wen        (1)
    );
    ALU #(64) i2 (
        .in_x       (rs1_data),
        .in_y       ({{52{imm[11]}} , imm}), //符号扩展imm
        .clk        (clk),
        .out_c      (rs2_data),
        .carry      () //悬空不需要
    );
    always begin
        if (en == 1) begin
            en = 0;
            if ((opcode == 7'b0010011) && (func3 == 3'b000)) begin 
                ; 
            end
        end
    end
    RegisterFile #(5, 64, 64) i3 (
        .clk        (clk),
        .wdata      (0), //利用了0号寄存器的特性
        .waddr      (0),
        .rs1        (rs1),
        .rs1_data   (rs1_data),
        .wen        (1)
    );


endmodule
