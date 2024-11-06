#pragma once
#include "message.h"
#include "skeleton.h"

class AllToOneSndFn : SndDestFn {
public:
    int target_x;
    int target_y;
    AllToOneSndFn(int target_x, int target_y) :
        target_x(target_x), target_y(target_y) 
    { }

    std::pair<int, int> get_dest() override {
        return {target_x, target_y};
    }

    static init_snd_dest_t initializer(int t_x, int t_y) {
        return [=](int, int) {
            return (SndDestFn*)(new AllToOneSndFn(t_x, t_y));
        };
    }
};

template<int N>
class RandomKSndFn : SndDestFn {
public:
    int K;
    int target_x;
    int target_y;
    std::vector<std::pair<int, int>> targets;
    RandomKSndFn(int K) : K(K) {
        for (int i = 0; i < K; ++i)
            targets.push_back({rand() % N, rand() % N });
        next_msg();
    }

    std::pair<int, int> get_dest() override {
        return {target_x, target_y};
    }

    void next_msg() override {
        auto [tx, ty] = targets[rand() % K];
        target_x = tx;
        target_y = ty;
    }

    static init_snd_dest_t initializer(int K) {
        return [=](int, int) {
            return (SndDestFn*)(new RandomKSndFn<N>(K));
        };
    }
};


template<int N>
class RightOrBottomSndFn : SndDestFn {
public:
    int target_x;
    int target_y;
    int x;
    int y;
    RightOrBottomSndFn(int x, int y) : x(x), y(y) {
        next_msg();
    }

    std::pair<int, int> get_dest() override {
        return {target_x, target_y};
    }

    void next_msg() override {
        int flag = rand() & 1;
        if (flag == 0) {
            target_x = N - 1;
            target_y = y;
        } else {
            target_x = x;
            target_y = N - 1;
        }
    }

    static init_snd_dest_t initializer() {
        return [=](int x, int y) {
            return (SndDestFn*)(new RightOrBottomSndFn<N>(x, y));
        };
    }  
};

template<int B>
class BlockSndFn : SndDestFn {
public:
    int target_x;
    int target_y;
    int x;
    int y;
    BlockSndFn(int x, int y) : x(x), y(y) {
        next_msg();
    }

    std::pair<int, int> get_dest() override {
        return {target_x, target_y};
    }

    void next_msg() override {
        int block_x = (x / B) * B;
        int block_y = (y / B) * B;

        int off_x = rand() % B;
        int off_y = rand() % B;

        target_x = block_x + off_x;
        target_y = block_y + off_y;        
    }

    static init_snd_dest_t initializer() {
        return [=](int x, int y) {
            return (SndDestFn*)(new BlockSndFn<B>(x, y));
        };
    }
};