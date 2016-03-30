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


#include "aras.h"
#include "predecessor_generator.h"
#include "neighborhood_graph.h"
#include "open_list.h"
#include "../postprocessor.h"
#include "../operator_cost.h"
#include "../globals.h"
#include "../successor_generator.h"
#include "../timer.h"
#include "../option_parser.h"
#include "../plugin.h"

#include "fstream"
#include "iostream"
#include "sstream"
#include "time.h"
#include "math.h"
#include <climits>
#include <sys/times.h>
#include <sys/resource.h>
#include <deque>
#include <set>

#define OVERHEAD_FACTOR 1.25
#define BOUNDERY_FACTOR 0.9
#define TIME_KEEPING_PERIOD 1000

using namespace std;

Aras::Aras(const Options &opts) : cost_type(OperatorCost(opts.get_enum("cost_type"))){
	memory_limit = opts.get<int>("memory_limit");
	time_limit = opts.get<int>("time_limit");
	num_iterations = opts.get<int>("num_iterations");
	steps_per_node = opts.get<int>("spn");
	reg_graph = opts.get<bool>("reg_graph");
	ae = opts.get<bool>("ae");

	input_plan = g_input_plan_filename;

	plan_count = 1;
	time_keeping_counter = 0;
	mem = 0; 
	predecessor_generator = new PredecessorGenerator;
	cout << "Initializing Aras with the following parameters: " << endl;
	dump();
}
void Aras::run(){
	if(g_input_plan_filename != ""){
		cout << "Running Aras..." << endl;
		ifstream inputfile;
		inputfile.open(g_input_plan_filename.c_str(), ios::in);
		if(!inputfile){
			cerr << "Error: cannot open the file \"" << g_input_plan_filename << "\"" << endl;
			abort();
		}
		iterative_PNGS(inputfile);
		inputfile.close();
	}
}

void Aras::dump(){
	cout << "memory_limit: ";
	if(memory_limit == -1)
		cout << "None";
	else
		cout << memory_limit;

	cout << " time_limit: ";
	if(time_limit == -1)
		cout << "None";
	else
		cout << time_limit;
	cout << " num_iterations: " << num_iterations << endl;
	cout << "steps per node: " << steps_per_node << endl;
	cout << "action elimination: ";
	if(ae){
		cout << "true" << endl;
	}else
		cout << "false" << endl;
	cout << "build regression graph: ";
	if(reg_graph)
	    cout << "true" << endl;
	else
        cout << "false" << endl;
	if(input_plan != ""){
		cout << "Input plan: " << input_plan << endl;
	}else
		cout << "Input plan: None" << endl;
    cout << "cost_type: ";
    print(cost_type);
    cout << endl;
}



Aras::~Aras() {
}

int Aras::get_adjusted_cost(const Operator &op) const {
    return get_adjusted_action_cost(op, cost_type);
}

bool Aras::memory_is_full(size_t mem_usage){
	mem = size_t(OVERHEAD_FACTOR * mem_usage / 1000); 
	if((mem_usage * OVERHEAD_FACTOR) / 1000 >= BOUNDERY_FACTOR * memory_limit){
		return true;
	}
	return false;
}

bool Aras::exceed_time_limit(){
	if(time_limit == -1)
	    return false;
	time_keeping_counter ++;
	if(time_keeping_counter == TIME_KEEPING_PERIOD){
		if(timer->operator ()() > time_limit){
			return true;
		}
		time_keeping_counter = 0;
	}
	return false;
}

void Aras::trim_space(string& str){
	int temp_index = str.find_last_not_of(" ");
	str = str.substr(0, temp_index + 1);
}

void Aras::read_plan(ifstream& in, vector<const Operator*>& plan) {
	map<string,const Operator *> operators;
	for (int i = 0; i < g_operators.size(); ++i) {
		string str = g_operators[i].get_name();
		trim_space(str);
		operators.insert(make_pair(str, &g_operators[i]));
	}
	// cout << operators.size() << endl;
	string str;
	while (!in.eof()) {
		// cout << "l" << endl;
		getline(in, str);
		//cout << str << endl;
		int start = str.find("(");
		int end = str.find(")");
		if (start == string::npos || end == string::npos)
			continue;
		str = str.substr(start + 1, (end - start) - 1);
		//cout << str << endl;
		str = strtolower(str);
		trim_space(str);
		map<string, const Operator*>::iterator it = operators.find(str);
		if (it != operators.end()) {
			plan.push_back(it->second);
		} else {
			cout << "Error: the operator: " << str << " is not found" << endl;
		}
	}
}

void Aras::update_plan_info(vector<const Operator*>& plan){
	states.clear();
	states.push_back(*g_initial_state);

	for (int i = 0; i < plan.size(); ++i) {
		states.push_back(State(states.back(), *plan[i]));
	}
	/*if(wrapper != 0 )
		wrapper->memorize(plan);*/
	assert(test_goal(states[states.size() - 1]));
}

string Aras::strtolower(string str) {
	int leng = str.length();
	for (int i=0; i<leng; i++)
		if (65<=str[i]&&str[i]<=90)//A-Z
			str[i]+=32;
	return str;
}

void Aras::dump_plan(vector<const Operator*>& plan) {
	for (int i = 0; i < plan.size(); ++i) {
		cout << i << ": " << plan[i]->get_name() << endl;
	}
}

vector<const Operator*> Aras::action_elimination(ifstream& in){
	cout << "Running the Action Elimination ..." << endl;
	vector<const Operator*> plan;
	read_plan(in, plan);
	cout << "Plan size: " << plan.size() << endl;
	cout << "Plan cost: " << calculate_plan_cost(plan) << endl;
	return action_elimination(plan);
}
vector<const Operator*> Aras::action_elimination(vector<const Operator*>& plan){

	vector<State *> pool;
	vector<bool> removed;
	removed.resize(plan.size(), false);
	State* current_state = g_initial_state;
	for (int i = 0; i < plan.size(); ++i) {
		//cout << "after" << endl;
		// cout << plan[i]->get_name() << endl;
		
		//cout << "I am double checking" << endl;
		if(removed[i])
			continue;
		//cout << "step: " << i << endl;
		if(!plan[i]->is_applicable(*current_state)){
		    throw InvalidPlan();
		}
		vector<int> temp_removed;
		temp_removed.push_back(i);
		State* temp_state = current_state;
		for (int j = i + 1; j < plan.size(); ++j) {
			if(removed[j]){
				continue;
				//cout << i << " is removed" << endl;
			}

			if(plan[j]->is_applicable(*temp_state)){
				//cout << "    " << j << endl;
				temp_state = new State(*temp_state, *plan[j]);
				pool.push_back(temp_state);
			}else{
				temp_removed.push_back(j);
			}
		}
		if(test_goal(*temp_state)){
			//cout << temp_removed.size() << " actions are removed" << endl;
			for (int i = 0; i < temp_removed.size(); ++i) {
				removed[temp_removed[i]] = true;
			}
		}else{
			//plan[i]->dump();
			//current_state->dump();

			assert(plan[i]->is_applicable(*current_state));

			
			current_state = new State(*current_state, *plan[i]);
			pool.push_back(current_state);
			//cout << "before" << endl;
		}
	}
	vector<const Operator*> new_plan;
	for (int i = 0; i < plan.size(); ++i) {
		if(!removed[i]){
			new_plan.push_back(plan[i]);
		}/*else{
			util << i << endl; 
		}*/
	}
	// cout << "Modified plan size: " << new_plan.size() << endl;
	for (int i = 0; i < pool.size(); ++i) {
		delete pool[i];
	}
	// util.close();
	return new_plan;
}

vector<const Operator*> Aras::iterative_PNGS(ifstream& in){
	vector<const Operator*> plan;
	read_plan(in, plan);
	plan = iterative_PNGS(plan);
	return plan;
}

vector<const Operator*> Aras::iterative_PNGS(vector<const Operator*>& plan){
	timer = new Timer;
	NeigborhoodGraph neighborhood_graph;
	int step = 0;
	int current_cost = calculate_plan_cost(plan);
	cout << "Initial plan size: " << plan.size();
	cout << " Initial plan cost: " << current_cost << endl;
	if(ae){
		plan = action_elimination(plan);
		current_cost = calculate_plan_cost(plan);
		cout << "After action elimination";
		cout << " Plan size: " << plan.size();
		cout << " plan cost: " << current_cost << endl;
	    save_plan(plan, plan_count);
		plan_count++;
	}
	
	int succ_cost = current_cost;
	for (int i = 0; i < num_iterations; ++i) {
		Timer iteration_time;
		update_plan_info(plan);
		int start = 0;
		int finish = plan.size();
		int pre_size = neighborhood_graph.size();
		current_cost = succ_cost;
		vector<const Operator*> backup_plan;
		backup_plan.insert(backup_plan.end(), plan.begin(), plan.end());
		assert(backup_plan.size() == plan.size());
		try{
			plan = PNGS(start, finish, neighborhood_graph, plan);
		    if(ae)
		        plan = action_elimination(plan);
		}catch (FullMemory e){
			cout << "Memory Usage: " << mem << endl;
			cout << "Memory-limit is exceeded, estimated memory usage: " << (e.memory * OVERHEAD_FACTOR)/1000 << " KB" << endl;
			delete timer;
			return plan;
		} catch (FinishTime e){
			cout << "Time-limit is exceeded." << endl;
			delete timer;
			return plan;
		} catch (InvalidPlan e){
	           // HACK!!! regression expansion does not work with derived predicates
		       // and leads to invalid plans. For now, if it happens we just switch off
		       // the regression expansion.
		       reg_graph = false;
		       plan.clear();
		       plan.insert(plan.end(), backup_plan.begin(), backup_plan.end());
		       assert(backup_plan.size() == plan.size());
		       cout << "Found an invalid plan. Switch off the recursive expansion " << endl;
		       return plan;
		}
		succ_cost = calculate_plan_cost(plan);
		if(succ_cost != current_cost){
		    save_plan(plan, plan_count);
		    plan_count++;
		}
		assert(succ_cost <= current_cost);
		iteration_time.stop();
		cout << "Step: " << step;  
		cout << " n: " << steps_per_node; 
		cout << " Plan_size: " << plan.size();
		cout << " Plan_cost: " << succ_cost;
		cout << " Iteration_Time: " << iteration_time;
		cout << " Total_Time: " << g_timer; 
		cout << " Size: " << neighborhood_graph.size(); 
		cout << " Number_goals: " << goals.size();
		cout << " Memory usage: " << mem; 
		cout << endl;
		if(neighborhood_graph.size() == pre_size){
			delete timer;
			cout << "Neighborhood graph does not grow any more" << endl;
			return plan;
		}
		float factor = 2;
		int temp = int(steps_per_node * factor);
		if(temp > steps_per_node)
			steps_per_node = temp;
		else{
			cout << "The steps_per_node value does not fit in an integer" << endl;
			delete timer;
			return plan;
		}
		step ++;
	}
	return plan;
}


vector<const Operator*> Aras::PNGS(ifstream& in){
	vector<const Operator*> plan;

	read_plan(in, plan);
	update_plan_info(plan);
	NeigborhoodGraph search_space;
	return PNGS(0, plan.size(), search_space, plan);
}

// Plan Neighborhood Graph Search
vector<const Operator*> Aras::PNGS(int start, int finish, NeigborhoodGraph& search_space,vector<const Operator*>& plan){

	// cout << "building progression graph ..." << endl;
	build_progression_graph(start, finish, search_space, plan);
	
	if(reg_graph){
		// cout << "progression graph size: " << search_space.size() << endl;
		// cout << "building regression graph ..." << endl;
		build_regression_graph(search_space, plan);
		// cout << "regression graph size: " << search_space.size() << endl;
	}
	plan = chain_backward(search_space);
	
	return plan;
}

int Aras::blind_evaluation(State state){
	if(test_goal(state))
		return 0;
	return g_min_action_cost;
}

int Aras::backward_blind_evaluation(State state){
	if(state == *g_initial_state)
		return 0;
	return g_min_action_cost;
}


void Aras::build_progression_graph(int start, int finish, NeigborhoodGraph& search_space,vector<const Operator*>& plan){
	int current_state = start;
	int initial_h_value = blind_evaluation(states[current_state]); 
	
	for (int i = start; i <= finish; ++i) {
		Node node = search_space.get_node(states[current_state]);
		if(i == start)	
			node.open_initial(initial_h_value);
		
		progression_expand(search_space, node);
		
		current_state ++; 
		if(i == finish)
			break;
		
		const Operator* op = plan[i];
		Node succ_node = search_space.get_node(states[current_state]);
	
		assert(succ_node.is_new());
		int succ_h = blind_evaluation(states[current_state]);
		int op_cost = get_adjusted_cost(*op);
		succ_node.update_and_open(succ_h, 0, op_cost);

		//if(!node.is_expanded() || succ_node.get_parent_num() == 0){	
		succ_node.add_parent(node, op);
		//}
	}
	current_state = 0;
	for (int i = 0; i < plan.size(); ++i) {
		Node node = search_space.get_node(states[current_state]);
		current_state ++; 
		const Operator* op = plan[i];
		Node succ_node = search_space.get_node(states[current_state]);
		assert(succ_node.is_new());
		//if(!node.is_expanded() || succ_node.get_parent_num() == 0){	
		succ_node.add_parent(node, op);
		//}
	}
	assert(test_goal(states[states.size() -1 ]));
	goals.insert(states.back());
}

void Aras::progression_expand(NeigborhoodGraph& search_space, Node& initial_node){
	int generated_states = 0;
	ArasOpenList<state_var_t *> open_list;

	vector<state_var_t *> close_list;
	assert(initial_node.is_open());
	open_list.insert(make_pair(initial_node.get_f(), initial_node.get_h()), initial_node.get_state_buffer());
	int sum_applicable = 0;
	int count = 0;
	while(generated_states < steps_per_node && !open_list.empty()){
		
		//cout << "openlist: " << open_list.memory_usage() << "search_space: " <<
		//search_space.memory_usage() << " close_list: " << close_list.capacity() << endl; 
		size_t mem_usage = open_list.memory_usage() + search_space.memory_usage() + close_list.capacity() * sizeof(state_var_t *);
		if(memory_is_full(mem_usage))
			throw FullMemory(mem_usage);
		if(exceed_time_limit())
			throw FinishTime();
		State state(open_list.remove_min());
		Node node = search_space.get_node(state);
		if(node.is_closed()){
			continue;
		}
		vector<const Operator *> applicable_ops;
		applicable_ops.clear();
		g_successor_generator->generate_applicable_ops(node.get_state(), applicable_ops);
		sum_applicable += applicable_ops.size();
		count ++;
		for (int i = 0; i < applicable_ops.size(); i++) {
			const Operator *op = applicable_ops[i];
			int op_cost = get_adjusted_cost(*op);

			State succ_state(node.get_state(), *op);
			generated_states++;

			Node succ_node = search_space.get_node(succ_state);
				
			if(!node.is_expanded()){	
				succ_node.add_parent(node, op);
			}

			if (succ_node.is_dead_end()) {
				continue;
			}


			if(succ_node.is_new()){
				int succ_h = blind_evaluation(succ_state);
				succ_node.update_and_open(succ_h, node.get_g(), op_cost);
				open_list.insert(make_pair(succ_node.get_f(), succ_h), succ_node.get_state_buffer());
				assert(succ_node.get_parent_num() > 0);
				
				continue;
			}
			
			if(succ_node.is_open()){
			    if(node.get_g() + op_cost < succ_node.get_g()) {
			    	succ_node.update_and_reopen(node.get_g(), op_cost);
			    	open_list.insert(make_pair(succ_node.get_f(), succ_node.get_h()), succ_node.get_state_buffer());
			    }
			}else{
				assert(succ_node.get_g() <= node.get_g() + op_cost);
			}

		}
		node.close();
		close_list.push_back(node.get_state_buffer());
	}
	for (int i = 0; i < close_list.size(); ++i) {
		State state(close_list[i]);
		Node node = search_space.get_node(state);
		node.make_permanent();
		if(test_goal(state))
			goals.insert(state);
	}
	
	while(!open_list.empty()){
		State state(open_list.remove_min());
		Node node = search_space.get_node(state);
		node.make_permanent();
		if(test_goal(state))
			goals.insert(state);
	}
}

void Aras::build_regression_graph(NeigborhoodGraph& search_space,vector<const Operator*>& plan){
	
	int current_state = plan.size();
	for (int i = 0; i < plan.size(); ++i) {
		assert(current_state < states.size());
		Node node = search_space.get_node(states[current_state]);
		int h = backward_blind_evaluation(states[current_state]);
		node.open_initial(h);
		regression_expand(search_space, node);
		current_state --; 
	}
	return;
}

void Aras::regression_expand(NeigborhoodGraph& search_space, Node& initial_node){
	
	int generated_states = 0;
	ArasOpenList<state_var_t *> open_list;
	vector<state_var_t *> close_list;
	open_list.insert(make_pair(initial_node.get_f(), initial_node.get_h()), initial_node.get_state_buffer());
	int sum_leading = 0;
	int count = 0;
	while(generated_states < steps_per_node && !open_list.empty()){
		//cout << "openlist: " << open_list.memory_usage() << "search_space: " <<
		//search_space.memory_usage() << " close_list: " << close_list.capacity() << endl; 
		size_t mem_usage = open_list.memory_usage() + search_space.memory_usage() + close_list.capacity() * sizeof(state_var_t *);
		if(memory_is_full(mem_usage))
			throw FullMemory(mem_usage);
		if(exceed_time_limit())
			throw FinishTime();

		State state(open_list.remove_min());
		Node node = search_space.get_node(state);
		if(node.is_closed()){
			continue;
		}

		vector<const Operator *> leading_ops;
		predecessor_generator->generate_leading_operators(node.get_state(), leading_ops);
		sum_leading += leading_ops.size();
		count ++;
		for (int i = 0; i < leading_ops.size(); i++) {
			const Operator *op = leading_ops[i];
			// TODO make the cost type flexible
			int op_cost = 1;
			State pred_state(*op, node.get_state());
			generated_states++;

			Node pred_node = search_space.get_node(pred_state);
			
			if(!pred_node.is_expanded() && !node.is_reg_expanded()){	
				node.add_parent(pred_node, op);
			}

			if(pred_node.is_new()){
				int pred_h = backward_blind_evaluation(*g_initial_state);
				pred_node.update_and_open(pred_h, node.get_g(), op_cost);
				open_list.insert(make_pair(pred_node.get_f(), pred_h), pred_node.get_state_buffer());
			}
			
			if(pred_node.is_open()){
				if(node.get_g() + op_cost < pred_node.get_g()) {
			    	pred_node.update_and_reopen(node.get_g(), op_cost);
			    	open_list.insert(make_pair(pred_node.get_f(), pred_node.get_h()), pred_node.get_state_buffer());
			    }
			}else{
				assert(pred_node.get_g() <= node.get_g() + op_cost);
			}
		}
		node.close();
		close_list.push_back(node.get_state_buffer());
	}
	
	for (int i = 0; i < close_list.size(); ++i) {
		State state(close_list[i]);
		Node node = search_space.get_node(state);
		node.make_reg_permanent();
	}
	
	while(!open_list.empty()){
		State state(open_list.remove_min());
		Node node = search_space.get_node(state);
		node.make_reg_permanent();
	}
}


vector<const Operator*> Aras::chain_backward(NeigborhoodGraph& state_space){
	// This function uses the actual cost of the actions
	vector<const Operator*> plan;
	ArasOpenList<state_var_t *> open_list;
	NeigborhoodGraph search_space;
	search_space.set_shallow();
	set<State>::iterator curr, end = goals.end();
	for(curr = goals.begin(); curr != end; ++curr) {
		Node head = search_space.get_node(*curr);
		int h = backward_blind_evaluation(*curr);
		/*if(g_use_metric){
		    if(h > 0)
		       h--;
		}*/
		head.open_initial(h);
		open_list.insert(make_pair(head.get_f(), h), head.get_state_buffer());
	}
	int num_expanded_states = 0;
	int sum_parent_ptrs = 0;
	while(true){
	        	
		//cout << "openlist: " << open_list.memory_usage() << "search_space: " <<
		//search_space.memory_usage() << " state_space: " << state_space.memory_usage() << endl; 
		size_t mem_usage = search_space.memory_usage() + state_space.memory_usage()
								+ open_list.memory_usage();
		if(memory_is_full(mem_usage))
			throw FullMemory(mem_usage);
		if(exceed_time_limit())
			throw FinishTime();
		assert(!open_list.empty());
		State state(open_list.remove_min());
		if(state == (*g_initial_state)){
			search_space.trace_path(state, plan);
			//std::reverse(plan.begin(), plan.end());
			avg_parent_ptrs = float(num_expanded_states) / float(sum_parent_ptrs); 
			return plan;
		}
		Node node = search_space.get_node(state);
		Node paragon = state_space.get_node(state);
		
		const vector<pair<state_var_t *, const Operator *> > parents = paragon.get_parents();
		
		
		num_expanded_states ++;
		
		for (int i = parents.size() - 1; i >= 0 ; i--) {
			const Operator* op = parents[i].second;
			Node succ_node = search_space.get_node(State(parents[i].first));
			
			assert(!succ_node.is_dead_end());

			if(succ_node.is_new()){
				int succ_h = backward_blind_evaluation(succ_node.get_state());
		                /*if(g_use_metric){
		                    if(succ_h > 0)
		                        succ_h--;
		                }*/
				succ_node.open(succ_h, node, op, op->get_cost());
				open_list.insert(make_pair(succ_node.get_f(), succ_h), succ_node.get_state_buffer());
				continue;
			}
			
			if(node.is_open()){
			    if(node.get_g() + op->get_cost() < succ_node.get_g()) {
			    	succ_node.reopen(node, op, op->get_cost());
			    	open_list.insert(make_pair(succ_node.get_f(), succ_node.get_h()), succ_node.get_state_buffer());
			    }
			}else{
				assert(succ_node.get_g() <= node.get_g() + op->get_cost());
			}
		}
	}
}

static Postprocessor *_parse(OptionParser &parser) {
	parser.add_option<int>("memory_limit", DEFAULT_MEM_LIMIT, "memory limit for aras");
	parser.add_option<int>("time_limit", DEFAULT_TIME_LIMIT, "time limit for aras");
	parser.add_option<bool>("reg_graph", DEFAULT_RG_GRAPH, "build the regression graph");
    parser.add_option<bool>("ae", DEFAULT_AE, "use action elimination");
    parser.add_option<int>("num_iterations", DEFAULT_N_ITER, "number of iterations");
    parser.add_option<int>("spn", DEFAULT_SPN, "steps per node in the neighborhood graph");

    vector<string> cost_types;
    cost_types.push_back("NORMAL");
    cost_types.push_back("ONE");
    cost_types.push_back("PLUSONE");
    parser.add_enum_option("cost_type",
                           cost_types,
                           "PLUSONE",
                           "operator cost adjustment type");

    Options opts = parser.parse();
    Postprocessor *postprocessor = 0;
    if (!parser.dry_run()) {
    	postprocessor = new Aras(opts);
    }
    return postprocessor;
}

static Plugin<Postprocessor> _plugin("Aras", _parse);
