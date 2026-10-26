#pragma once

#include <map>

class Node{
    public:
        Node();
        ~Node();

        void addConnection(Node* node, double weight);
        void removeConnection(Node* node);

        bool isConnectedTo(Node* node);

        double getValue();

        void setValue(double value);

        void resetCalculatedFlag(){
            wasCalculated_ = false;
        }

    private:
        std::map<Node*, double> connections_;
        bool wasCalculated_;

        double storedValue_;

};
