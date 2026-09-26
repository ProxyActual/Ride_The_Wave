#include "RobinhoodConnector.h"

#include <curl/curl.h>
#include <sodium.h>

#include <cstdlib>
#include <ctime>
#include <stdexcept>
#include <string>
#include <cstring>

RobinhoodConnector::RobinhoodConnector() {
}

RobinhoodConnector::~RobinhoodConnector() {
}

std::string RobinhoodConnector::getOrders() {
    const char* apiKey = std::getenv("ROBINHOOD_API_KEY");
    const char* encodedSeed = std::getenv("ROBINHOOD_PRIVATE_KEY_BASE64");
    if (!apiKey || !encodedSeed) {
        throw std::runtime_error("Robinhood credentials are not set");
    }
    if (sodium_init() < 0) {
        throw std::runtime_error("libsodium initialization failed");
    }

    unsigned char seed[crypto_sign_SEEDBYTES];
    size_t seedLength = 0;
    if (sodium_base642bin(seed, sizeof(seed), encodedSeed,
                          std::strlen(encodedSeed), nullptr, &seedLength,
                          nullptr, sodium_base64_VARIANT_ORIGINAL) != 0 ||
        seedLength != sizeof(seed)) {
        throw std::runtime_error("Invalid Robinhood private-key seed");
    }

    unsigned char publicKey[crypto_sign_PUBLICKEYBYTES];
    unsigned char secretKey[crypto_sign_SECRETKEYBYTES];
    crypto_sign_seed_keypair(publicKey, secretKey, seed);
    sodium_memzero(seed, sizeof(seed));

    const std::string path = "/api/v1/crypto/trading/orders/";
    const std::string timestamp = std::to_string(std::time(nullptr));
    const std::string message = std::string(apiKey) + timestamp + path + "GET";

    unsigned char signature[crypto_sign_BYTES];
    crypto_sign_detached(signature, nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(), secretKey);
    sodium_memzero(secretKey, sizeof(secretKey));

    char encodedSignature[sodium_base64_ENCODED_LEN(
        crypto_sign_BYTES, sodium_base64_VARIANT_ORIGINAL)];
    sodium_bin2base64(encodedSignature, sizeof(encodedSignature),
                      signature, sizeof(signature),
                      sodium_base64_VARIANT_ORIGINAL);

    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Could not initialize curl");
    }

    curl_slist* headers = nullptr;
    const auto addHeader = [&](const std::string& header) {
        curl_slist* updated = curl_slist_append(headers, header.c_str());
        if (!updated) {
            curl_slist_free_all(headers);
            curl_easy_cleanup(curl);
            throw std::runtime_error("Could not allocate HTTP headers");
        }
        headers = updated;
    };
    addHeader(std::string("x-api-key: ") + apiKey);
    addHeader("x-timestamp: " + timestamp);
    addHeader(std::string("x-signature: ") + encodedSignature);

    std::string response;
    const std::string url = "https://trading.robinhood.com" + path;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,
                     +[](char* data, size_t size, size_t count, void* context) -> size_t {
                         const size_t bytes = size * count;
                         static_cast<std::string*>(context)->append(data, bytes);
                         return bytes;
                     });
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    const CURLcode result = curl_easy_perform(curl);
    long status = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &status);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);

    if (result != CURLE_OK || status < 200 || status >= 300) {
        throw std::runtime_error("Robinhood orders request failed (HTTP " +
                                 std::to_string(status) + "): " + response);
    }
    return response;
}