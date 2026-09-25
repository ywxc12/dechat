#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>
#include <Windows.h>
#include "AES.h"  // 你的类定义

using namespace std;

// 十六进制输出
string toHex(const vector<unsigned char>& data) {
    stringstream ss;
    for (unsigned char c : data)
        ss << hex << setw(2) << setfill('0') << (int)c;
    return ss.str();
}

// 十六进制解析
vector<unsigned char> fromHex(const string& hex) {
    vector<unsigned char> data;
    for (size_t i = 0; i < hex.length(); i += 2) {
        string byte = hex.substr(i, 2);
        data.push_back(static_cast<unsigned char>(stoi(byte, nullptr, 16)));
    }
    return data;
}

int test() {
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    cout << "Encrypto v1.0\n";
    while (true) {
        cout << "\n模式: A-加密 B-解密 Q-退出\n";
        string mode;
        cin >> mode;
        if (mode == "Q" || mode == "q") break;

        if (mode == "A" || mode == "a") {
            cin.ignore(); // 清除换行符
            string plain, password;
            cout << "请输入明文: ";
            getline(cin, plain);
            cout << "请输入密码: ";
            getline(cin, password);

            // 随机盐
            auto salt = AES::randomBytes(16);
            auto key = AES::deriveKey(password, salt);
            AES aes(key);

            auto cipher = aes.encryptGCM(plain);

            // 输出: salt + cipher (salt 是明文存储的)
            vector<unsigned char> output;
            output.insert(output.end(), salt.begin(), salt.end());
            output.insert(output.end(), cipher.begin(), cipher.end());
            cout << "密文(hex): " << toHex(output) << endl;
        }
        else if (mode == "B" || mode == "b") {
            cin.ignore();
            string hexCipher, password;
            cout << "请输入密文(hex): ";
            getline(cin, hexCipher);
            cout << "请输入密码: ";
            getline(cin, password);

            try {
                auto data = fromHex(hexCipher);
                if (data.size() < 16 + 16 + 16) throw runtime_error("密文太短");
                // 提取盐
                vector<unsigned char> salt(data.begin(), data.begin() + 16);
                vector<unsigned char> cipher(data.begin() + 16, data.end());

                auto key = AES::deriveKey(password, salt);
                AES aes(key);
                string plain = aes.decryptGCM(cipher);
                cout << "明文: " << plain << endl;
            } catch (const exception& e) {
                cout << "解密失败: " << e.what() << endl;
            }
        }
    }
    return 0;
}