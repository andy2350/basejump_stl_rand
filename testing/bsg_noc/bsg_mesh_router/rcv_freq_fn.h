#pragma once
#include "message.h"
#include "skeleton.h"
#include <random>

class AlwaysRcvFn : RcvFreqFn {
public:
    bool decide_rcv() override { 
        return true;
    }

    static init_rcv_freq_t initializer() {
        return []() { 
            return (RcvFreqFn*)(new AlwaysRcvFn); 
        };
    }
};

class RandomRcvFn : RcvFreqFn {
public:
    double p;
    RandomRcvFn(double p) : p(p) {}
    bool decide_rcv() override {
        if ((double)rand() / RAND_MAX < p)
            return 1;
        else
            return 0;
    }

    static init_rcv_freq_t initializer(double p) {
        return [=]() {
            return (RcvFreqFn*)(new RandomRcvFn(p));
        };
    }
};
