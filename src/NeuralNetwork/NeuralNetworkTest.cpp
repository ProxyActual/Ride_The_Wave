#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
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

bool NetworkRandomWeightTest() {
	std::srand(0);
	Network network(1, 128);
	network.evolve();
	bool hasNegative = false;
	bool hasPositive = false;
	for (double output : network.getOutputs({1.0})) {
		hasNegative = hasNegative || output < 0.0;
		hasPositive = hasPositive || output > 0.0;
	}
	return hasNegative && hasPositive;
}

bool NetworkCalculateTest() {
	Network network(2, 0);
	auto* hidden = new Node();
	auto* output = new Node();
	network.addNode(hidden, Network::HIDDEN);
	network.addNode(output, Network::OUTPUT);
	network.addConnection(output, hidden, 2.0);
	hidden->setValue(5.0);
	// getOutputs keeps the stale cached hidden value; calculate recomputes it.
	bool cachedOk = network.getOutputs({1.0, 0.0}) == std::vector<double>({10.0});
	bool freshOk = network.calculate({1.0, 0.0}) == std::vector<double>({0.0});
	return cachedOk && freshOk;
}

bool NetworkSaveLoadTest() {
	std::srand(3);
	Network original(4, 2);
	for (int i = 0; i < 5; ++i) {
		original.evolve(0.3, 0.1);
		original.mutateWeights(0.5, 0.2);
	}
	const std::string path = "networkSaveLoadTest.net";
	if (!original.saveToFile(path)) {
		return false;
	}
	Network loaded(0, 0);
	if (!loaded.loadFromFile(path)) {
		return false;
	}
	std::remove(path.c_str());
	if (loaded.getNetworkSize() != original.getNetworkSize() || loaded.getInputCount() != 4 || loaded.getOutputCount() != 2) {
		return false;
	}
	for (const auto& inputs : std::vector<std::vector<double>>{{1, 2, 3, 4}, {-1, 0.5, 0, 2}}) {
		// Nodes sum their inputs in pointer order, which differs after a reload, so allow rounding error.
		const auto loadedOutputs = loaded.calculate(inputs);
		const auto originalOutputs = original.calculate(inputs);
		if (loadedOutputs.size() != originalOutputs.size()) {
			return false;
		}
		for (size_t i = 0; i < loadedOutputs.size(); ++i) {
			if (std::abs(loadedOutputs[i] - originalOutputs[i]) > 1e-9 * std::max(1.0, std::abs(originalOutputs[i]))) {
				return false;
			}
		}
	}
	Network untouched(1, 1);
	return !untouched.loadFromFile("does_not_exist.net") && untouched.getNetworkSize() == 2;
}

bool EvolveAdjustsExistingConnectionTest() {
	const unsigned seed = 7;
	Network network(0, 0);
	auto* input = new Node();
	auto* output = new Node();
	network.addNode(input, Network::INPUT);
	network.addNode(output, Network::OUTPUT);
	input->setValue(1.0);
	network.addConnection(output, input, 0.5);

	// evolve(1.0, 0.0) adds hidden node H, then tries each pair in order
	// (H,H) (H,input) (output,H) (output,input), drawing a weight then a roll for each.
	std::srand(seed);
	std::vector<double> weights;
	for (int pair = 0; pair < 4; ++pair) {
		weights.push_back(2.0 * static_cast<double>(std::rand()) / RAND_MAX - 1.0);
		std::rand();
	}
	const double expected = weights[2] * weights[1] + (0.5 + weights[3]);

	std::srand(seed);
	network.evolve(1.0, 0.0);
	const auto outputs = network.calculate({});
	return outputs.size() == 1 && std::abs(outputs[0] - expected) < 1e-12;
}

int main() {
	if (!EvolveAdjustsExistingConnectionTest()) {
		std::cerr << "Evolve Adjusts Existing Connection Test Failed" << std::endl;
		return 1;
	}
	std::cout << "Evolve Adjusts Existing Connection Test Passed" << std::endl;
	if (!NetworkCalculateTest()) {
		std::cerr << "Network Calculate Test Failed" << std::endl;
		return 1;
	}
	std::cout << "Network Calculate Test Passed" << std::endl;
	if (!NetworkSaveLoadTest()) {
		std::cerr << "Network Save/Load Test Failed" << std::endl;
		return 1;
	}
	std::cout << "Network Save/Load Test Passed" << std::endl;
	if (!NetworkRandomWeightTest()) {
		std::cerr << "Network Random Weight Test Failed" << std::endl;
		return 1;
	}
	std::cout << "Network Random Weight Test Passed" << std::endl;
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
