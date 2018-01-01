/*
 * ConvertVectorToChar.h
 *
 *  Created on: Dec 18, 2017
 *      Author: salud
 */

#ifndef CONVERTVECTORTOCHAR_H_
#define CONVERTVECTORTOCHAR_H_

#include <vector>
using namespace std;

char** VectorToChar(const vector<char*>& oldVector) {
	char** all_cmds = new char*[sizeof(char*) * (oldVector.size() + 1)]; // total size is current vector's size + 1 (+1 is for NULL)

	for (unsigned int i = 0; i < oldVector.size(); ++i) {
		all_cmds[i] = oldVector.at(i);
	}
	all_cmds[oldVector.size()] = 0;
	return all_cmds;
}



#endif /* CONVERTVECTORTOCHAR_H_ */
