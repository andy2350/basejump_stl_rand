#include "message.h"
#include "skeleton.h"
#include "snd_dest_fn.h"
#include "snd_freq_fn.h"
#include "rcv_freq_fn.h"
#include "Vtest_bsg.h"
#include "configs.h"
#include <random>

extern void set_msg_gen(MessageGenerator *gen);

int main(int argc, char **argv) {
    int seed = 0;
    if (argc >=2) {
        seed = atoi(argv[1]);
    }
    srand(seed);
    
    GeneratorSkeleton<EDGE_LP, DATA_LP> gen(
        SndDestFn_initializer,
        SndFreqFn_initializer,
        RcvFreqFn_initializer
    );

    set_msg_gen((MessageGenerator*)&gen);

    auto dut = new Vtest_bsg;
    int cnt = 0;
    while (!gen.is_finished() && cnt < TOTAL_CYCLES) {
        printf("New Cycle\n");
        printf("===========Neg Edge============\n");
        dut->clk = 0;
        dut->eval();
        printf("===========Pos Edge============\n");
        dut->clk = 1;
        gen.clk();
        dut->eval();
        cnt += 1;
    }

    printf("FINISHED\n");
}