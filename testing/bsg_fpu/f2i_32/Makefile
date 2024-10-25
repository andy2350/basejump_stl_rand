export BASEJUMP_STL_DIR = $(abspath ../../..)
INCDIR = -I$(BASEJUMP_STL_DIR)/bsg_misc -I$(BASEJUMP_STL_DIR)/bsg_fpu

FPU_TRACE_ROM = bsg_fpu_trace_rom


app:
	gcc -Wall -o app ./f2i_32.c ../common/fpu_common.c \
		-I../common -lm -std=c99

$(FPU_TRACE_ROM).tr: app
	./app > $@

$(FPU_TRACE_ROM).v: $(FPU_TRACE_ROM).tr
	python3 $(BASEJUMP_STL_DIR)/bsg_mem/bsg_ascii_to_rom.py $< $(FPU_TRACE_ROM) > $@


rtl_sim: $(FPU_TRACE_ROM).v
	verilator   --cc  --trace --coverage --assert --prof-cfuncs  --build  --exe\
		$(INCDIR) -f rtl_sv.include \
		 --top-module  testbench \
		--main --timing  -Wno-lint -Wno-MULTITOP -Wno-MULTIDRIVEN  

dve:
	gtkwave vcdplus.vpd &

clean:
	rm -rf obj_dir
	rm -f app
	rm -f vcdplus.vpd simv
	rm -rf csrc simv.daidir DVEfiles
	rm -f $(FPU_TRACE_ROM).tr $(FPU_TRACE_ROM).v
