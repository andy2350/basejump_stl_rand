#pragma once
#include "message.h"
#include <vector>
#include <functional>
#include <cstdio>

class SndDestFn {
public:
    virtual void clk() { }
    virtual std::pair<int, int> get_dest() { return {0, 0}; }
    virtual void next_msg() { }
};

class SndFreqFn {
public:
    virtual void clk() { }
    virtual bool decide_snd() { return false; }
    virtual void snd_ok() { }
};

class RcvFreqFn { 
public:
    virtual void clk() { }
    virtual bool decide_rcv() { return false; }
};

using init_snd_dest_t=std::function<SndDestFn* (int i, int j)>;
using init_snd_freq_t=std::function<SndFreqFn* ()>;
using init_rcv_freq_t=std::function<RcvFreqFn* ()>;

template<int edge_lp, int data_lp>
class NocTile {
public:
    static constexpr int N = 1 << edge_lp;
    static constexpr int total = N * N;
    static constexpr int mask_edge = (1 << edge_lp) - 1;
    static constexpr int mask_data = (1 << data_lp) - 1;
    
    int n_clk;
    int coord_x;
    int coord_y;
    int last_yummi;
    SndDestFn *snd_dest;
    SndFreqFn *snd_freq;
    RcvFreqFn *rcv_freq;

    Message msg_rcv, msg_snd;

    ~NocTile() {
        delete snd_dest;
        delete snd_freq;
        delete rcv_freq;    
    }

    NocTile() {
        n_clk = 0;
        last_yummi = 0;
        coord_x = 0;
        coord_y = 0;
        snd_dest = nullptr;
        snd_freq = nullptr;
        rcv_freq = nullptr;
    }

    void clk() {
        snd_dest->clk();
        snd_freq->clk();
        rcv_freq->clk();
    }

    void get_message(Message *msg) {
        auto [dest_x, dest_y] = snd_dest->get_dest();

        msg->dest_x = dest_x;
        msg->dest_y = dest_y;
        msg->data = coord_x * N + coord_y;
        msg->valid = snd_freq->decide_snd();
        msg_snd = *msg;
    }

    Message notify_on_send() {
        snd_dest->next_msg();
        snd_freq->snd_ok();
        return msg_snd;
    }

    void notify_on_receive(const Message &msg) {
        msg_rcv = msg;
    }

    bool check_consume() {
        last_yummi = rcv_freq->decide_rcv();
        return last_yummi;
    }

    void reset() {

    }
};

template<int edge_lp, int data_lp>
class GeneratorSkeleton : MessageGenerator {
public:
    static constexpr int N = 1 << edge_lp;
    static constexpr int total = N * N;
    static constexpr int mask_edge = (1 << edge_lp) - 1;
    static constexpr int mask_data = (1 << data_lp) - 1;

    std::vector<NocTile<edge_lp, data_lp>> tiles;
    // msg_cnt[i][j] is the number of messages i send to j
    // msg_cnt[i][j] is increase when i send a message to j
    // and is decrease when j received a message from i
    std::vector<std::vector<int>> msg_cnt; 

    GeneratorSkeleton(init_snd_dest_t snd_dest_init,
                      init_snd_freq_t snd_freq_init,
                      init_rcv_freq_t rcv_freq_init) {
        tiles.resize(total);
        msg_cnt.resize(total, std::vector<int>(total, 0));
        for (int i = 0; i < N; ++i)
            for (int j = 0; j < N; ++j) {
                int idx = i * N + j;
                tiles[idx].coord_x = i;
                tiles[idx].coord_y = j;
                tiles[idx].snd_dest = snd_dest_init(i, j);
                tiles[idx].snd_freq = snd_freq_init();
                tiles[idx].rcv_freq = rcv_freq_init();
            }
    }

    void clk() {
        for (auto &tile : tiles)
            tile.clk();    
    }

    void get_message(int i, int j, Message *msg) override {
        int idx = i * N + j;
        tiles[idx].get_message(msg);
    }

    void notify_on_send(int i, int j) override {
        int idx = i * N + j;
        auto msg_snd = tiles[idx].notify_on_send();

        msg_cnt[idx][msg_snd.dest_x * N + msg_snd.dest_y] += 1;
        printf("(%d, %d) send to (%d, %d)\n", i, j, msg_snd.dest_x, msg_snd.dest_y);
    }

    void notify_on_receive(int i, int j, void *msg) override {
        int idx = i * N + j;
        int coord_x = (*((int*)msg) >> edge_lp) & mask_edge;
        int coord_y = *((int*)msg) & mask_edge;
        int data = (*((int*)msg) >> (edge_lp * 2)) & mask_data;

        Message msg_rcv{
            .dest_x = coord_x, 
            .dest_y = coord_y, 
            .data = data,
            .valid = 1
        };
        tiles[idx].notify_on_receive(msg_rcv);

        if (tiles[idx].last_yummi) {
            auto &msg_rcv = tiles[idx].msg_rcv;
            if (msg_cnt[msg_rcv.data][idx] == 0) {
                printf("Error: %d recieved more data from %d than send\n", idx, msg_rcv.data);
            } else {
                msg_cnt[msg_rcv.data][idx] -= 1;
                printf("%d recieved message from %d\n", idx, msg_rcv.data);
            }
        }
    }

    void check_consume(int i, int j, char *yummi) override {
        int idx = i * N + j;
        *yummi = tiles[idx].check_consume();
    }

    void reset_fn() override {
        for (auto &tile : tiles)
            tile.reset();
        for (int i = 0; i < total; ++i)
            for (int j = 0; j < total; ++j)
                msg_cnt[i][j] = 0;
    }

    bool is_finished() {
        return false;
    }
};