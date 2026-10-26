#pragma once

#include "Node.h"
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

		void evolve();

		bool addNode(Node* node, LayerType layer);
		bool addConnection(Node* from, Node* to, double weight);

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