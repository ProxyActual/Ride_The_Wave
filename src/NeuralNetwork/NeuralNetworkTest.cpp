#include <iostream>

#include "Node.h"

int main() {
	std::cout << "Neural Network Test Started" << std::endl;
	
	Node input1_node;
	Node input2_node;
	Node output_node;
	output_node.addConnection(&input1_node, 1.0);
	output_node.addConnection(&input2_node, 1.0);

	input1_node.setValue(1.0);
	input2_node.setValue(1.0);

	if(output_node.getValue() == 2.0) {
		std::cout << "Test Passed" << std::endl;
	} else {
		std::cout << "Test Failed" << std::endl;
	}




    return 0;
}
