#include "message.h"
#include "svdpi.h"

static MessageGenerator *msg_gen = nullptr;

void set_msg_gen(MessageGenerator *gen) {
    msg_gen = gen;    
}

extern "C" void get_message(int i, int j, svBitVecVal *msg) {
    msg_gen->get_message(i, j, (Message *)msg);
}

extern "C" void notify_on_send(int i, int j) {
    msg_gen->notify_on_send(i, j);
}

extern "C" void notify_on_receive(int i, int j, svBitVecVal *rcv_data) {
    msg_gen->notify_on_receive(i, j, (void*)rcv_data);
}

extern "C" void reset_fn() {
    msg_gen->reset_fn();
}

extern "C" void check_consume(int i, int j, svBitVecVal *yumi) {
    msg_gen->check_consume(i, j, (char*)yumi);
}