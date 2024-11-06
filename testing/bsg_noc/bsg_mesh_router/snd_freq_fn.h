#include "message.h"
#include "skeleton.h"
#include <random>

class AlwaysSndFn : SndFreqFn {
public:
    bool decide_snd() override {
        return 1;
    }

    static init_snd_freq_t initializer() {
        return []() {
            return (SndFreqFn*)(new AlwaysSndFn);
        };
    }
};

class RandomSndFn : SndFreqFn {
public:
    double p;
    int prev;
    RandomSndFn(double p) : p(p) {}
    bool decide_snd() override {
        if ((double)rand() / RAND_MAX < p)
            return 1;
        else
            return 0;
    }

    static init_snd_freq_t initializer(double p) {
        return [=]() {
            return (SndFreqFn*)(new RandomSndFn(p));
        };
    }
};