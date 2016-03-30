/*********************************************************************
 * Author: Malte Helmert (helmert@informatik.uni-freiburg.de)
 * (C) Copyright 2003-2004 Malte Helmert
 * Modified by: Matthias Westphal (westpham@informatik.uni-freiburg.de)
 * (C) Copyright 2008 Matthias Westphal
 *
 * This file is part of LAMA.
 *
 * LAMA is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the license, or (at your option) any later version.
 *
 * LAMA is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 * Modified by: Hootan Nakhost
 * (C) Copyright 2013 Hootan Nakhost
 * The file is modified to be used inside Aras.
 *
 *
 *********************************************************************/

// #include "open_list.h"

// HACK! Ignore this if used as a top-level compile target.
#ifdef ARAS_OPEN_LIST_H
#define OL_ENTRY_OVERHEAD 20
#define OL_LOAD_FACTOR 2

using namespace std;

/*
  Priority_queue based implementation of an open list.
*/

template<class Entry>
ArasOpenList<Entry>::ArasOpenList() {
}

template<class Entry>
ArasOpenList<Entry>::~ArasOpenList() {
}

template<class Entry>
inline void ArasOpenList<Entry>::insert(pair<int, int> key, const Entry &entry) {
    queue.push(make_pair(key, entry));
}

template<class Entry>
inline pair<int, int> ArasOpenList<Entry>::min() const {
    return (queue.top()).first;
}

template<class Entry>
inline Entry ArasOpenList<Entry>::remove_min() {
    Entry best = queue.top().second;
    queue.pop();
    return best;
}

template<class Entry>
inline bool ArasOpenList<Entry>::empty() const {
    return queue.empty();
}

template<class Entry>
inline void ArasOpenList<Entry>::clear() {
    queue = priority_queue<pair<pair<int, int>, Entry>, vector<pair<pair<int, int>, Entry> >,
	typename ArasOpenList<Entry>::IsWorse >();
}

template<class Entry>
size_t ArasOpenList<Entry>::approx_num_bytes() const {
    size_t value = sizeof(queue) +
                   OL_LOAD_FACTOR*queue.size()*(OL_ENTRY_OVERHEAD + sizeof(pair<pair<int, int>, Entry>));
    return value;
}

template<class Entry>
int ArasOpenList<Entry>::size() const {
    return queue.size();
}

#endif
