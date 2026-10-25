#include "Node.h"

Node::Node() {
    wasCalculated_ = false;
}

Node::~Node() {
}

void Node::addConnection(Node* node, double weight) {
    connections_[node] = weight;
}

void Node::removeConnection(Node* node) {
    connections_.erase(node);
}

bool Node::hasConnection(Node* node) {
    if(node == this){
        return true;
    }
    for (auto& connection : connections_) {
        Node* nodePtr = connection.first;
        if (nodePtr->hasConnection(node)) {
            return true;
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