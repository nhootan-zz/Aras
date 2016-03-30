#include "globals.h"
#include "operator.h"
#include "option_parser.h"
#include "ext/tree_util.hh"
#include "timer.h"
#include "utilities.h"
#include "search_engine.h"
#include "simulator.h"
#include "analyzer.h"
#include "postprocessor.h"

#include <iostream>
using namespace std;

int main(int argc, const char **argv) {
	register_event_handlers();

	if (argc < 2) {
		cout << OptionParser::usage(argv[0]) << endl;
		exit(1);
	}

	// Edited by Hootan
	// Begin
	bool run_simulator = false;
	for (int i = 1; i < argc; i++) {
		if (string(argv[i]).compare("--simulator") == 0) {
			run_simulator = true;
			break;
		}
	}

	if (string(argv[1]).compare("--help") != 0 && !run_simulator)
		read_everything(cin);
	// END
	SearchEngine *engine = 0;

	//the input will be parsed twice:
	//once in dry-run mode, to check for simple input errors,
	//then in normal mode
	try {
		OptionParser::parse_cmd_line(argc, argv, true);
		engine = OptionParser::parse_cmd_line(argc, argv, false);
	} catch (ParseError &pe) {
		cout << pe << endl;
		exit(1);
	}

	//Added by Hootan
	// begin
	cout << "random seed " << g_seed << endl;
	if(g_postprocessor != 0 && g_input_plan_filename != ""){
		g_postprocessor->run();
		return 0;
	} if (g_analyzer != 0 ){
		if(g_simulator != 0){
			cerr << "Error: can not run an analyzer and a simulator together" << endl;
			return 1;
		}else if (engine != 0){
			cerr << "Error: can not run an analyzer and a search engine together" << endl;
			return 1;
		}
		g_analyzer->run();
		cout << "Total time: " << g_timer << endl;
		return 0;
	} else if (g_simulator != 0) {
		g_simulator->run();
		return 0;
	}
	if(engine == 0){
		cerr << "Error: no search engine is given." << endl;
		return 1;
	}
	// end

	Timer search_timer;
	engine->search();
	search_timer.stop();
	g_timer.stop();

	engine->save_plan_if_necessary();
	engine->statistics();
	engine->heuristic_statistics();
	cout << "Search time: " << search_timer << endl;
	cout << "Total time: " << g_timer << endl;

	return engine->found_solution() ? 0 : 1;
}
