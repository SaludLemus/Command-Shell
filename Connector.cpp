/*
 * Connector.cpp
 *
 *  Created on: Dec 20, 2017
 *      Author: salud
 */
#include "Connector.h"
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

And::And() : Connector() {}

And::~And() {
	if (leftChild) {
		delete leftChild;
	}
	if (rightChild) {
		delete rightChild;
	}
}

bool And::execute() {
	if (leftChild && leftChild->execute() && rightChild) { // exec left child and check return status; execute right child
		return rightChild->execute();
	}
	return false;
}

void And::display() {
	std::cout << "     &&" << std::endl;
	std::cout << ' ' << '/' << '\t' << ' ' << "\\" << std::endl;
	if (leftChild) {
		leftChild->display();
		std::cout << "\t ";
	}
	if (rightChild) {
		rightChild->display();
		std::cout << std::endl;
	}
	return;
}

void And::setLeftChild(Command* new_left_child) {
	leftChild = new_left_child;
	return;
}

void And::setRightChild(Command* new_right_child) {
	rightChild = new_right_child;
	return;
}

char** And::getALLCMDS() {
	return 0;
}

void And::setALLCMDS(char** new_cmds) {
	return;
}

Or::Or() : Connector() {}

Or::~Or() {
	if (leftChild) {
		delete leftChild;
	}
	if (rightChild) {
		delete rightChild;
	}
}

bool Or::execute() {
	if (leftChild && !leftChild->execute() && rightChild) { // execute left child; execute right child if leftchild failed
		return rightChild->execute();
	}
	return true;
}

void Or::display() {
	std::cout << "     ||" << std::endl;
	std::cout << ' ' << '/' << '\t' << ' ' << "\\" << std::endl;
	if (leftChild) {
		leftChild->display();
		std::cout << "\t ";
	}
	if (rightChild) {
		rightChild->display();
		std::cout << std::endl;
	}
	return;
}

void Or::setLeftChild(Command* new_left_child) {
	leftChild = new_left_child;
	return;
}

void Or::setRightChild(Command* new_right_child) {
	rightChild = new_right_child;
	return;
}

char** Or::getALLCMDS() {
	return 0;
}

void Or::setALLCMDS(char** new_cmds) {
	return;
}

Output::Output() : Connector(), entire_command(0) {}

Output::~Output() {
	if (leftChild) {
		delete leftChild;
		leftChild = 0;
	}
	if (rightChild) {
		delete rightChild;
		rightChild = 0;
	}
	if (entire_command) {
		entire_command->setALLCMDS(0);  // prevent double-free error
		delete entire_command;
		entire_command = 0;
	}
}

bool Output::execute() {
	if (!leftChild || !rightChild) {
		return false;
	}

	bool execute_value = true;

	char** right_side_files = rightChild->getALLCMDS();

	entire_command = new DefaultCommand(getALLCMDS()); // create new command

	// set fds
	int save_1 = dup(1); // duplicate fd[1]

	int open_file = open(right_side_files[0], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if (open_file == -1) { // open() failed
		perror("File does not exist.");
		return false;
	}

	dup2(open_file, 1); // set file to fd[1]
	close(open_file);

	if (!entire_command->execute()) { // execute
		execute_value = false;
	}

	// revert fds to normal
	dup2(save_1, 1);
	close(save_1);

	return execute_value;
}

void Output::display() {
	if (leftChild) {
		leftChild->display();
		cout << ' ' << '>' << ' ';
	}

	if (rightChild) {
		rightChild->display();
	}
	return;
}

void Output::setLeftChild(Command* new_command) {
	leftChild = new_command;
	return;
}

void Output::setRightChild(Command* new_command) {
	rightChild = new_command;
	return;
}

char** Output::getALLCMDS() { // for multiple redirections
	vector<char*> all_files;
	char** left_side_files = leftChild->getALLCMDS();
	char** right_side_files = rightChild->getALLCMDS();
	int index = 0;

	while (left_side_files[index]) { // get left-side files (includes cmd)
		all_files.push_back(left_side_files[index]); // append char*
		++index;

	}
	index = 1;

	while(right_side_files[index]) { // get right-side files
		all_files.push_back(right_side_files[index]); // append char*
		++index;
	}
	return VectorToChar(all_files);
}

void Output::setALLCMDS(char** new_cmds) {
	if (entire_command) { // dealloc mem.
		entire_command->setALLCMDS(0);
		delete entire_command;
		entire_command = 0;
	}

	entire_command = new DefaultCommand(new_cmds); // set data mem. to param.
	return;
}

char** VectorToChar(const vector<char*>& oldVector) {
	char** all_cmds = new char*[sizeof(char*) * (oldVector.size() + 1)]; // total size is current vector's size + 1 (+1 is for NULL)

	for (unsigned int i = 0; i < oldVector.size(); ++i) {
		all_cmds[i] = oldVector.at(i);
	}
	all_cmds[oldVector.size()] = 0;
	return all_cmds;
}

Append::Append() : Connector(), entire_command(0) {}

Append::~Append() {
	if (leftChild) {
		delete leftChild;
		leftChild = 0;
	}
	if (rightChild) {
		delete rightChild;
		rightChild = 0;
	}
	if (entire_command) {
		entire_command->setALLCMDS(0);  // prevent double-free error
		delete entire_command;
		entire_command = 0;
	}
}

bool Append::execute() {
	if (!leftChild || !rightChild) {
		return false;
	}

	bool execute_value = true;
	char** right_side_files = rightChild->getALLCMDS();

	entire_command = new DefaultCommand(getALLCMDS()); // create new command

	// set fds
	int save_1 = dup(1); // duplicate fd[1]

	int open_file = open(right_side_files[0], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);

	if (open_file == -1) { // open() failed
		perror("File does not exist.");
		return false;
	}

	dup2(open_file, 1); // set file to fd[1]
	close(open_file);

	if (!entire_command->execute()) { // execute
		execute_value = false;
	}

	// revert fds to normal
	dup2(save_1, 1);
	close(save_1);

	return execute_value;
}

void Append::display() {
	if (leftChild) {
		leftChild->display();
		cout << ' ' << '>' << ' ';
	}

	if (rightChild) {
		rightChild->display();
	}
	return;
}

void Append::setLeftChild(Command* new_command) {
	leftChild = new_command;
	return;
}

void Append::setRightChild(Command* new_command) {
	rightChild = new_command;
	return;
}

char** Append::getALLCMDS() { // for multiple redirections
	vector<char*> all_files;
	char** left_side_files = leftChild->getALLCMDS();
	char** right_side_files = rightChild->getALLCMDS();
	int index = 0;

	while (left_side_files[index]) { // get left-side files (includes cmd)
		all_files.push_back(left_side_files[index]); // append char*
		++index;

	}
	index = 1;

	while(right_side_files[index]) { // get right-side files
		all_files.push_back(right_side_files[index]); // append char*
		++index;
	}
	return VectorToChar(all_files);
}

void Append::setALLCMDS(char** new_cmds) {
	if (entire_command) { // dealloc mem.
		entire_command->setALLCMDS(0);
		delete entire_command;
		entire_command = 0;
	}

	entire_command = new DefaultCommand(new_cmds); // set data mem to param.
	return;
}

Pipe::Pipe() : Connector() {}

Pipe::~Pipe() {
	if (leftChild) {
		delete leftChild;
		leftChild = 0;
	}

	if (rightChild) {
		delete rightChild;
		rightChild = 0;
	}
}

bool Pipe::execute() {
	int change_redirection[2];
	bool execute_value = true;

	// call pipe()
	pipe(change_redirection);

	int save_1 = dup(1); // redirect output

	// change 1 to be fd from pipe
	dup2(change_redirection[1], 1); // set write-end of the pipe
	close(change_redirection[1]);

	// execute left child
	if (!leftChild->execute()) {
		close(change_redirection[0]);

		execute_value = false;
	}

	// restore 1 to its initial state
	dup2(save_1, 1);
	close(save_1);

	if (!execute_value) { // leftChild did not execute correctly
		return execute_value;
	}

	int save_0 = dup(0); // redirect input

	// change 0 to be fd from pipe
	dup2(change_redirection[0], 0);
	close(change_redirection[0]);

	// execute right child
	if (!rightChild->execute()) {
		execute_value = false;
	}

	// restore 0 to its initial state
	dup2(save_0, 0);
	close(save_0);

	return execute_value;
}

void Pipe::display() {
	if (leftChild) {
		leftChild->display();
		cout << " | ";
	}

	if (rightChild) {
		rightChild->display();
	}
	return;
}

void Pipe::setLeftChild(Command* new_left_child) {
	leftChild = new_left_child;
	return;
}

void Pipe::setRightChild(Command* new_right_child) {
	rightChild = new_right_child;
	return;
}

char** Pipe::getALLCMDS() {
	return 0;
}

void Pipe::setALLCMDS(char** new_cmds) {
	return;
}

Input::Input() : Connector(), entire_command(0) {}

Input::~Input() {
	if (leftChild) {
		delete leftChild;
		leftChild = 0;
	}

	if (rightChild) {
		delete rightChild;
		rightChild = 0;
	}

	if (entire_command) {
		entire_command->setALLCMDS(0); // prevent double-free error
		delete entire_command;
		entire_command = 0;
	}
}

bool Input::execute() {
	if (!leftChild || !rightChild) {
		return false;
	}
	bool execute_value = true;
	char** right_side_files = rightChild->getALLCMDS();

	entire_command = new DefaultCommand(getALLCMDS()); // generate new command with left-side and right-side combined

	int input_file = open(right_side_files[0], O_RDONLY, S_IRUSR | S_IWUSR); // open file in read-only

	if (input_file == -1) {
		perror("Input file does not exist.");
		return false;
	}

	int save_0 = dup(0); // save fd[0]

	dup2(input_file, 0); // copy input_file's fd into fd[0]
	close(input_file);

	if (!entire_command->execute()) { // set boolean value if failed
		execute_value = false;
	}

	dup2(save_0, 0); // restore fds
	close(save_0);

	return execute_value;
}

void Input::display() {
	if (leftChild) {
		leftChild->display();
		cout << ' ' << "< ";
	}

	if (rightChild) {
		rightChild->display();
	}
	return;
}

void Input::setLeftChild(Command* new_left_child) {
	leftChild = new_left_child;
	return;
}

void Input::setRightChild(Command* new_right_child) {
	rightChild = new_right_child;
	return;
}

char** Input::getALLCMDS() { // for multiple redirections
	vector<char*> all_files;
	char** left_side_files = leftChild->getALLCMDS();
	char** right_side_files = rightChild->getALLCMDS();
	int index = 0;

	while (left_side_files[index]) { // get left-side files (includes cmd)
		all_files.push_back(left_side_files[index]); // append char*
		++index;

	}
	index = 1; // start file one after first right-side

	while(right_side_files[index]) { // get right-side files
		all_files.push_back(right_side_files[index]); // append char*
		++index;
	}
	return VectorToChar(all_files);
}

void Input::setALLCMDS(char** new_cmds) {
	if (entire_command) { // dealloc mem.
		entire_command->setALLCMDS(0);
		delete entire_command;
		entire_command = 0;
	}

	entire_command = new DefaultCommand(new_cmds); // set data mem. to param.
	return;
}


