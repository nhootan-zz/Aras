
/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Aras.
 *
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


#ifndef ARAS_H_
#define ARAS_H_

#include "../operator.h"
#include "../state.h"
#include "../heuristic.h"
#include "../postprocessor.h"
#include "../option_parser.h"
#include "../operator_cost.h"
#include "neighborhood_graph.h"
#include "predecessor_generator.h"
#include <vector>
#include <map>
#include <set>

// default values for Aras parameters
static const bool DEFAULT_RG_GRAPH = true;
static const bool DEFAULT_AE = true;
static const int DEFAULT_MEM_LIMIT = -1;
//static const int DEFAULT_MEM_LIMIT = 2000000;
static const int DEFAULT_TIME_LIMIT = -1;
static const int DEFAULT_N_ITER = 1000000;
static const int DEFAULT_SPN = 1000;


using namespace std;

class FullMemory
{
    public:
        size_t memory;
        FullMemory(size_t memory_) : memory(memory_){}
};

class FinishTime
{
	public:
		FinishTime(){}
};

class InvalidPlan{
   public:
   InvalidPlan(){}
};

struct NodeInfo{
	const State* parent;
	const Operator* op;
	int level;
	NodeInfo(const State* p, const Operator* o, int l){
		parent = p;
		op = o;
		level = l;
	}
};


class Aras : public Postprocessor
{
	//parameters
	string input_plan;
	bool reg_graph;
	int num_iterations;
	bool ae;
	int step_limit;
	int steps_per_node;
	size_t memory_limit;
	int time_limit;
	OperatorCost cost_type;

	PredecessorGenerator *predecessor_generator;
	enum{DONT_CARE = -1};
	vector<bool> can_be_removed;
	vector<int> goal_agenda;
	vector<State> states;
	map<State, int> plan_container;
	int num_removed;
	set<State> goals;
	int nodes_in_graph;
	int plan_count;
	float avg_parent_ptrs;
	size_t mem;
	int time_keeping_counter;
	Timer* timer;
	void read_plan(ifstream& in, vector<const Operator*>& plan);

	string strtolower(string str);
	void trim_space(string& str);
	void dump_plan(vector<const Operator*>& plan);
	void update_plan_info(vector<const Operator*>& plan);
	vector<const Operator*> action_elimination(vector<const Operator*>& plan);
	vector<const Operator*> PNGS(int start, int finish, NeigborhoodGraph& search_space, vector<const Operator*>& plan);
	
	void progression_expand(NeigborhoodGraph& search_space, Node& initial_node);
	void regression_expand(NeigborhoodGraph& search_space, Node& initial_node);
	
	void build_regression_graph(NeigborhoodGraph& search_space,vector<const Operator*>& plan);
	void build_progression_graph(int start, int finish, NeigborhoodGraph& search_space,vector<const Operator*>& plan);
	
	vector<const Operator*> chain_backward(NeigborhoodGraph& state_space);
	vector<const Operator*> tsa_star(NeigborhoodGraph& search_space, Node& initial_node, int initial_h_value, int d, int& next_d);
	
	int blind_evaluation(State state);
	int backward_blind_evaluation(State state);
	void output_memory_usage(NeigborhoodGraph&);
	bool exceed_time_limit();
	bool memory_is_full(size_t mem_usage);
	int get_adjusted_cost(const Operator &op) const;
	
public:
	void run();
	void dump();

	vector<const Operator*> action_elimination(ifstream& in);
	vector<const Operator*> PNGS(ifstream& in);
	vector<const Operator*> iterative_PNGS(vector<const Operator*>& plan);
	vector<const Operator*> iterative_PNGS(ifstream& in);

	Aras(const Options &opts);
	virtual ~Aras();
};

#endif /*ARAS_H_*/
