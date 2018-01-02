/*
 * Connector.h
 *
 *  Created on: Dec 18, 2017
 *      Author: salud
 */

#ifndef CONNECTOR_H_
#define CONNECTOR_H_

#include "Command.h"
#include <iostream>

bool checkDupStatus(int);
bool checkCloseStatus(int);
bool checkPipeStatus(int);
bool checkOpenStatus(int);
bool checkDup2Status(int);

char** VectorToChar(const vector<char*>& oldVector);

class Connector : public Command {
protected:
	Command* leftChild;
	Command* rightChild;
public:
	Connector() : Command() , leftChild(0), rightChild(0) {}
	virtual ~Connector() {}
	virtual void setLeftChild(Command*) = 0;
	virtual void setRightChild(Command*) = 0;
};

class And : public Connector {
public:
	And();
	~And();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class Or : public Connector {
public:
	Or();
	~Or();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class Output : public Connector {
private:
	Command* entire_command;
public:
	Output();
	~Output();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class Append : public Connector {
private:
	Command* entire_command;
public:
	Append();
	~Append();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class Input: public Connector {
private:
	Command* entire_command;
public:
	Input();
	~Input();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

class Pipe : public Connector {
public:
	Pipe();
	~Pipe();
	bool execute();
	void display();
	void setLeftChild(Command*);
	void setRightChild(Command*);
	char** getALLCMDS();
	void setALLCMDS(char**);
};

#endif /* CONNECTOR_H_ */
