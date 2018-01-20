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
#include <boost/tokenizer.hpp>

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

ChangeDirectory::ChangeDirectory() : Command(), command(0) {}

ChangeDirectory::ChangeDirectory(char** new_cmd, const stack<string> &current_dir) : Command() , command(new_cmd), entire_directory(current_dir) {}

ChangeDirectory::~ChangeDirectory() {
	if (command) {
		int index = 0;

		while (command[index]) { // dealloc each char*
			char* entire_cmd = command[index];
			delete [] entire_cmd;

			++index;
		}
		delete [] command; // dealloc char**
		command = 0;
	}
}

bool ChangeDirectory::execute() {
	bool change_directory_success = false;
	string next_directory = "/";

	if (command) { // exists
		string directory(command[0]); // contains directory
		boost::char_separator<char> sep("/");
		boost::tokenizer<boost::char_separator<char> > dir(directory, sep);
		stack<string> temp_directory(entire_directory); // determines whether new directory exists or not
		
		for (boost::tokenizer<boost::char_separator<char> >::iterator itr = dir.begin(); itr != dir.end(); ++itr) {
			string current_item(*itr); // get string
			
			if (current_item.empty()) { // a '/' was encountered
				;
			}
			else if (current_item == ".") { // stay in the same directory
				;
			}
			else if (current_item == "..") { // go back one directory
				if (!temp_directory.empty() && temp_directory.top() != "home") {
					temp_directory.pop(); // remove current directory
				}
			}
			else if (current_item == "~") { // start from the home directory
				changeToHomeDirectory();
			}
			else { // a directory
				temp_directory.push(current_item); // add to PATH
			}
		}
		
		string temp_string_directory = next_directory; // will hold absolute PATH
		stack<string> temp_stack; // revert the stack
		
		while (!temp_directory.empty()) { // add to temporary stack
			temp_stack.push(temp_directory.top());
			
			temp_directory.pop();
		}
		
		while (!temp_stack.empty()) { // change back to normal
			temp_directory.push(temp_stack.top());
			temp_string_directory.append(temp_stack.top()); // get entire size
			temp_string_directory.append(next_directory);
			
			temp_stack.pop();
		}
		
		if (!temp_string_directory.empty()) {
			temp_string_directory.pop_back(); // remove last '/'
		}
		
		char* new_directory = new char[sizeof(char) * (temp_string_directory.size() + 1)]; // total size: string's + 1 (+1 for NULL char)
		
		for (unsigned int i = 0; i < temp_string_directory.size(); ++i) { // convert to char*
			new_directory[i] = temp_string_directory.at(i);
		}
		
		new_directory[temp_string_directory.size()] = '\0'; // append NULL terminator
		
		int change_directory_value = chdir(new_directory); // change current directory of the current process
		
		if (change_directory_value == -1) { // cannot change directory
			checkChangeDirectoryFailure();
		}
		else {
			entire_directory = temp_directory; // copy over new directory for the Parse's PATH
		}
		
		change_directory_success = true; // success
	}

	return change_directory_success;
}

void ChangeDirectory::display() {
	if (command) {
		cout << "cd " << command[0];
	}
	return;
}

char** ChangeDirectory::getALLCMDS() {
	return command;
}

void ChangeDirectory::setALLCMDS(char** new_cmd) {
	command = new_cmd;
	return;
}

stack<string>& ChangeDirectory::getDirectory() {
	return entire_directory;
}

void ChangeDirectory::checkChangeDirectoryFailure() {
	if (errno == EACCES) { // errno is set
		perror("Search permission is denied for one of the directories.");
	}
	else if (errno == EFAULT) {
		perror("Path points outside your accessible address space.");
	}
	else if (errno == EIO) {
		perror("An I/O error occurred.");
	}
	else if (errno == ELOOP) {
		perror("Too many symbolic links were encountered in resolving path.");
	}
	else if (errno == ENAMETOOLONG) {
		perror("Path is too long.");
	}
	else if (errno == ENOENT) {
		perror("The file does not exist.");
	}
	else if (errno == ENOMEM) {
		perror("Insufficient kernel memory was available.");
	}
	else if (errno == ENOTDIR) {
		perror("A component of path is not a directory");
	}
	return;
}

void ChangeDirectory::changeToHomeDirectory() {
	while (!entire_directory.empty() && entire_directory.top() != "home") { // continue to pop until home directory is reached
		entire_directory.pop();
	}
	return;
}

HistoryCommand::HistoryCommand() : Command(), command_history(0) {}

HistoryCommand::HistoryCommand(const vector<string> &new_history) : Command(), command_history(new_history) {}

HistoryCommand::~HistoryCommand() {}

bool HistoryCommand::execute() {
	if (command_history.size() > 0) { // history exists
		for (int i = 0; i < command_history.size(); ++i) { // print all commands
			cout << "  " << i + 1 << "  " << command_history.at(i) << endl;
		}
		
		return true;
	}
	return false;
}

void HistoryCommand::display() {
	cout << "history";
	return;
}

char** HistoryCommand::getALLCMDS() {
	return 0;
}

void HistoryCommand::setALLCMDS(char** new_cmd) {
	return;
}

HelpCommand::HelpCommand() : Command() {
	supported_commands = "   echo [string]   - echo back the string\n\n";
	supported_commands.append("   cd [directory]   - change directory\n\n");
	supported_commands.append("   cat [output file name] < [input file name]   - take file as input and\n   redirect to new output file\n\n");
	supported_commands.append("   cat [source file] > [output file name]   - echo contents of file into another file\n   (it will delete contents of that file)\n\n");
	supported_commands.append("   cat [source file] >> [output file name]   - same as above, but contents will be appended to the output file\n\n");
	supported_commands.append("   history   - displays all commands used\n\n");
	supported_commands.append("   test [p] [directory] ]  - determine whether the directory or file exists;\n   [p] parameters: -e if directory/file exists, -d if directory/file is a directory, -f if directory/file is a regular file\n\n");
	supported_commands.append("   \tNOTE: alternative to test functionality: [ [p] [directory/file] ]\n\n");
	supported_commands.append("   [command1] | [command2]   - piping will take the output of the left-side and\n   act as the input for the right-side\n\n");
	supported_commands.append("   help   - displays all supported commands\n\n");
	supported_commands.append("   [command3] &&/|| [command4]   - connectors such as && will execute the right-side command\n   iff the left-side succeeded and || will execute the right-side command iff the left-side failed\n\n");
	supported_commands.append("\tFINAL NOTE: The Command Shell is very strict in accordance to its syntax, so please no erroneous command(s)\n\n");
}

HelpCommand::~HelpCommand() {;}

bool HelpCommand::execute() {
	cout << supported_commands << flush;
	return true;
}

void HelpCommand::display() {
	cout << "help" << endl;
	return;
}

char** HelpCommand::getALLCMDS() {
	return 0;
}

void HelpCommand::setALLCMDS(char** new_command) {
	return;
}


