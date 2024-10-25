
export BASEJUMP_STL_DIR = $(abspath ../../..)

run:
	verilator   --cc  --trace --coverage --assert --prof-cfuncs  --build  --exe\
		-I$(BASEJUMP_STL_DIR)/bsg_misc -f sv.include \
		 --top-module  testbench \
		--main --timing  -Wno-lint -Wno-MULTITOP -Wno-MULTIDRIVEN  -Wno-UNOPTFLAT  -Wno-WIDTHCONCAT


clean:
	rm -r -f DVEfiles
	rm -r -f csrc
	rm -f sim.log
	rm simv
	rm -r -f simv.daidir 
	rm -f ucli.key
	rm -f vcdplus.vpd
	rm -rf obj_dir