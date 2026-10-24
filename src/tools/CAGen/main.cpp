#include <iostream>
#include <memory>
#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <stdexcept>
#include <string>


// RAII 封装，防止内存泄漏
struct X509Deleter {
    void operator()(X509 *x) const { X509_free(x); }
};
struct EVPKeyDeleter {
    void operator()(EVP_PKEY *p) const {
        EVP_PKEY_free(p);
    }
};
struct EVPKeyCtxDeleter {
    void operator()(EVP_PKEY_CTX *c) const {
        EVP_PKEY_CTX_free(c);
    }
};

using X509Ptr = std::unique_ptr<X509, X509Deleter>;
using EVPKeyPtr =
    std::unique_ptr<EVP_PKEY, EVPKeyDeleter>;
using EVPKeyCtxPtr =
    std::unique_ptr<EVP_PKEY_CTX, EVPKeyCtxDeleter>;

void print_errors() { ERR_print_errors_fp(stderr); }

// 获取带默认值的交互式输入
std::string getInput(const std::string &prompt,
                     const std::string &defaultValue) {
    std::cout << prompt << " [" << defaultValue
              << "]: ";
    std::string input;
    std::getline(std::cin, input);
    if (input.empty()) {
        return defaultValue;
    }
    return input;
}

int main() {
    std::cout << "====================================="
                 "=====\n";
    std::cout << "        自签名 SSL 证书生成器\n";
    std::cout << "====================================="
                 "=====\n";

    // ==================== 交互式获取配置
    // ====================
    std::string country =
        getInput("请输入国家代码 (C)", "CN");
    std::string org =
        getInput("请输入组织名称 (O)", "DeChat");
    std::string cn = getInput(
        "请输入通用名称/域名 (CN)", "localhost");

    // SAN 扩展：允许通过域名或 IP 访问
    std::string san =
        getInput("请输入 SAN 扩展 (逗号分隔)",
                 "DNS:localhost,IP:127.0.0.1");

    std::string daysStr =
        getInput("请输入证书有效期 (天数)", "365");
    long validDays = 365;
    try {
        validDays = std::stol(daysStr);
    } catch (...) {
        validDays = 365;
    }

    std::string keyFile =
        getInput("请输入私钥保存文件名", "server.key");
    std::string crtFile =
        getInput("请输入证书保存文件名", "server.crt");

    std::cout << "\n--- 开始生成证书 ---\n";

    // 1. 生成 RSA 密钥对
    EVPKeyCtxPtr ctx(
        EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr));
    if (!ctx || EVP_PKEY_keygen_init(ctx.get()) <= 0) {
        print_errors();
        return 1;
    }
    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx.get(),
                                         2048) <= 0) {
        print_errors();
        return 1;
    }

    EVP_PKEY *raw_pkey = nullptr;
    if (EVP_PKEY_keygen(ctx.get(), &raw_pkey) <= 0) {
        print_errors();
        return 1;
    }
    EVPKeyPtr pkey(raw_pkey);
    std::cout << "[1/5] RSA 私钥生成成功 (2048位)\n";

    // 2. 创建 X509 证书结构
    X509Ptr x509(X509_new());
    if (!x509) {
        print_errors();
        return 1;
    }
    X509_set_version(x509.get(), 2); // X509v3
    ASN1_INTEGER_set(X509_get_serialNumber(x509.get()),
                     1);
    X509_gmtime_adj(X509_get_notBefore(x509.get()), 0);
    X509_gmtime_adj(X509_get_notAfter(x509.get()),
                    validDays * 24 *
                        3600L); // 天数换算成秒
    std::cout << "[2/5] X509 证书结构初始化成功\n";

    // 3. 设置证书主体和签发者
    X509_NAME *name = X509_get_subject_name(x509.get());
    X509_NAME_add_entry_by_txt(
        name, "C", MBSTRING_ASC,
        (const unsigned char *)country.c_str(), -1, -1,
        0);
    X509_NAME_add_entry_by_txt(
        name, "O", MBSTRING_ASC,
        (const unsigned char *)org.c_str(), -1, -1, 0);
    X509_NAME_add_entry_by_txt(
        name, "CN", MBSTRING_ASC,
        (const unsigned char *)cn.c_str(), -1, -1, 0);

    X509_set_issuer_name(x509.get(), name); // 自签名
    X509_set_pubkey(x509.get(), pkey.get());
    std::cout << "[3/5] 证书主体信息设置成功\n";

    // 4. 添加 SAN (Subject Alternative Name) 扩展
    X509V3_CTX v3_ctx;
    X509V3_set_ctx_nodb(&v3_ctx);
    X509V3_set_ctx(&v3_ctx, x509.get(), x509.get(),
                   nullptr, nullptr, 0);
    X509_EXTENSION *ext = X509V3_EXT_conf_nid(
        nullptr, &v3_ctx, NID_subject_alt_name,
        san.c_str());
    if (ext) {
        X509_add_ext(x509.get(), ext, -1);
        X509_EXTENSION_free(ext);
    }
    std::cout << "[4/5] SAN 扩展添加成功 (" << san
              << ")\n";

    // 5. 签名并保存
    if (!X509_sign(x509.get(), pkey.get(),
                   EVP_sha256())) {
        print_errors();
        return 1;
    }
    std::cout << "[5/5] 证书签名成功\n";

    FILE *f_key = fopen(keyFile.c_str(), "wb");
    if (f_key) {
        PEM_write_PrivateKey(f_key, pkey.get(), nullptr,
                             nullptr, 0, nullptr,
                             nullptr);
        fclose(f_key);
        std::cout << "-> 成功写入私钥: " << keyFile
                  << "\n";
    }

    FILE *f_crt = fopen(crtFile.c_str(), "wb");
    if (f_crt) {
        PEM_write_X509(f_crt, x509.get());
        fclose(f_crt);
        std::cout << "-> 成功写入证书: " << crtFile
                  << "\n";
    }

    std::cout << "--- 证书生成完毕！ ---\n";
    return 0;
}