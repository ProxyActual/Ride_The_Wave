CORE_APP_SRC = AppMain.cpp \
	ConfigManager.cpp \
	RobinhoodConnector.cpp \
	CryptoHolding.cpp \

CORE_APP_LIBS = -lcurl -lsodium

THIRD_PARTY_DIR = ../3rd_parties
NLOHMANN_JSON_VERSION = v3.11.3
NLOHMANN_JSON_HEADER = $(THIRD_PARTY_DIR)/nlohmann/json.hpp

CXXFLAGS = -std=c++20 -I$(THIRD_PARTY_DIR)

appCore: $(CORE_APP_SRC) $(NLOHMANN_JSON_HEADER)
	g++ $(CXXFLAGS) -o appCore $(CORE_APP_SRC) $(CORE_APP_LIBS)

$(NLOHMANN_JSON_HEADER):
	mkdir -p $(dir $@)
	curl -fsSL -o $@ https://github.com/nlohmann/json/releases/download/$(NLOHMANN_JSON_VERSION)/json.hpp

clean:
	rm -f appCore