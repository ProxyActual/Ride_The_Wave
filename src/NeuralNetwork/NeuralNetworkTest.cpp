#include <iostream>
#include <stdexcept>

#include "Node.h"
#include "Network.h"

std::string TestLoopCreation(){
	Network network(1, 1);
	Node* nodeA = new Node();
	Node* nodeB = new Node();
	Node* nodeC = new Node();

	network.addNode(nodeA, Network::HIDDEN);
	network.addNode(nodeB, Network::HIDDEN);
	network.addNode(nodeC, Network::HIDDEN);

	network.addConnection(nodeA, nodeB, 1.0);
	network.addConnection(nodeB, nodeC, 1.0);
	if(network.addConnection(nodeC, nodeA, 1.0)){
		return "ERROR: Loop creation should not be allowed";
	}

	return "Loop Creation Test Completed";
}

std::string NodeIsConnectedTest() {
	Node nodeA;
	Node nodeB;
	Node nodeC;
	nodeA.addConnection(&nodeB, 1.0);
	nodeB.addConnection(&nodeC, 1.0);
	if(nodeA.isConnectedTo(&nodeC)) {
		return "Node Is Connected Test Passed";
	} else {
		return "Node Is Connected Test Failed";
	}
}

std::string NetworkLargeTest(){
	Network network(1, 9);
	for(int i = 0; i < 10; ++i) {
		network.evolve();
	}
	network.getOutputs({1.0}); // Example input for the network
	return "Network Large Test Completed";
}

bool NetworkCopyTest() {
	Network assigned(3, 2);
	Network survivor(0, 0);
	{
		Network source(2, 1);
		auto* leaf = new Node();
		auto* left = new Node();
		auto* right = new Node();
		auto* output = new Node();
		source.addNode(leaf, Network::HIDDEN);
		source.addNode(left, Network::HIDDEN);
		source.addNode(right, Network::HIDDEN);
		source.addNode(output, Network::OUTPUT);
		leaf->setValue(4.0);
		source.addConnection(left, leaf, 2.0);
		source.addConnection(right, leaf, -0.5);
		source.addConnection(output, left, 0.25);
		source.addConnection(output, right, 3.0);

		Network copied(source);
		if (&(assigned = source) != &assigned) {
			return false;
		}
		leaf->setValue(100.0);
		if (source.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -100.0}) ||
			copied.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -4.0}) ||
			copied.getNetworkSize() != source.getNetworkSize()) {
			return false;
		}
		survivor = copied;

		output->setValue(9.0);
		Network cached(source);
		if (cached.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, 9.0})) {
			return false;
		}
	}

	if (assigned.getNetworkSize() != 7 ||
		assigned.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -4.0}) ||
		survivor.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -4.0}) ||
		&(assigned = assigned) != &assigned ||
		assigned.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -4.0})) {
		return false;
	}

	Network empty(0, 0);
	Network emptyCopy(empty);
	{
		Node external;
		Network invalid(0, 0);
		auto* output = new Node();
		invalid.addNode(output, Network::OUTPUT);
		output->addConnection(&external, 1.0);
		try {
			assigned = invalid;
			return false;
		} catch (const std::out_of_range&) {
			if (assigned.getNetworkSize() != 7 ||
				assigned.getOutputs({1.0, 2.0}) != std::vector<double>({0.0, -4.0})) {
				return false;
			}
		}
	}
	assigned = survivor = emptyCopy;
	return assigned.getNetworkSize() == 0 && survivor.getNetworkSize() == 0 &&
		assigned.getOutputs({}).empty() && survivor.getOutputs({}).empty();
}

int main() {
	if (!NetworkCopyTest()) {
		std::cerr << "Network Copy Test Failed" << std::endl;
		return 1;
	}
	std::cout << "Network Copy Test Passed" << std::endl;
	std::cout << TestLoopCreation() << std::endl;
	std::cout << NodeIsConnectedTest() << std::endl;
	std::cout << NetworkLargeTest() << std::endl;
   return 0;
}
