#include "Node.h"

#include <iostream>

Node::Node() {
    wasCalculated_ = false;
    connections_.clear();
    storedValue_ = 0.0;
    wasCalculated_ = false;
}

Node::~Node() {
}

void Node::addConnection(Node* node, double weight) {
    connections_[node] = weight;
}

void Node::removeConnection(Node* node) {
    if (connections_.find(node) != connections_.end()) {
        connections_.erase(node);
    }
}

bool Node::isConnectedTo(Node* node) {
    if(node == this){

        return true;
    }
    for (auto& connection : connections_) {
        Node* nodePtr = connection.first;
        if (nodePtr->isConnectedTo(node)) {
            return true;
        }
    }
    return false;
}

double Node::getValue() {
    std::cout << "Calculating value for node " << this << std::endl;
    if (!wasCalculated_) {
        std::cout << "\tNot Calculated" << std::endl;
        storedValue_ = 0.0;
        for (auto& connection : connections_) {
            storedValue_ += connection.second * connection.first->getValue();
            std::cout << "\tNew Value: " << storedValue_ << std::endl;
        }
        setValue(storedValue_);
    }
    return storedValue_;
}

void Node::setValue(double value) {
    storedValue_ = value;
    wasCalculated_ = true;
}