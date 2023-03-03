VERILATOR = verilator
VERILATOR_FLAG = -Wall --cc --exe --build --trace --timing
VSRC = ./vsrc/top.v
CSRC = ./csrc/sim_main.cpp


all:
	@echo "Write this Makefile by your self."
	$(VERILATOR) $(VERILATOR_FLAG) $(VSRC) $(CSRC)
sim:
	$(call git_commit, "sim RTL") # DO NOT REMOVE THIS LINE!!!
	@echo "Write this Makefile by your self."
	./obj_dir/Vtop
	gtkwave wave.vcd
	
include ../Makefile
