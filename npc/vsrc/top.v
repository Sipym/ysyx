module top(
    input [1:0] sw,
    output led0,
    output led1
);
    assign  led0 = sw[0] ^ sw[1];
endmodule
