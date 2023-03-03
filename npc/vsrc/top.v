`timescale 1ns/1ns
module top(
    input a,
    input b,
    output f
);
    assign  f = a ^ b;
    always begin
       #10;
       if($time >= 100) begin
           $finish;
        end
    end
endmodule
