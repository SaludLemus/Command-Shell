/*
 * Parser.cpp
 *
 *  Created on: Dec 17, 2017
 *      Author: salud
 */

#include "Parser.h"
#include <iostream>
#include <unistd.h>
#include <cstdlib>
#include <sys/types.h>
#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>
#include <locale.h>
#include <readline/readline.h>
#include <readline/history.h>

Parser::Parser() : current_commands(0) {
	current_commands = new vector<Command*>;
}

Parser::Parser(const string& user_input) : user_input(user_input) {
	current_commands = new vector<Command*>();
}

Parser::~Parser() {current_commands = 0;}

vector<Command*>* Parser::getCommands() {
	return current_commands;
}

void Parser::askUser() {
	uid_t user_id = getuid(); // get current user's ID
	errno = 0;
	struct passwd* user_info = getpwuid(user_id); // retrieve user ID's info
	char* current_host_name = new char[sizeof(char) * HOST_NAME_MAX]; // retrieve host name
	char* user_input_cstr = NULL; // for readline()

	if (user_info == NULL) { // entry not found (getpwuid() failed)
		checkUserFailure();
		exit(EXIT_FAILURE);
	}
	
	char* full_path = get_current_dir_name(); // gets full path
		
	if (full_path == NULL) { // get_current_dir_name() failed
		checkPathFailure();
		exit(EXIT_FAILURE);
	}
		
	updatePath(full_path); // convert char* to string path
	
	if (gethostname(current_host_name, HOST_NAME_MAX) == -1) { // error
		checkHostNameFailure(); // find source of error
		exit(EXIT_FAILURE); // terminate the process
	}
	
	while (true) {
		cout << "[" << user_info->pw_name << '@' << current_host_name << ' ' << flush;
		printPath(full_path, user_info); // print proper PATH

		user_input_cstr = readline ("]$ ");

		if (user_input_cstr && strlen(user_input_cstr) > 0) { // make command to str and break from loop
			user_input = user_input_cstr;
			break;
		}
		else if (user_input_cstr && strlen(user_input_cstr) == 0) { // be sure to free the allocated memory for an empty string
			free(user_input_cstr);
		}
	}

	command_history.push_back(user_input); // add to history
	
	free(full_path); // deallocate PATH
	free(user_input_cstr); // deallocate user input
	return;
}

void Parser::parse() {
	boost::char_separator<char> sep(" \"");
	boost::tokenizer<boost::char_separator<char> > tok(user_input, sep); // set tokenizer
	vector<char*> all_cmds; // holds char*
	
	for (boost::tokenizer<boost::char_separator<char> >::iterator start = tok.begin(); start != tok.end(); ++start) {
		string current_word(*start);
		int number_parses = 0; // for connectors
		if (current_word.size() >= 1 && current_word.at(current_word.size() - 1) == ';') { // semi-colon
			current_word = current_word.substr(0, current_word.size() - 1); // from index 0 to (last_index - 1); last_index == ;
			all_cmds.push_back(convertStrToChar(current_word)); // convert to char*

			DefaultCommand* default_cmd= new DefaultCommand(VectorToChar(all_cmds));
			current_commands->push_back(default_cmd);
			all_cmds.resize(0); // reset vector
		}
		else if (current_word.size() >= 2 && (current_word == "&&" || current_word == "||")) { // && or ||
			Connector* new_connector_cmd = 0;
			int current_parse = 0; // loop through parser

			 // determine type of connector (either && or ||)
			if (current_word == "&&") { // &&
				new_connector_cmd = new And();
			}
			else { // ||
				new_connector_cmd = new Or();
			}
			// handle empty vs not empty
			if (all_cmds.size() > 0) { // not empty
				new_connector_cmd->setLeftChild(new DefaultCommand(VectorToChar(all_cmds))); // add left side
			}
			else { // empty
				new_connector_cmd->setLeftChild(current_commands->back()); // get last command (is left child)
				current_commands->pop_back(); // remove last command
			}

			// set right child (right side)
			++start; // move iterator by one
			new_connector_cmd->setRightChild(getNextCommand(start, tok, number_parses)); // get next Command*

			updateParser(start, current_parse, number_parses, all_cmds, new_connector_cmd); // update
		}
		else if (current_word.size() >= 1 && (current_word == "test" || current_word.at(0) == '[')) { // stat or []
			StatCommand* new_stat_cmd = 0;
			int current_parse = 0; // loop through parser

			new_stat_cmd = new StatCommand(getRightSide(start, tok , number_parses)); // fetch right side

			updateParser(start, current_parse, number_parses, all_cmds, new_stat_cmd); // update parser
		}
		else if (current_word.size() >= 1 && (current_word == ">" || current_word == ">>" || current_word == "<")) { // redirectors
			Connector* new_connector_cmd = 0;
			int current_parse = 0;

			if (current_word == ">") { // for output
				new_connector_cmd = new Output();
			}
			else if (current_word == ">>") { // for append
				new_connector_cmd = new Append();
			}
			else { // for input
				new_connector_cmd = new Input();
			}

			if (all_cmds.size() > 0) { // leftChild will be cmd and files
				new_connector_cmd->setLeftChild(new DefaultCommand(VectorToChar(all_cmds))); // get command (e.g. cat, echo) and left side files
			}
			else { // leftChild will be a redirection
				new_connector_cmd->setLeftChild(current_commands->back());
				current_commands->pop_back();
			}
			++start;
			new_connector_cmd->setRightChild(new DefaultCommand(getRightSide(start, tok, number_parses))); // get right side files

			updateParser(start, current_parse, number_parses, all_cmds, new_connector_cmd); // update parser
		}
		else if (current_word.size() >= 1 && current_word == "|") { // pipe
			Connector* new_pipe_cmd = new Pipe();
			int current_parse = 0;

			new_pipe_cmd->setLeftChild(current_commands->back()); // get left command (last index of vector)
			current_commands->pop_back(); // remove cmd from vector

			++start;
			new_pipe_cmd->setRightChild(getNextCommand(start, tok, number_parses));

			updateParser(start, current_parse, number_parses, all_cmds, new_pipe_cmd);
		}
		else if (current_word.size() >= 2 && current_word == "cd") { // change directory
			int current_parse = 0;
			++start;

			ChangeDirectory* new_change_directory = new ChangeDirectory(getRightSide(start, tok, number_parses), current_path);
			
			updateParser(start, current_parse, number_parses, all_cmds, new_change_directory); // update the parser
		}
		else if (current_word.size() == 7 && current_word == "history") { // generate a new history command
			current_commands->push_back(new HistoryCommand(command_history));
		}
		else if (current_word.size() == 4 && current_word == "help") { // create new help command to the user
			current_commands->push_back(new HelpCommand());
		}
		else { // append to vector
			all_cmds.push_back(convertStrToChar(current_word)); // append char*
		}
	}
	
	if (all_cmds.size() > 0) { // single command (default cmd)
		DefaultCommand* new_default_cmd = new DefaultCommand(VectorToChar(all_cmds)); // create char**
		current_commands->push_back(new_default_cmd); // add new command to vector
	}
	return;
}

char** Parser::getRightSide(boost::tokenizer<boost::char_separator<char> >::iterator nextParse, boost::tokenizer<boost::char_separator<char> > &originalToken, int &num_parses) {
	vector<char*> cur_v; // holds char*
	for (; nextParse != originalToken.end(); ++nextParse) { // get next command
		string current_word(*nextParse); // holds right side
		if (current_word.size() >= 1 && (current_word == "&&" || current_word == "||" || current_word.at(current_word.size() - 1) == ';' || current_word.at(current_word.size() - 1) == ']' || current_word == "|")) { // break for connectors
			if (current_word.at(current_word.size() - 1) == ';') { // append to vector
				current_word = current_word.substr(0, current_word.size() - 1);
				cur_v.push_back(convertStrToChar(current_word));
				++num_parses;
			}
			else if (current_word.size() >= 1 && current_word == "]") {
				cur_v.push_back(convertStrToChar(current_word));
				++num_parses;
			}
			break;
		}
		else if (current_word.size() >= 1 && (current_word == "<" || current_word == ">" || current_word == ">>")) {
			break;
		}
		else { // add char* to vector
			cur_v.push_back(convertStrToChar(current_word)); // append char**
			++num_parses; // inc. num parses
		}
	}
	return VectorToChar(cur_v); // convert vector's contents to char**
}

Command* Parser::getNextCommand(boost::tokenizer<boost::char_separator<char> >::iterator nextParse, boost::tokenizer<boost::char_separator<char> > &originalToken, int &num_parses) {
	vector<char*> cur_v; // holds char*
	for (; nextParse != originalToken.end(); ++nextParse) { // get next command
		string current_word(*nextParse); // holds right side
		if (current_word.size() >= 1 && (current_word == "&&" || current_word == "||" || current_word.at(current_word.size() - 1) == ';' || current_word.at(current_word.size() - 1) == ']' || current_word == "|")) { // break for connectors
			if (current_word.at(current_word.size() - 1) == ';') { // append to vector
				if (current_word.substr(0, current_word.size() - 1) == "history") { // check for history command
					++num_parses;
					return new HistoryCommand(command_history);
				}
				else if (current_word == "help") { // check for help command
					++num_parses;
					return new HistoryCommand();
				}
				current_word = current_word.substr(0, current_word.size() - 1);
				cur_v.push_back(convertStrToChar(current_word));
				++num_parses;
			}
			else if (current_word.size() >= 1 && current_word == "]") { // stat command alternative ending
				cur_v.push_back(convertStrToChar(current_word));
				++num_parses;
			}

			break;
		}
		else if (current_word.size() >= 1 && (current_word == ">" || current_word == ">>" || current_word == "<")) { // create a new redirection
			Connector* new_connector_cmd = 0;
			if (current_word == ">") { // for ouput
				new_connector_cmd = new Output();
			}
			else if (current_word == ">>") { // for append
				new_connector_cmd = new Append();
			}
			else { // for input
				new_connector_cmd = new Input();
			}
			new_connector_cmd->setLeftChild(new DefaultCommand(VectorToChar(cur_v))); // left-side has cmd and files

			++nextParse; // go to next token
			++num_parses; //  increment parses
			new_connector_cmd->setRightChild(new DefaultCommand(getRightSide(nextParse, originalToken, num_parses))); // get right-side files
			return new_connector_cmd; // new connector
		}
		else if (current_word.size() == 2 && current_word == "cd") { // create new change directory command
			++num_parses;
			++nextParse;
			return new ChangeDirectory(getRightSide(nextParse, originalToken, num_parses), current_path);
		}
		else if (current_word.size() == 7 && current_word == "history") { // create new history command
			++num_parses;
			return new HistoryCommand(command_history);
		}
		else if (current_word.size() == 4 && current_word == "help") { // create new help command
			++num_parses;
			return new HelpCommand();
		}
		else { // add char* to vector
			cur_v.push_back(convertStrToChar(current_word)); // append char**
			++num_parses; // inc. num parses
		}
	}
	return new DefaultCommand(VectorToChar(cur_v)); // convert vector's contents to char**
}

char* Parser::convertStrToChar(const string &current_word) {
	char* current_char_ar = new char[current_word.size() + 1]; // total word size + 1 (+1 for NULL)

	for (unsigned int i = 0; i < current_word.size(); ++i) { // transfer each char from str to char*
		current_char_ar[i] = current_word.at(i);
	}

	current_char_ar[current_word.size()] = '\0'; // append NULL char
	return current_char_ar;
}

void Parser::updateParser(boost::tokenizer<boost::char_separator<char> >::iterator& initial_position, int current_parse, int & number_parses, vector<char*> & all_cmds, Command* new_cmd) {
	if (!new_cmd) {return;} // update is not necessary
	while(current_parse != (number_parses - 1)) { // move current position to new position
		++initial_position;
		++current_parse;
	}

	all_cmds.resize(0); // reset vector
	number_parses = 0; // reset parses
	current_commands->push_back(new_cmd); // add new cmd to vector
	return;
}

void Parser::checkUserFailure() {
	if (errno == 0 || errno == ENOENT || errno == EBADF || errno == EPERM) { // errno was set; send to stdout the error message
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

string Parser::getUserInput() {
	return user_input;
}

void Parser::setUserInput(const string & new_input) {
	user_input = new_input;
	return;
}

void Parser::updatePath(char* full_path) {
	string str_full_path(full_path); // convert to str
	boost::char_separator<char> sep("/");
	boost::tokenizer<boost::char_separator<char> > tok(str_full_path, sep); // set tokenizer
	
	updatePath(); // remove previous PATH for new PATH
	
	for (boost::tokenizer<boost::char_separator<char> >::iterator start = tok.begin(); start != tok.end(); ++start) {
		current_path.push(*start);
	}

	return;
}

void Parser::updatePath(stack<string> & new_directory) {
	current_path = new_directory; // set directory
	return;
}


void Parser::printPath(char* full_path, struct passwd* current_user) {
	if (full_path && current_user) {
		string current_directory(full_path);
		size_t current_user_index = current_directory.find(current_user->pw_name); // check if current user is in PATH
		if (current_user_index != string::npos) { // send to std::out directory from current user up to current directory
			cout << current_directory.substr(current_user_index); // print directory contents
		}
		else {
			cout << current_path.top(); // current user is not within the PATH
		}
	}

	return;
}

void Parser::checkPathFailure() {
	if (errno == EACCES) { // errno is set
		perror("Permission to read or search a component of the filename was denied.");
		
	}
	else if (errno == ENOENT) {
		perror("The current working directory has been unlinked.");
	}
	return;
}

void Parser::checkHostNameFailure() {
	if (errno == EFAULT) { // determine error code
		perror("Name is an invalid address.");
	}
	else if (errno == EINVAL) {
		perror("Either the len is negative or len is larger than the maximum allowed size.");
	}
	else if (errno == ENAMETOOLONG) {
		perror("Len is smaller than the actual size.");
	}
	return;
}

void Parser::updatePath() {
	while (!current_path.empty()) { // remove entire directory
		current_path.pop();
	}
	return;
}

