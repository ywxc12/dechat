#pragma once
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <cstring>

class AES {
public:
    // 16/24/32 字节分别对应 AES-128/192/256
    static constexpr size_t KEY_LEN = 32;  // AES-256
    static constexpr size_t IV_LEN  = 16;
    static constexpr size_t TAG_LEN = 16;  // GCM 认证标签

    // 构造时传入密钥（32字节）
    explicit AES(const std::vector<unsigned char>& key) : key_(key) {
        if (key_.size() != KEY_LEN)
            throw std::invalid_argument("AES-256 key must be 32 bytes");
    }

    // 从字符串派生密钥（方便使用）
    static std::vector<unsigned char> deriveKey(const std::string& password,
                                                const std::vector<unsigned char>& salt) {
        std::vector<unsigned char> key(KEY_LEN);
        if (!PKCS5_PBKDF2_HMAC(password.c_str(), password.size(),
                               salt.data(), salt.size(),
                               100000, EVP_sha256(),
                               KEY_LEN, key.data())) {
            throw std::runtime_error("PBKDF2 failed");
        }
        return key;
    }

    // 生成随机盐
    static std::vector<unsigned char> randomBytes(size_t len) {
        std::vector<unsigned char> buf(len);
        if (RAND_bytes(buf.data(), len) != 1)
            throw std::runtime_error("RAND_bytes failed");
        return buf;
    }

    // ==================== AES-256-GCM（推荐）====================
    // 输出格式: [IV(16)] [密文] [TAG(16)]
    std::vector<unsigned char> encryptGCM(const std::string& plain) {
        auto iv = randomBytes(IV_LEN);
        std::vector<unsigned char> cipher(plain.size() + TAG_LEN);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

        int len = 0, total = 0;

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
            throw std::runtime_error("EncryptInit failed");
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) != 1)
            throw std::runtime_error("Set IV len failed");
        if (EVP_EncryptInit_ex(ctx, nullptr, nullptr, key_.data(), iv.data()) != 1)
            throw std::runtime_error("Set key/iv failed");

        if (EVP_EncryptUpdate(ctx, cipher.data(), &len,
                              reinterpret_cast<const unsigned char*>(plain.data()),
                              plain.size()) != 1)
            throw std::runtime_error("EncryptUpdate failed");
        total = len;

        if (EVP_EncryptFinal_ex(ctx, cipher.data() + total, &len) != 1)
            throw std::runtime_error("EncryptFinal failed");
        total += len;

        // 取出 GCM 认证标签
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, TAG_LEN, cipher.data() + total) != 1)
            throw std::runtime_error("Get tag failed");

        EVP_CIPHER_CTX_free(ctx);

        // 拼接: IV + 密文 + TAG
        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), cipher.begin(), cipher.end());
        return result;
    }

    std::string decryptGCM(const std::vector<unsigned char>& data) {
        if (data.size() < IV_LEN + TAG_LEN)
            throw std::runtime_error("Cipher too short");

        const unsigned char* iv = data.data();
        const unsigned char* cipher = data.data() + IV_LEN;
        size_t cipherLen = data.size() - IV_LEN - TAG_LEN;
        const unsigned char* tag = data.data() + IV_LEN + cipherLen;

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        if (!ctx) throw std::runtime_error("EVP_CIPHER_CTX_new failed");

        int len = 0, total = 0;
        std::string plain(cipherLen, '\0');

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, nullptr, nullptr) != 1)
            throw std::runtime_error("DecryptInit failed");
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_IVLEN, IV_LEN, nullptr) != 1)
            throw std::runtime_error("Set IV len failed");
        if (EVP_DecryptInit_ex(ctx, nullptr, nullptr, key_.data(), iv) != 1)
            throw std::runtime_error("Set key/iv failed");

        if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&plain[0]), &len,
                              cipher, cipherLen) != 1)
            throw std::runtime_error("DecryptUpdate failed");
        total = len;

        // 设置预期的 TAG，解密时会自动校验
        if (EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, TAG_LEN,
                                const_cast<unsigned char*>(tag)) != 1)
            throw std::runtime_error("Set tag failed");

        // 如果 TAG 不匹配，Final 会失败 ← 数据被篡改！
        if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&plain[0]) + total, &len) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            throw std::runtime_error("Authentication failed: data tampered!");
        }
        total += len;
        plain.resize(total);

        EVP_CIPHER_CTX_free(ctx);
        return plain;
    }

    // ==================== AES-256-CBC（兼容旧系统）====================
    // 输出格式: [IV(16)] [密文]
    std::vector<unsigned char> encryptCBC(const std::string& plain) {
        auto iv = randomBytes(IV_LEN);
        std::vector<unsigned char> cipher(plain.size() + 16 + IV_LEN);

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        int len = 0, total = 0;

        if (EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv.data()) != 1)
            throw std::runtime_error("EncryptInit failed");

        if (EVP_EncryptUpdate(ctx, cipher.data() + IV_LEN, &len,
                              reinterpret_cast<const unsigned char*>(plain.data()),
                              plain.size()) != 1)
            throw std::runtime_error("EncryptUpdate failed");
        total = len;

        if (EVP_EncryptFinal_ex(ctx, cipher.data() + IV_LEN + total, &len) != 1)
            throw std::runtime_error("EncryptFinal failed");
        total += len;

        EVP_CIPHER_CTX_free(ctx);

        std::vector<unsigned char> result;
        result.insert(result.end(), iv.begin(), iv.end());
        result.insert(result.end(), cipher.begin(), cipher.begin() + IV_LEN + total);
        return result;
    }

    std::string decryptCBC(const std::vector<unsigned char>& data) {
        if (data.size() < IV_LEN) throw std::runtime_error("Cipher too short");

        const unsigned char* iv = data.data();
        const unsigned char* cipher = data.data() + IV_LEN;
        size_t cipherLen = data.size() - IV_LEN;

        EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
        int len = 0, total = 0;
        std::string plain(cipherLen + 16, '\0');

        if (EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key_.data(), iv) != 1)
            throw std::runtime_error("DecryptInit failed");

        if (EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char*>(&plain[0]), &len,
                              cipher, cipherLen) != 1)
            throw std::runtime_error("DecryptUpdate failed");
        total = len;

        if (EVP_DecryptFinal_ex(ctx, reinterpret_cast<unsigned char*>(&plain[0]) + total, &len) != 1)
            throw std::runtime_error("DecryptFinal failed (wrong key or corrupted)");
        total += len;
        plain.resize(total);

        EVP_CIPHER_CTX_free(ctx);
        return plain;
    }

private:
    std::vector<unsigned char> key_;
};