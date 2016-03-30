#ifndef STATE_H
#define STATE_H

#include <iostream>
#include <vector>
using namespace std;

class Operator;

#include "state_var_t.h"

class State {
    state_var_t *vars; // values for vars
    // added by hootan
    static std::vector<std::pair<int, int> > effects;
    bool borrowed_buffer;
    void _allocate();
    void _deallocate();
    void _copy_buffer_from_state(const State &state);
public:
    // Added by Hootan
    // begin
    void update(const Operator &op);
    State(const Operator &op, const State &successor);
    // end

    void dump() const;

    explicit State(istream &in);
    State(const State &state);
    State(const State &predecessor, const Operator &op);
    ~State();
    State &operator=(const State &other);
    state_var_t &operator[](int index) {
        return vars[index];
    }
    int operator[](int index) const {
        return vars[index];
    }
    bool operator==(const State &other) const;
    bool operator<(const State &other) const;
    size_t hash() const;


    explicit State(state_var_t *buffer) {
        vars = buffer;
        borrowed_buffer = true;
    }
    const state_var_t *get_buffer() const {
        return vars;
    }
};

#endif
