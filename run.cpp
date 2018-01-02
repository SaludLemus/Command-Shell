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
#include <sys/types.h>
#include <pwd.h>
#include <stdio.h>
#include <errno.h>
#include "Parser.h"
using namespace std;

void checkUserFailure();

int main() {
	uid_t user_id = getuid(); // get current user's ID
	errno = 0;
	struct passwd* user_info = getpwuid(user_id); // retrieve user ID's info
	string user_input;

	if (user_info == NULL) { // entry not found
		checkUserFailure();
		exit(EXIT_FAILURE);
	}

	while (true) {
		while(user_input.size() == 0) {
			cout << user_info->pw_name << "$ " << flush; // prompt
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

void checkUserFailure() {
	if (errno == 0 || errno == ENOENT || errno == EBADF || errno == EPERM) {
		perror("Either the name or uid was not found.");
	}
	else if (errno == EINTR) {
		perror("A signal was caught.");
	}
	else if (errno == EIO) {
			perror("I/O error.");
	}
	else if (errno == EMFILE) {
			perror("The maximum number of files have already been opened within the current calling process.");
	}
	else if (errno == ENOMEM) {
			perror("Insufficient memory to allocate passwd structure.");
	}
	else if (errno == ERANGE) {
			perror("Insufficient buffer space supplied.");
	}
	else if (errno == ENFILE) {
			perror("The maximum number of files was open already in the system.");
	}
	return;
}
