
# Workspace directory

export BASEJUMP_STL_DIR = $(abspath ../../..)

run:
	verilator   --cc  --trace --coverage-toggle --assert --prof-cfuncs  --build  --exe\
		-I$(BASEJUMP_STL_DIR)/bsg_misc  -I$(BASEJUMP_STL_DIR)/bsg_noc -f ./filelist \
		--top-module  bsg_wormhole_network_tester --timescale "1ps/1ps" \
		--main --timing  -Wno-lint -Wno-MULTITOP -Wno-MULTIDRIVEN  -Wno-UNOPTFLAT  -Wno-WIDTHCONCAT -Wno-MODDUP
	@executable=$$(find ./obj_dir -maxdepth 1 -type f -perm /u+x | head -n 1); \
    if [ -n "$$executable" ]; then \
        $$executable; \
    else \
        echo "No executable found in ./obj_dir"; \
    fi
	verilator_coverage --annotate-min 1 --annotate ./annotate coverage.dat


junk = obj_dir annotate coverage.dat

clean:
	rm -rf $(junk)

