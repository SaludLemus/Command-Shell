/*
 * run.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: salud
 */

#include <iostream>
#include <string>
#include <unistd.h>
#include <cstdlib>
#include "Parser.h"
using namespace std;

int main() {
	string user_input;
	char* current_user = getlogin(); // returns current logged in user

	if (current_user == NULL) {
		cout << "Error, current user can not be determined." << endl;
		exit(EXIT_FAILURE);
	}

	while (true) {
		while(user_input.size() == 0) {
			cout << current_user << "$ " << flush; // prompt
			getline(cin, user_input); // ask for user input
		}

		Parser new_parse(user_input);

		new_parse.parse();

		AllCommands* new_cmd = new AllCommands(new_parse.getCommands());

		new_cmd->execute(); // execute cmds

		delete new_cmd; // dealloc. cmd
		user_input = ""; // reset user's input
	}
	return 0;
}
