#include "Node.h"

#include <iostream>
#include <unordered_set>
#include <vector>

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
    std::vector<Node*> pending{this};
    std::unordered_set<Node*> visited;
    while (!pending.empty()) {
        Node* current = pending.back();
        pending.pop_back();
        if (current == node) {
            return true;
        }
        if (!visited.insert(current).second) {
            continue;
        }
        for (auto& connection : current->connections_) {
            pending.push_back(connection.first);
        }
    }
    return false;
}

double Node::getValue() {
    if (!wasCalculated_) {
        storedValue_ = 0.0;
        for (auto& connection : connections_) {
            storedValue_ += connection.second * connection.first->getValue();
        }
        setValue(storedValue_);
    }
    return storedValue_;
}

void Node::setValue(double value) {
    storedValue_ = value;
    wasCalculated_ = true;
}