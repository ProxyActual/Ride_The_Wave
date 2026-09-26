.PHONY: appCore

# Build the appCore target directly from the project root.
appCore:
	$(MAKE) -C src
	cp src/appCore .

clean:
	$(MAKE) -C src clean
	rm -f appCore
