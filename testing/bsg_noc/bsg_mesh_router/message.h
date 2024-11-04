#pragma once

struct Message {
    int dest_x;
    int dest_y;
    int data;
    char valid;
};

class MessageGenerator {
public:
    // Inform a new clock cycle
    virtual void clk() { }

    // Generate a message goes from (i, j)
    virtual void get_message(int i, int j, Message *msg) { }

    // Notify that the message is successfully send
    virtual void notify_on_send(int i, int j) { }

    // Verify the message arrives at (i, j)
    virtual void notify_on_receive(int i, int j, void *msg) { }

    // Reset the generator when ~reset~ is high
    virtual void reset_fn() { } 

    // Notify whether the current received message is consumed
    // If consumed, assign yumi to true
    virtual void check_consume(int i, int j, char *yumi) { }
};