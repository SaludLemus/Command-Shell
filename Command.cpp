/*
 * Command.cpp
 *
 *  Created on: Dec 20, 2017
 *      Author: salud
 */

#include "Command.h"
#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

AllCommands::AllCommands() : Command(), current_commands(0) {}

AllCommands::AllCommands(vector<Command*> *all_cmds) : Command(), current_commands(all_cmds) {}

AllCommands::~AllCommands() {
	for (unsigned int i = 0; i < current_commands->size(); ++i) {
		delete current_commands->at(i);
	}
	current_commands->resize(0);
}

bool AllCommands::execute() {
	for (unsigned int i = 0; i < current_commands->size(); ++i) {
		current_commands->at(i)->execute();
	}
	return true;
}

void AllCommands::display() {
	for (unsigned int i = 0; i < current_commands->size(); ++i) {
		current_commands->at(i)->display();
	}
	return;
}

char** AllCommands::getALLCMDS() {
	return 0;
}

void AllCommands::setALLCMDS(char** new_cmds) {
	return;
}

DefaultCommand::DefaultCommand() : Command(), command(0) {}

DefaultCommand::DefaultCommand(char** cmd_list) : Command(), command(cmd_list) {}

DefaultCommand::~DefaultCommand() {
	if (command) {
		char* current_cmd = command[0];
		int index = 0; // for looping

		while (command[index]) { // loop through char**
			delete [] current_cmd; // dealloc array
			current_cmd = command[++index];
		}
		delete [] command;
		command = 0;
	}
}

bool DefaultCommand::execute() {
	pid_t child_p; // for fork()

	if (command) {
		string check_exit(command[0]);
		if (check_exit == "exit") { // check for exit
			delete command;
			exit(EXIT_SUCCESS); // terminate prog
		}
	}

	child_p = fork(); // create child process

	if (child_p == -1) { // fork() failed
		perror("Fork failed.");
		exit(EXIT_FAILURE);
	}
	else if (child_p == 0) { // in child process
		if (execvp(command[0], command) < 0) { // execvp() failed
			perror("Execvp() failed.");
			exit(EXIT_FAILURE);
		}
	}
	else { // wait for child
		pid_t return_child_status;
		int child_status; // for waitpid()
		do {
			return_child_status = waitpid(child_p, &child_status, 0); // get return status

			if (WIFEXITED(child_status) && WEXITSTATUS(child_status) == EXIT_FAILURE) { // child failed
				perror("Child process failed to execute properly.");
				return false;
			}

		} while(return_child_status != child_p); // wait for child

	}
	return true;
}

void DefaultCommand::display() {
	if (command) {
		int index = 0;
		char* each_cmd = command[index];
		while (each_cmd) {
			if (command[index + 1] == 0) { // last cmd
				std::cout << each_cmd;
			}
			else { // cout cmd and append [space]
				std::cout << each_cmd << ' ';
			}
			each_cmd = command[++index];
		}
	}
	return;
}

char** DefaultCommand::getALLCMDS() {
	return command;
}

void DefaultCommand::setALLCMDS(char** new_cmds) {
	command = new_cmds;
	return;
}

StatCommand::StatCommand() : Command(), command(0) {}

StatCommand::StatCommand(char** new_cmd) : Command(), command(new_cmd) {}

StatCommand::~StatCommand() {
	if (command) {
		int index = 0; // to loop through char* array
		char* temp_cmd = command[index];

		while (command[index]) {
			delete [] temp_cmd; // dealloc char*
			temp_cmd = command[++index];
		}
		delete [] command; // dealloc char**
		command = 0; // set to NULL
	}
}

bool StatCommand::execute() {
	// check if command exists
	if (!command) {
		cout << "Command does not exist." << endl;
	}

	char* get_file_ptr = 0;
	string get_file_type(command[1]);
	struct stat file_info;
	bool file_exists = true;

	if (get_file_type.size() >= 1 && (get_file_type == "-e" || get_file_type == "-f" || get_file_type == "-d")) { // check if command[1] has flag type
		get_file_ptr = command[2];
	}
	else { // -e by default
		get_file_ptr = command[1];
		get_file_type = "-e";
	}

	// call stat command
	if (stat(get_file_ptr, &file_info) == -1) {
		file_exists = false;
	}

	// get flag type
		// check if -e
	if (get_file_type == "-e" && file_exists) {
		cout << "True" << endl;
		return true;
	}

		// check if -d
	else if (get_file_type == "-d") {
			// checks if directory
		if (S_ISDIR(file_info.st_mode)) {
			cout << "True" << endl;
			return true;
		}
		else { // else print false
			cout << "False" << endl;
		}
	}

		// check if -f
	else if (get_file_type == "-f") {
			// if regular file
		if (S_ISREG(file_info.st_mode)) {
			cout << "True" << endl;
			return true;
		}
		else {
			cout << "False" << endl; // else print false
		}
	}

	else {
		cout << "False" << endl;
	}

	return false;
}

void StatCommand::display() {
	if (command) {
		bool flag_exists = true;
		// if [ ] exists, will use state instead
		// assuming full command exists (stat or [] and flag or not and PATH)
		cout << command[0] << ' ';

		string determine_flag(command[1]); // check if flag exists

		if (determine_flag.size() >= 2 && (determine_flag == "-e" || determine_flag == "-f" || determine_flag == "-d")) {
			cout << command[1] << ' ' << command[2];
		}
		else {
			cout << "-e " << command[1];
			flag_exists = false;
		}

		string check_alt_test(command[0]); // check if [
		if (check_alt_test.size() >= 1 && check_alt_test == "[") {
			if (flag_exists) {
				cout << ' ' << command[3];
			}
			else{
				cout << ' ' << command[2];
			}
		}
	}
	return;
}

char** StatCommand::getALLCMDS() {
	return command;
}


void StatCommand::setALLCMDS(char** new_cmds) {
	command = new_cmds;
	return;
}


