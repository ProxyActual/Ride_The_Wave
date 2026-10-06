.PHONY: appCore

# Build the appCore target directly from the project root.
appCore:
	$(MAKE) -C src
	cp src/appCore .

consoleUITest:
	$(MAKE) -C src consoleUITest
	cp src/consoleUITest .

clean:
	$(MAKE) -C src clean
	rm -f appCore
	rm -f consoleUITest
