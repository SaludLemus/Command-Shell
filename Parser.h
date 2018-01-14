/*
 * Parser.h
 *
 *  Created on: Dec 17, 2017
 *      Author: salud
 */

#ifndef PARSER_H_
#define PARSER_H_

#include <string>
#include <vector>
#include <stack>
#include <boost/tokenizer.hpp>
#include "Connector.h"
using namespace std;

class Parser {
private:
	string user_input; // what to do/ act on
	vector<Command*>* current_commands; // holds all the commands
	stack<string> current_path; // allows removal/adding of current directory
	char** getRightSide(boost::tokenizer<boost::char_separator<char> >::iterator , boost::tokenizer<boost::char_separator<char> >&, int &); // form next char** (right-side of expression)
	Command* getNextCommand(boost::tokenizer<boost::char_separator<char> >::iterator , boost::tokenizer<boost::char_separator<char> >&, int &); // // form next Command* (right-side of expression)
	char* convertStrToChar(const string &); // convert string to char*
	void updateParser(boost::tokenizer<boost::char_separator<char> >::iterator&, int, int &, vector<char*>&, Command*); // moves the iterator forward and updates vector
	void updatePath(char*); // converts current path (char*) into a stack
	void updatePath(stack<string>&); // command's stack will serve as input to change parser's stack
	void printPath(char*); // sent to stdout the current path
	void checkPathFailure(); // perror the source of error for the system call
	void checkUserFailure(); // perror the source of error for current user within the process
	void checkHostNameFailure(); // perror the source of error when retrieving the current host name of the process
public:
	Parser();
	Parser(const string&);
	~Parser();
	void parse(); // parse user's input
	vector<Command*>* getCommands(); // get all commands
	void askUser();
	string getUserInput();
	void setUserInput(const string &);
};
#endif /* PARSER_H_ */
