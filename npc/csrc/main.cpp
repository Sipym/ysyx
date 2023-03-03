#include "Vtop.h"   // create `top.v`,so use `Vtop.h`
#include "verilated.h"
#include <assert.h>
#include <nvboard.h>
#include <stdio.h>
#include <stdlib.h>

void nvboard_bind_all_pins(Vtop* top);

int main(int argc, char** argv, char** env)
{

    VerilatedContext* contextp = new VerilatedContext;
    contextp->commandArgs(argc, argv);
    Vtop* top = new Vtop{contextp};

    nvboard_bind_all_pins(top);
    nvboard_init();
    while (!contextp->gotFinish()) {
        nvboard_update();
        top->sw = top->sw;
        top->eval();

    }
    delete top;
    // tfp->close();
    delete contextp;
    return 0;
}
