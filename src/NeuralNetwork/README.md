# Neural network library

Build the reusable C++20 library from the repository root:

```sh
make -C src/NeuralNetwork neuralNetwork
```

This also works with the default target (`make -C src/NeuralNetwork`). It
produces:

- `build/Node.o` and `build/Network.o`: individual compiled object files.
- `build/libneuralNetwork.a`: a static library packaging those object files.

Other C++ programs include `Network.h` (or `Node.h`) and link the library:

```sh
g++ -std=c++20 -Isrc/NeuralNetwork main.cpp \
    src/NeuralNetwork/build/libneuralNetwork.a -o myApp
```

Place the library after the sources or object files that use it on the link
command. Alternatively, link both object files directly instead of the archive.
No test entry point is included in the library, and no curl dependency is needed.

To distribute the library, provide `Network.h`, `Node.h`, and
`build/libneuralNetwork.a`. Consumers need a compatible compiler, C++ standard
library, and target architecture; rebuild from source for other platforms.

Build the existing test executable with:

```sh
make -C src/NeuralNetwork neuralNetworkTest
```

`Network::evolve()` generates connection weights in the inclusive range
`[-1.0, 1.0]`, allowing both negative and positive contributions.

Copy a network using assignment or copy construction:

```cpp
Network original(2, 1);
Network copied(original);
Network assigned(3, 2);
assigned = original;
```

Both operations make independent deep copies of all layers, node values,
calculation flags, connections, and weights. Changing or destroying the original
does not affect its copies. Assignment replaces the destination's previous
contents and supports self-assignment. All connection endpoints must belong to
the network; copying a connection to an external node throws `std::out_of_range`
and leaves the assignment destination unchanged.

The test target links against the same static library. Header changes trigger
recompilation of dependent objects. `make -C src/NeuralNetwork clean` removes
the generated objects, dependency files, archive, and test executable.

## Evaluating, mutating, and persisting

`getOutputs()` reuses cached hidden/output node values. Use `calculate()` when
the same network is evaluated repeatedly with different inputs; it clears the
cached values first so the outputs always reflect the given inputs.

`evolve(connectionProbability, deletionProbability)` accepts optional rates
(both default to `0.5`). When evolution picks a pair of nodes that is already
connected, the new random weight is added to the existing weight (increasing or
decreasing it) instead of replacing it. `addConnection()` still sets the weight. `mutateWeights(probability, magnitude)` nudges each
existing weight by up to `±magnitude` with the given probability.

Save and reload a network as text:

```cpp
network.saveToFile("DOGE_network.txt");
Network restored(0, 0);
if (restored.loadFromFile("DOGE_network.txt")) { /* use restored */ }
```

`saveToFile()` writes to a temporary file and renames it into place.
`loadFromFile()` leaves the network unchanged if the file is missing or invalid.

## Coin predictor training

The application's `CoinPredictor` trains toward zero mean squared error (MSE).
Each scored day adds `(estimate - actual)^2` to the error total, then the total
is divided by the number of scored days. Changes are measured in percentage
points, so MSE is in squared percentage points: estimates/actuals of `1/1.5`,
`1/0.5`, and `-1/1.5` contribute `0.25`, `0.25`, and `6.25` respectively.
Errors never cancel each other out, and doubling a miss quadruples its penalty.

A candidate replaces the current network only when its MSE is lower. Equal
errors keep the existing network. Direction hit rate remains a separate display
statistic; it does not influence selection. The console's `AVG DIFF` still shows
mean absolute error (MAE) in percentage points for readability, not the squared
training score. Saved networks are re-scored with the MSE objective on startup.
Zero is the ideal score, not a guarantee of future prediction accuracy.

Run the scoring and selection regression tests from the repository root:

```sh
make -C src coinPredictorTest
```
