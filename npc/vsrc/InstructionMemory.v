module InstructionMemory #(ADDR_WIDTH = 32, INS_NUM = 1) (
    input [ADDR_WIDTH-1:0]  pc,
    output reg [11:0]       imm,
    output reg [4:0]        rs1,
    output reg [2:0]        func3,
    output reg [4:0]        rd,
    output reg [6:0]        opcode,
    output reg              en

);
    
    reg [31:0] insts [INS_NUM:0];
    reg [31:0] ins;
    initial begin  //存放指令
        //inst[0] = ... 
    end

    assign ins     = insts[(pc - 32'h8000_0000)/4]; 
	assign imm     = ins[31:20];
	assign rs1     = ins[19:15];
	assign func3   = ins[14:12];
	assign rd      = ins[11:7];
	assign opcode  = ins[6:0];
	assign en      = 1;

endmodule
