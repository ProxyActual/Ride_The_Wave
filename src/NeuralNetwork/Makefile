NETWORK_CORE = Node.cpp \
	Network.cpp

BUILD_DIR = build
NETWORK_OBJECTS = $(NETWORK_CORE:%.cpp=$(BUILD_DIR)/%.o)
NETWORK_LIBRARY = $(BUILD_DIR)/libneuralNetwork.a
NETWORK_DEPS = $(NETWORK_OBJECTS:.o=.d)

CXXFLAGS = -std=c++20

.PHONY: neuralNetwork clean

neuralNetwork: $(NETWORK_LIBRARY)

$(NETWORK_LIBRARY): $(NETWORK_OBJECTS)
	$(AR) rcs $@ $^

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@

$(BUILD_DIR):
	mkdir -p $@

neuralNetworkTest: NeuralNetworkTest.cpp Node.h Network.h $(NETWORK_LIBRARY)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ NeuralNetworkTest.cpp $(NETWORK_LIBRARY) $(LDLIBS)

clean:
	rm -f neuralNetwork
	rm -f neuralNetworkTest
	rm -f $(NETWORK_OBJECTS) $(NETWORK_DEPS) $(NETWORK_LIBRARY)

-include $(NETWORK_DEPS)