CORE_APP_SRC = AppMain.cpp \
	ConfigManager.cpp \
	RobinhoodConnector.cpp \

CORE_APP_LIBS = -lcurl -lsodium

appCore: $(CORE_APP_SRC)
	g++ -o appCore $(CORE_APP_SRC) $(CORE_APP_LIBS)

clean:
	rm -f appCore