/*
 * simulator.h
 *
 *  Created on: Aug 16, 2012
 *      Author: hootan
 */

#ifndef SIMULATOR_H_
#define SIMULATOR_H_

class Heuristic;

class Simulator{
public:
	virtual void set_heuristic(Heuristic* h) = 0;
	virtual Heuristic* get_heuristic() = 0;
	virtual ~Simulator() {}
	virtual void run() = 0;
};


#endif /* SIMULATOR_H_ */
