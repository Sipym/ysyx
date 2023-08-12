#根据工程实时更改
TOP_NAME = cpu

VERILATOR = verilator
#VERILATOR_FLAG = -Wall --cc --exe --build --trace
VERILATOR_FLAG = --cc -j --build -Wall --exe  --trace --top-module $(TOP_NAME)
VSRC = ./vsrc/cpu.v ./vsrc/InstructionMemory
CSRC = ./csrc/main.cpp


all:
	@echo "Write this Makefile by your self."
	$(VERILATOR) $(VERILATOR_FLAG) $(VSRC) $(CSRC)

sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	./obj_dir/Vtop
	gtkwave wave.vcd
	
include ../Makefile
