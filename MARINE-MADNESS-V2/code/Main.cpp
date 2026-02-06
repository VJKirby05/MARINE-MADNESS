#pragma once
#include "MarineMachine.h"
#include <iostream>

using namespace std;

// Main function
int main()
{
	// Declare instance of machine
	MarineMachine mar;

	// Start the machine
	mar.run();

	// Quit in the usual way when machine comes to a halt
	return 0;
}