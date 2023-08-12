#include "Vcpu.h"   // create `top.v`,so use `Vtop.h`
//#include <nvboard.h>
#include <verilated.h>
#include <verilated_vcd_c.h>

VerilatedContext* contextp = NULL;
VerilatedVcdC* tfp = NULL;
static Vcpu* top = new Vcpu;

void step_and_dump_wave(void) {
    top->eval();
    contextp->timeInc(1);
    tfp->dump(contextp->time());
}

void sim_init(void) {
    contextp = new VerilatedContext;
    tfp = new VerilatedVcdC;
    contextp->traceEverOn(true);
    top->trace(tfp,0);
    tfp->open("wave.vcd");
}

void sim_exit(void) {
    step_and_dump_wave();
    tfp->close();
}

int main() {
    sim_init();
    top->in_x = 0b11; top->in_y = 0b01; step_and_dump_wave();
    top->in_x = 0b01; top->in_y = 0b01; step_and_dump_wave();
    top->in_x = 0b01; top->in_y = 0b00; step_and_dump_wave();
    top->in_x = 0b01; top->in_y = 0b10; step_and_dump_wave();
    sim_exit();
}
