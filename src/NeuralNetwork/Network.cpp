#include "Network.h"

#include <algorithm>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <utility>

Network::Network(int inputCount, int outputCount)
{
    inputCount_ = inputCount;
    outputCount_ = outputCount;
    layers_.resize(3); // INPUT, HIDDEN, OUTPUT
    // Initialize input layer
	for(int i = 0; i < inputCount_; ++i) {
		layers_[INPUT].push_back(new Node());
	}
    // Initialize output layer
	for(int i = 0; i < outputCount_; ++i) {
		layers_[OUTPUT].push_back(new Node());
	}
}

Network::Network(const Network& other) : Network(0, 0) {
	Network copy(0, 0);
	copy.inputCount_ = other.inputCount_;
	copy.outputCount_ = other.outputCount_;
	std::unordered_map<const Node*, Node*> nodes;

	for (size_t i = 0; i < other.layers_.size(); ++i) {
		for (const auto* node : other.layers_[i]) {
			auto clonedNode = std::make_unique<Node>(*node);
			copy.layers_[i].push_back(clonedNode.get());
			Node* cloned = clonedNode.release();
			nodes.emplace(node, cloned);
		}
	}

	for (auto& layer : copy.layers_) {
		for (auto* node : layer) {
			std::map<Node*, double> connections;
			for (const auto& connection : node->connections_) {
				connections.emplace(nodes.at(connection.first), connection.second);
			}
			node->connections_.swap(connections);
		}
	}

	swap(copy);
}

Network& Network::operator=(const Network& other) {
	if (this != &other) {
		Network copy(other);
		swap(copy);
	}
	return *this;
}

void Network::swap(Network& other) noexcept {
	layers_.swap(other.layers_);
	std::swap(inputCount_, other.inputCount_);
	std::swap(outputCount_, other.outputCount_);
}

Network::~Network() {
	for (auto& layer : layers_) {
		for (auto* node : layer) {
			delete node;
		}
	}
}

std::vector<double> Network::getOutputs(std::vector<double> inputs) {
	std::vector<double> outputs;
	if(inputs.size() != inputCount_) {
		std::cerr << "Error: Number of inputs does not match input layer size." << std::endl;
		return outputs;
	}
	for (size_t i = 0; i < inputCount_; ++i) {
		layers_[INPUT][i]->setValue(inputs[i]);
	}
	for (auto& node : layers_[OUTPUT]) {
		outputs.push_back(node->getValue());
	}
	return outputs;
}

void Network::evolve() {
	addnewHiddenNodes(1); // Example: Add one new hidden node before evolving connections
	buildRandomConnections();
	removeRandomConnections(); // Example: Remove random connections after building new ones
	cleanUpHiddenLayer();
	// Implement the evolution logic for the network here
}

bool Network::addNode(Node* node, LayerType layer) {
	if (node) {
		switch (layer) {
			case INPUT:
				layers_[INPUT].push_back(node);
				return true;
			case HIDDEN:
				layers_[HIDDEN].push_back(node);
				return true;
			case OUTPUT:
				layers_[OUTPUT].push_back(node);
				return true;
			default:
				return false;
		}
	}
	return false;
}

bool Network::addConnection(Node* from, Node* to, double weight) {
	if (from && to) {
		if (!to->isConnectedTo(from)) {
			from->addConnection(to, weight);
			return true;
		}
	}
	return false;
}

void Network::cleanUpHiddenLayer() {
	std::vector<Node*> nodesToClean;
	for (auto* node : layers_[HIDDEN]) {
		bool shouldClean = true;
		for (auto* outputNode : layers_[OUTPUT]) {
			if (outputNode->isConnectedTo(node)) {
				shouldClean = false;
				break;
			}
		}
		if (shouldClean) {
			nodesToClean.push_back(node);
		}
	}	

	for (auto* node : nodesToClean) {
		layers_[HIDDEN].erase(
			std::remove_if(
				layers_[HIDDEN].begin(),
				layers_[HIDDEN].end(),
				[node](Node* n) {
					return n == node;
				}
			),
			layers_[HIDDEN].end()
		);
	}
}

void Network::addnewHiddenNodes(int count) {
	for (int i = 0; i < count; ++i) {
		layers_[HIDDEN].push_back(new Node());
	}
}

void Network::buildRandomConnections(double connectionProbability) {
	std::vector<Node*> connectionsFrom;
	for (auto* hiddenNode : layers_[HIDDEN]) {
		connectionsFrom.push_back(hiddenNode);
	}
	for (auto* outputNode : layers_[OUTPUT]) {
		connectionsFrom.push_back(outputNode);
	}

	std::vector<Node*> connectionsTo;
	for (auto* hiddenNode : layers_[HIDDEN]) {
		connectionsTo.push_back(hiddenNode);
	}
	for (auto* inputNode : layers_[INPUT]) {
		connectionsTo.push_back(inputNode);
	}

	for (auto* fromNode : connectionsFrom) {
		for (auto* toNode : connectionsTo) {
			double weight = static_cast<double>(rand()) / RAND_MAX; // Random weight between 0 and 1
			double randomValue = static_cast<double>(rand()) / RAND_MAX; // Random value between 0 and 1
			if (randomValue < connectionProbability) {
				addConnection(fromNode, toNode, weight);
			}
		}
	}
}

void Network::removeRandomConnections(double deletionProbability) {
	std::vector<Node*> connectionsFrom;
	for (auto* hiddenNode : layers_[HIDDEN]) {
		connectionsFrom.push_back(hiddenNode);
	}
	for (auto* outputNode : layers_[OUTPUT]) {
		connectionsFrom.push_back(outputNode);
	}

	std::vector<Node*> connectionsTo;
	for (auto* hiddenNode : layers_[HIDDEN]) {
		connectionsTo.push_back(hiddenNode);
	}
	for (auto* inputNode : layers_[INPUT]) {
		connectionsTo.push_back(inputNode);
	}

	for (auto* fromNode : connectionsFrom) {
		for (auto* toNode : connectionsTo) {
			double randomValue = static_cast<double>(rand()) / RAND_MAX; // Random value between 0 and 1
			if (randomValue < deletionProbability && toNode != nullptr && fromNode != nullptr) {
				fromNode->removeConnection(toNode);
			}
		}
	}
}