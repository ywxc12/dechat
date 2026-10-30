#pragma once

#include <iomanip>
#include <openssl/evp.h>
#include <sstream>
#include <string>

// Modern OpenSSL 3.x SHA-256 implementation
inline std::string sha256(const std::string& str) {
    EVP_MD_CTX* ctx = EVP_MD_CTX_new();
    if (!ctx) return "";

    // Initialize
    if (EVP_DigestInit_ex(ctx, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Update data
    if (EVP_DigestUpdate(ctx, str.c_str(), str.size()) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Finalize hash
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;
    if (EVP_DigestFinal_ex(ctx, hash, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(ctx);
        return "";
    }

    // Free contextW
    EVP_MD_CTX_free(ctx);

    // Convert to hex string
    std::stringstream ss;
    for (unsigned int i = 0; i < lengthOfHash; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return ss.str();
}