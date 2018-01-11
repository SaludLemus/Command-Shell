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
		
	while (user_input.size() == 0) {
		cout << "[" << user_info->pw_name << "@test " << flush;
		printPath(full_path); // entire path
		cout << "]$ " << flush; // prompt

		getline(cin, user_input); // ask for user input
	}
	
	free(full_path); // deallocate PATH
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

			new_change_directory->execute(); // change PATH

			// update parser's PATH
			updatePath(new_change_directory->getDirectory());
			
			updateParser(start, current_parse, number_parses, all_cmds, 0); // update the parser
			
			delete new_change_directory; // dealloc mem
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
		else if (current_word.size() >= 1 && (current_word == ">" || current_word == ">>" || current_word == "<")) {
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
		else { // add char* to vector
			cur_v.push_back(convertStrToChar(current_word)); // append char**
			++num_parses; // inc. num parses
		}
	}
	return new DefaultCommand(VectorToChar(cur_v)); // convert vector's contents to char**
}

char* Parser::convertStrToChar(const string &current_word) {
	char* current_char_ar = new char[current_word.size() + 1]; // total word size + 1 (+1 for NULL)

	for (unsigned int i = 0; i < current_word.size(); ++i) {
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

	for (boost::tokenizer<boost::char_separator<char> >::iterator start = tok.begin(); start != tok.end(); ++start) {
		current_path.push(*start);
	}

	return;
}

void Parser::updatePath(stack<string> & new_directory) {
	current_path = new_directory; // set directory
	return;
}


void Parser::printPath(char* full_path) {
	if (full_path) {
		cout << current_path.top(); // current directory
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





