#ifndef OPERATOR_H
#define OPERATOR_H

#include <cassert>
#include <iostream>
#include <string>
#include <vector>

#include "globals.h"
#include "state.h"

struct Prevail {
    int var;
    int prev;
    Prevail(std::istream &in);
    Prevail(int v, int p) : var(v), prev(p) {}

    bool is_applicable(const State &state) const {
        assert(var >= 0 && var < g_variable_name.size());
        assert(prev >= 0 && prev < g_variable_domain[var]);
        return state[var] == prev;
    }

    bool operator==(const Prevail &other) const {
        return var == other.var && prev == other.prev;
    }

    bool operator!=(const Prevail &other) const {
        return !(*this == other);
    }

    void dump() const;
};

struct PrePost {
    int var;
    int pre, post;
    std::vector<Prevail> cond;
    PrePost() {} // Needed for axiom file-reading constructor, unfortunately.
    PrePost(std::istream &in);
    PrePost(int v, int pr, int po, const std::vector<Prevail> &co)
        : var(v), pre(pr), post(po), cond(co) {}

    bool is_applicable(const State &state) const {
        assert(var >= 0 && var < g_variable_name.size());
        assert(pre == -1 || (pre >= 0 && pre < g_variable_domain[var]));
        return pre == -1 || state[var] == pre;
    }

    bool does_fire(const State &state) const {
        for (int i = 0; i < cond.size(); i++)
            if (!cond[i].is_applicable(state))
                return false;
        return true;
    }

    void dump() const;
};

class Operator {
    bool is_an_axiom;
    std::vector<Prevail> prevail;      // var, val
    std::vector<PrePost> pre_post;     // var, old-val, new-val, effect conditions
    mutable bool marked; // Used for short-term marking of preferred operators
// Edited by Hootan
protected:
    std::string name;
    int cost;

public:
    Operator(std::istream &in, bool is_axiom);
    // Added by Hootan
    // begin
    // TODO:Hootan
    // HACK! This is added to be used by ArtificialOperator.
    // The right way to do this is to introduce an abstract class
    // with name Operator and make the current Operator (of course with other name. e.g., RealOperator)
    // and ArtificialOperator inherent from the abstract class.
    Operator(){
		is_an_axiom = false;
    }
    //end
    virtual void dump() const;
    virtual ~Operator(){}
    std::string get_name() const {return name; }

    bool is_axiom() const {return is_an_axiom; }

    const std::vector<Prevail> &get_prevail() const {return prevail; }
    const std::vector<PrePost> &get_pre_post() const {return pre_post; }

    bool is_applicable(const State &state) const {
        for (int i = 0; i < prevail.size(); i++)
            if (!prevail[i].is_applicable(state))
                return false;
        for (int i = 0; i < pre_post.size(); i++)
            if (!pre_post[i].is_applicable(state))
                return false;
        return true;
    }
    // Begin Hootan
    virtual void get_effects(const State& state, std::vector<std::pair<int, int> >& effects) const;
	int get_op_index() const {
	    int op_index = 0;
		if(!is_axiom()){
			op_index = this - &*g_operators.begin();
		    assert(op_index >= 0 && op_index < g_operators.size());
	    }else{
	    	op_index = this - &*g_axioms.begin() + g_operators.size();
	    	assert(op_index >= g_operators.size() && op_index < g_operators.size() + g_axioms.size());
	    }
	    return op_index;
	}
	// END Hootan
    bool is_marked() const {
        return marked;
    }
    void mark() const {
        marked = true;
    }
    void unmark() const {
        marked = false;
    }

    mutable bool marker1, marker2; // HACK! HACK!

    int get_cost() const {return cost; }
	
	
};

#endif
