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
	string user_input;
	vector<Command*>* current_commands;
	stack<string> current_path;
	char** getRightSide(boost::tokenizer<boost::char_separator<char> >::iterator , boost::tokenizer<boost::char_separator<char> >&, int &);
	Command* getNextCommand(boost::tokenizer<boost::char_separator<char> >::iterator , boost::tokenizer<boost::char_separator<char> >&, int &);
	char* convertStrToChar(const string &);
	void updateParser(boost::tokenizer<boost::char_separator<char> >::iterator&, int, int &, vector<char*>&, Command*);
	void checkUserFailure();
	void updatePath();
	void printPath(char*);
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
