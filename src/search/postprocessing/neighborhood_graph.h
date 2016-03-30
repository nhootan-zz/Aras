/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Aras.
 * This is a modified version of search_space.cc in FD's code base.
 * Aras is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the license, or (at your option) any later version.
 *
 * Aras is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/


#ifndef NEIGHBORHOOD_GRAPH_H
#define NEIGHBORHOOD_GRAPH_H
#include "../state.h"

#include <vector>
#define LOADING_FACTOR 2
#define PER_NODE_OVERHEAD 12
using namespace std;
class Operator;
class NodeInfo;

class Node {
    state_var_t *state_buffer;
    NodeInfo &info;
    size_t* parents_num;

public:
    Node(state_var_t *state_buffer_, NodeInfo &info_, size_t* parents_num_byte);

    state_var_t *get_state_buffer() {
      return state_buffer;
    }
    State get_state() const;

    bool is_goal() const;
    bool is_open() const;
    bool is_closed() const;
    bool is_dead_end() const;
    bool is_expanded() const;
    bool is_reg_expanded() const;
    bool is_new() const;

    int get_f() const;
    int get_g() const;
    int get_h() const;
    int get_level() const;
    int get_parent_num() const;
    
    void set_level(int  l);

	void add_parent(const Node &parent_node, const Operator *parent_op);
	void make_permanent();
	void make_reg_permanent();
	
	
	void lazy_open(int h, const Node &parent_node, const Operator *parent_op);
	void lazy_reopen(const Node &parent_node, const Operator *parent_op);

	void update_and_open(int h, int g, int op_cost);
	void update_and_reopen(int g, int op_cost);
    
    void open_initial(int h);
    void open(int h, const Node &parent_node, const Operator *parent_op);
    void reopen(const Node &parent_node, const Operator *parent_op);
    
    void open(int h, const Node &parent_node, const Operator *parent_op, int op_cost);
    void reopen(const Node &parent_node, const Operator *parent_op, int op_cost);

    void close();
    void mark_as_dead_end();
    const vector<pair<state_var_t *, const Operator *> > get_parents();


    void dump();
};

//class BoostingNode : public SearchNode{
	
// };

class NeigborhoodGraph {
    class HashTable;
    HashTable *nodes;
    bool keep_shallow_copy;
    size_t parents_num;
public:
    NeigborhoodGraph();
    ~NeigborhoodGraph();
    int size() const;
    size_t memory_usage() const;
    Node get_node(const State &state);
    void trace_path(const State &goal_state,
		    std::vector<const Operator *> &path) const;

    void set_shallow(){
    	keep_shallow_copy = true;
    }
    void dump();
    void statistics() const;
};



#endif
