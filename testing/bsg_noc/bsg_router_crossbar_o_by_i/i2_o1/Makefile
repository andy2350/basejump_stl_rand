
# Workspace directory
export BASEJUMP_STL_DIR = $(abspath ../../../..)

HISTO = | grep -E "[0-9]  CYCLE" | awk -v size=1 'BEGIN{bmin=10000000}{ b=int($$8/size); a[b]++; bmax=b>bmax?b:bmax; bmin=b<bmin?b:bmin } END { for(i=bmin;i<=bmax;++i) print i*size,(i+1)*size,a[i] }'
#HISTO=

run:
	/home/linb/verilator/bin/verilator   --cc  --trace --coverage --assert --prof-cfuncs  --build  --exe\
		-I$(BASEJUMP_STL_DIR)/bsg_misc  -I$(BASEJUMP_STL_DIR)/bsg_noc -f ./filelist \
		--top-module  bsg_router_crossbar_o_by_i_tester --timescale "1ps/1ps" \
		--main --timing  -Wno-lint -Wno-MULTITOP -Wno-MULTIDRIVEN  -Wno-UNOPTFLAT  -Wno-WIDTHCONCAT -Wno-MODDUP
	@executable=$$(find ./obj_dir -maxdepth 1 -type f -perm /u+x | head -n 1); \
    if [ -n "$$executable" ]; then \
        $$executable; \
    else \
        echo "No executable found in ./obj_dir"; \
    fi
	/home/linb/verilator/bin/verilator_coverage --annotate-min 1 --annotate ./annotate coverage.dat
view:
	$(VCS_BIN)/dve -full64 -vpd vcdplus.vpd &

junk = csrc DVEfiles simv.daidir *.old *.vpd simv *.key vc_hdrs.h

clean:
	rm -rf $(junk)

