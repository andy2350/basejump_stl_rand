#include <cstdio>
#include <cassert>
#include <vector>
#include <cstdlib>
#include "message.h"
#include "Vtest_bsg.h"

#ifdef NDEBUG
#undef NDEBUG
#endif

// edge_lp: Edge bits
// data_lp: Data bits
template<int edge_lp, int data_lp>
class SimpleGenerator : MessageGenerator {
public:
    static constexpr int N = 1 << edge_lp;
    static constexpr int total = N * N;
    static constexpr int mask_edge = (1 << edge_lp) - 1;
    static constexpr int mask_data = (1 << data_lp) - 1;

    int n_clk;
    int n_finished; // Number of tiles that finished its receiving
    bool finished;
    std::vector<int> send_cnt;
    std::vector<int> received_cnt;

    SimpleGenerator() : 
        send_cnt(total, 0), 
        received_cnt(total, 0), n_clk(0)
    {
        n_finished = 0;
        finished = false;
    }

    void clk() {
        n_clk += 1;
    }

    void get_message(int i, int j, Message *msg) {
        int idx = i * N + j;
        int dest = idx ^ send_cnt[idx];
        if (send_cnt[idx] < total) {
            msg->data = idx;
            msg->dest_x = dest / N;
            msg->dest_y = dest % N;
            msg->valid = 1;
        } else {
            msg->valid = 0;
        }
    }

    void notify_on_send(int i, int j) {
        if (send_cnt[i * N + j] < total) {
            printf("(%d, %d) send %d\n", i, j, send_cnt[i * N + j]);
            send_cnt[i * N + j] += 1;
        }
    }

    void notify_on_receive(int i, int j, void *msg) {
        int coord_x = (*((int*)msg) >> edge_lp) & mask_edge;
        int coord_y = *((int*)msg) & mask_edge;
        int data = (*((int*)msg) >> (edge_lp * 2)) & mask_data;

        /* Weird in Verilator. These assert does not report on error. */
        assert(i == coord_x);
        assert(j == coord_y);

        received_cnt[i * N + j] += 1;
        printf("(%d=%d, %d=%d) received %d %d\n", 
                coord_x, i, coord_y, j, 
                data, received_cnt[i * N + j]);
        if (received_cnt[i * N + j] == total)
            n_finished += 1;

        if (n_finished == total)
            finished = true;
    }

    void reset_fn() {
        finished = false;
        n_finished = 0;
        received_cnt.assign(total, 0);
        send_cnt.assign(total, 0);
    }

    void check_consume(int i, int j, char *yumi) {
        *yumi = 1;
    }

    bool is_finished() const {
        return finished;
    }
};

extern void set_msg_gen(MessageGenerator *gen);

int main() {
    SimpleGenerator<2, 4> gen;

    set_msg_gen((MessageGenerator*)&gen);
    auto dut = new Vtest_bsg;

    int cnt = 0;
    while (!gen.is_finished() && cnt < 100) {
        printf("New Cycle\n");
        printf("===========Neg Edge============\n");
        dut->clk = 0;
        dut->eval();
        printf("===========Pos Edge============\n");
        dut->clk = 1;
        dut->eval();
        gen.clk();
        cnt += 1;
    }

    printf("FINISHED\n");
}