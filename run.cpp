/*
 * run.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: salud
 */

#include <iostream>
#include <string>
#include <errno.h>
#include <stdio.h>
#include "Parser.h"
#include <unistd.h>
#include <limits.h>
using namespace std;

int main() {
	Parser new_parse; // commence parser
	while (true) {
		new_parse.askUser(); // prompt user and get input

		new_parse.parse(); // parse and build commands

		AllCommands* new_cmd = new AllCommands(new_parse.getCommands());

		new_cmd->execute(); // execute cmds

		delete new_cmd; // dealloc. cmd

		new_parse.setUserInput(""); // start over
	}
	return 0;
}
