#pragma once

#include "Node.h"
#include <string>
#include <vector>

class Network {
	public:
		enum LayerType {
			INPUT = 0,
			HIDDEN,
			OUTPUT
		};

		Network(int inputCount, int outputCount);
		Network(const Network& other);
		Network& operator=(const Network& other);
		~Network();

		std::vector<double> getOutputs(std::vector<double> inputs);
		// Like getOutputs, but clears cached hidden/output values first so results reflect the inputs.
		std::vector<double> calculate(const std::vector<double>& inputs);

		void evolve(double connectionProbability = 0.5, double deletionProbability = 0.5);
		// Nudges each existing connection weight by up to +/- magnitude with the given probability.
		void mutateWeights(double mutationProbability, double magnitude);

		bool saveToFile(const std::string& path) const;
		// Replaces this network with the one stored in path. Leaves it unchanged on failure.
		bool loadFromFile(const std::string& path);

		bool addNode(Node* node, LayerType layer);
		bool addConnection(Node* from, Node* to, double weight);

		void resetCalculatedFlags();

		int getInputCount() const { return inputCount_; }
		int getOutputCount() const { return outputCount_; }

		int getNetworkSize(){
			return layers_[INPUT].size() + layers_[HIDDEN].size() + layers_[OUTPUT].size();
		}

	private:
		std::vector<std::vector<Node*>> layers_;
		int inputCount_;
		int outputCount_;

		void swap(Network& other) noexcept;
		void cleanUpHiddenLayer();
		void addnewHiddenNodes(int count);
		void buildRandomConnections(double connectionProbability = 0.5);
		void removeRandomConnections(double deletionProbability = 0.5);

};