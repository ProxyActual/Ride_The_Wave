.PHONY: appCore
.PHONY: consoleUITest
.PHONY: historyTest


# Build the appCore target directly from the project root.
appCore:
	$(MAKE) -C src
	cp src/appCore .

consoleUITest:
	$(MAKE) -C src consoleUITest
	cp src/consoleUITest .

historyTest:
	$(MAKE) -C src historyTest
	cp src/historyTest .

clean:
	$(MAKE) -C src clean
	rm -f appCore
	rm -f consoleUITest
