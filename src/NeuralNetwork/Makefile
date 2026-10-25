NETWORK_CORE = Node.cpp

NETWORK_LIB = -lcurl 

CXXFLAGS = -std=c++20

neuralNetwork: $(NETWORK_CORE)
	g++ $(CXXFLAGS) -o neuralNetwork $(NETWORK_CORE) $(NETWORK_LIB)

neuralNetworkTest: NeuralNetworkTest.cpp $(NETWORK_CORE)
	g++ $(CXXFLAGS) -o neuralNetworkTest NeuralNetworkTest.cpp $(NETWORK_CORE) $(NETWORK_LIB)

clean:
	rm -f neuralNetwork
	rm -f neuralNetworkTest