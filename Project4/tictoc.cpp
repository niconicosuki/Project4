/*
* Copyright 2010 Andrew Leifer et al <leifer@fas.harvard.edu>
* This file is part of MindControl.
*
* MindControl is free software: you can redistribute it and/or modify
* it under the terms of the GNU  General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* MindControl is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with MindControl. If not, see <http://www.gnu.org/licenses/>.
*
* For the most up to date version of this software, see:
* http://github.com/samuellab/mindcontrol
*
*
*
* NOTE: If you use any portion of this code in your research, kindly cite:
* Leifer, A.M., Fang-Yen, C., Gershow, M., Alkema, M., and Samuel A. D.T.,
* 	"Optogenetic manipulation of neural activity with high spatial resolution in
*	freely moving Caenorhabditis elegans," Nature Methods, Submitted (2010).
*/


/*
* File:   tictoc.cpp
* Author: Marc
*
* Created on December 3, 2009, 12:07 PM
*/

#include "tictoc.h"
#include "limits.h"
#include <map>
#include <cassert>
#include <sstream>
#include "Timer.h"

using namespace std;
using namespace TICTOC;

tictoc::tictoc() {
	tim = new Timer();
	tim->start();
	enabled = true;
}
tictoc::~tictoc() {
	delete tim;
}

struct TICTOC::_tictoc_data {
	int ncalls;
	double starttime;
	double totaltime;
	double maxtime;
	double mintime;
	bool ticked;
	int numblowntics;
};

static _tictoc_data ntt();

_tictoc_data ntt() {
	_tictoc_data tt;
	tt.ncalls = 0;
	tt.starttime = 0;
	tt.totaltime = 0;
	tt.maxtime = 0;
	tt.mintime = INT_MAX;
	tt.ticked = false;
	tt.numblowntics = 0;
	return tt;
}



