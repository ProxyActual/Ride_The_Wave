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
