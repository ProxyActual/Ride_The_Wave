#include <iostream>

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
	for(int i = 0; i < 10000; ++i) {
		network.evolve();
		std::cout << "Network size after evolution " << network.getNetworkSize() << std::endl;
	}
	network.getOutputs({1.0}); // Example input for the network
	return "Network Large Test Completed";
}

int main() {
	std::cout << TestLoopCreation() << std::endl;
	std::cout << NodeIsConnectedTest() << std::endl;
	std::cout << NetworkLargeTest() << std::endl;
   return 0;
}


