/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Aras.
 * This is a modified version of successor_generator.h in FD's code base.
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


#ifndef PREDECESSOR_GENERATOR_H_
#define PREDECESSOR_GENERATOR_H_

#include "../state.h"
#include <vector>
 
using namespace std;

class Operator;

class PredecessorGenerator
{
	vector<vector<vector<const Operator*> > > effect_of;
public:
	PredecessorGenerator();
	void generate_leading_operators(const State &curr, vector<const Operator *> &ops);
	virtual ~PredecessorGenerator();
};

#endif /*PREDECESSOR_GENERATOR_H_*/
