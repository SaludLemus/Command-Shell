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
#include <errno.h>

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
	if (!checkDupStatus(save_1)) { // dup status
			return false;
	}

	int open_file = open(right_side_files[0], O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);

	if (!checkOpenStatus(open_file)) { // open status
			return false;
	}

	int dup2_status = dup2(open_file, 1); // set file to fd[1]
	if (!checkDup2Status(dup2_status)) {
		close(open_file);
		dup2(save_1, 1);
		close(save_1);
		return false;
	}

	int close_status = close(open_file);
	if (!checkCloseStatus(close_status)) { // close statuss
		dup2(save_1, 1);
		close(save_1);
		return false;
	}

	if (!entire_command->execute()) { // execute
		execute_value = false;
	}

	// revert fds to normal
	dup2(save_1, 1);

	int close_status = close(save_1);
	if (!checkCloseStatus(close_status)) {;} // close status

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
	if (!checkDupStatus(save_1)) { // dup status
		return false;
	}

	int open_file = open(right_side_files[0], O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (!checkOpenStatus(open_file)) { // open status
		return false;
	}

	int dup2_status = dup2(open_file, 1); // set file to fd[1]
	if (!checkDup2Status(dup2_status)) {
		close(open_file);
		dup2(save_1, 1);
		close(save_1);
		return false;
	}

	int close_status = close(open_file);
	if (!checkCloseStatus(close_status)) { // close status
		dup2(save_1, 1);
		close(save_1);
		return false;
	}

	if (!entire_command->execute()) { // execute
		execute_value = false;
	}

	// revert fds to normal
	dup2(save_1, 1);

	int close_status = close(save_1);
	if (!checkCloseStatus(close_status)) {;} // close status

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

//	if (input_file == -1) {
//		perror("Input file does not exist.");
//		return false;
//	}
	if (!checkOpenStatus(input_file)) {
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


bool checkDupStatus(int dup_status) {
	if (dup_status == -1) { // check errno
		if (errno == EBADF) {
			perror("Oldfd isn't an open file descriptor or newfd is out of the allowed range for file descriptors.");
		}
		else if (errno == EMFILE) {
			perror("The per-process limit on the number of open file descriptors has been reached.");
		}
		else {
			perror("Dup2() failed.");
		}
		return false;
	}

		return true;
}


bool checkCloseStatus(int close_status) {
	if (close_status == -1) { // check errno
		if (errno == EBADF) {
			perror("Fd isn't a valid open file descriptor.");
		}
		else if (errno == EINTR) {
			perror("The close() call was interrupted by a signal");
		}
		else if (errno == EIO) {
			perror("An I/O error occurred.");
		}
		else {
			perror("Pipe() failed.");
		}
		return false;
	}

	return true;
}


bool checkPipeStatus(int pipe_status) {
	if (pipe_status == -1) { // check errno
		if (errno == EFAULT) {
			perror("Pipefd is not valid");
		}
		else if (errno == EMFILE) {
			perror("The per-process limit on the number of open file descriptors has been reached.");
		}
		else if (errno == ENFILE) {
			perror("The system-wide limit on the total number of open files has been reached.");
		}
		else {
			perror("Pipe() failed.");
		}
		return false;
	}

	return true;
}


bool checkOpenStatus(int open_status) {
	if (open_status == -1) { // check errno
		if (errno == EACCES) {
			perror("The requested access to the file is not allowed or file does not exist");
		}
		else if (errno == EFAULT) {
			perror("PATHNAME points outside your accessible address space.");
		}
		else if (errno == ENFILE) {
			perror("The system-wide limit on the total number of open files has been reached.");
		}
		else {
			perror("Open() failed.");
		}
		return false;
	}

	return true;
}


bool checkDup2Status(int dup2_status) {
	if (dup2_status == -1) { // check errno
		if (errno == EBADF) {
			perror("Oldfd isn't an open file descriptor or newfd is out of the allowed range for file descriptors.");
		}
		else if (errno == EBUSY) {
			perror("This may be returned by dup2() or dup3() during a race condition with open(2) and dup().");
		}
		else if (errno == EINTR) {
			perror("Dup2() call was interrupted by a signal;");
		}
		else if (errno == EMFILE) {
			perror("The per-process limit on the number of open file descriptors has been reached.");
		}
		else {
			perror("Dup2() failed.");
		}
		return false;
	}

	return true;
}








