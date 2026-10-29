#include "Network.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>
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

	if (inputCount_ > 0 && outputCount_ > 0) {
		addConnection(layers_[OUTPUT][0], layers_[INPUT][0], 1.0);
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

std::vector<double> Network::calculate(const std::vector<double>& inputs) {
	resetCalculatedFlags();
	return getOutputs(inputs);
}

void Network::resetCalculatedFlags() {
	for (auto* node : layers_[HIDDEN]) {
		node->resetCalculatedFlag();
	}
	for (auto* node : layers_[OUTPUT]) {
		node->resetCalculatedFlag();
	}
}

void Network::evolve(double connectionProbability, double deletionProbability) {
	addnewHiddenNodes(rand() % 10);
	buildRandomConnections(connectionProbability);
	removeRandomConnections(deletionProbability);
	cleanUpHiddenLayer();
}

void Network::mutateWeights(double mutationProbability, double magnitude) {
	for (auto& layer : layers_) {
		for (auto* node : layer) {
			for (auto& connection : node->connections_) {
				double randomValue = static_cast<double>(rand()) / RAND_MAX;
				if (randomValue < mutationProbability) {
					connection.second += magnitude * (2.0 * static_cast<double>(rand()) / RAND_MAX - 1.0);
				}
			}
		}
	}
}

bool Network::saveToFile(const std::string& path) const {
	std::unordered_map<const Node*, size_t> indices;
	size_t connectionCount = 0;
	for (const auto& layer : layers_) {
		for (const auto* node : layer) {
			indices.emplace(node, indices.size());
			connectionCount += node->connections_.size();
		}
	}

	const std::string tempPath = path + ".tmp";
	{
		std::ofstream file(tempPath, std::ios::trunc);
		if (!file) {
			return false;
		}
		file << "RideTheWaveNetwork 1\n";
		file << "inputs " << layers_[INPUT].size() << "\n";
		file << "hidden " << layers_[HIDDEN].size() << "\n";
		file << "outputs " << layers_[OUTPUT].size() << "\n";
		file << "connections " << connectionCount << "\n";
		file << std::setprecision(17);
		for (const auto& layer : layers_) {
			for (const auto* node : layer) {
				for (const auto& connection : node->connections_) {
					auto target = indices.find(connection.first);
					if (target == indices.end()) {
						return false;
					}
					file << indices.at(node) << " " << target->second << " " << connection.second << "\n";
				}
			}
		}
		if (!file.good()) {
			return false;
		}
	}
	return std::rename(tempPath.c_str(), path.c_str()) == 0;
}

bool Network::loadFromFile(const std::string& path) {
	std::ifstream file(path);
	if (!file) {
		return false;
	}

	std::string magic, label;
	int version = 0;
	size_t inputs = 0, hidden = 0, outputs = 0, connections = 0;
	if (!(file >> magic >> version) || magic != "RideTheWaveNetwork" || version != 1) {
		return false;
	}
	if (!(file >> label >> inputs) || label != "inputs" ||
		!(file >> label >> hidden) || label != "hidden" ||
		!(file >> label >> outputs) || label != "outputs" ||
		!(file >> label >> connections) || label != "connections") {
		return false;
	}

	Network loaded(static_cast<int>(inputs), static_cast<int>(outputs));
	loaded.addnewHiddenNodes(static_cast<int>(hidden));
	std::vector<Node*> nodes;
	for (auto& layer : loaded.layers_) {
		nodes.insert(nodes.end(), layer.begin(), layer.end());
	}

	for (size_t i = 0; i < connections; ++i) {
		size_t from = 0, to = 0;
		double weight = 0.0;
		if (!(file >> from >> to >> weight) || from >= nodes.size() || to >= nodes.size()) {
			return false;
		}
		if (!loaded.addConnection(nodes[from], nodes[to], weight)) {
			return false;
		}
	}

	swap(loaded);
	return true;
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
	// Only other unreachable hidden nodes can reference these nodes, and they are removed too.
	for (auto* node : nodesToClean) {
		delete node;
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
			double weight = 2.0 * static_cast<double>(rand()) / RAND_MAX - 1.0;
			double randomValue = static_cast<double>(rand()) / RAND_MAX; // Random value between 0 and 1
			if (randomValue < connectionProbability) {
				auto existing = fromNode->connections_.find(toNode);
				if (existing != fromNode->connections_.end()) {
					// Already connected: strengthen or weaken it by the new weight instead of replacing it.
					existing->second += weight;
				} else {
					addConnection(fromNode, toNode, weight);
				}
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