/*
 * Command.h
 *
 *  Created on: Dec 17, 2017
 *      Author: salud
 */

#ifndef COMMAND_H_
#define COMMAND_H_

#include <vector>
#include <string>
#include <stack>
using namespace std;

class Command {
public:
	Command() {}
	virtual ~Command() {}
	virtual void display() = 0;
	virtual bool execute() = 0;
	virtual void setALLCMDS(char**) = 0;
	virtual char** getALLCMDS() = 0;
};

class AllCommands : public Command {
private:
	vector<Command*>* current_commands;
public:
	AllCommands();
	AllCommands(vector<Command*>*);
	~AllCommands();
	void display();
	bool execute();
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class DefaultCommand : public Command {
private:
	char** command;
public:
	DefaultCommand();
	DefaultCommand(char**);
	~DefaultCommand();
	bool execute();
	void display();
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class StatCommand : public Command {
private:
	char** command;
public:
	StatCommand();
	StatCommand(char**);
	~StatCommand();
	bool execute();
	void display();
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class ChangeDirectory : public Command {
private:
	char** command;
	stack<string> entire_directory;
public:
	ChangeDirectory();
	ChangeDirectory(char**, const stack<string> &);
	~ChangeDirectory();
	bool execute();
	void display();
	char** getALLCMDS();
	void setALLCMDS(char**);
};

#endif /* COMMAND_H_ */
