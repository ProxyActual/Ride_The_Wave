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

The test target links against the same static library. Header changes trigger
recompilation of dependent objects. `make -C src/NeuralNetwork clean` removes
the generated objects, dependency files, archive, and test executable.
