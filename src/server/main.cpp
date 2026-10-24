#include "Server.hpp"
using namespace std;
// 获取带默认值的交互式输入
#include <iostream>
#include <string>
#include <limits>
#include <functional>
#include <algorithm>

// 基础版：只获取字符串，带默认值
std::string getInputStr(const std::string& prompt, const std::string& defaultValue) {
    std::cout << prompt << " [" << defaultValue << "]: ";
    std::string input;
    
    // 如果读取失败（比如 EOF），直接返回默认值
    if (!std::getline(std::cin, input)) {
        return defaultValue;
    }

    // 去除首尾空白字符（防止用户只敲了一个空格）
    input.erase(0, input.find_first_not_of(" \t\r\n"));
    input.erase(input.find_last_not_of(" \t\r\n") + 1);

    // 如果用户直接敲回车，返回默认值
    if (input.empty()) {
        return defaultValue;
    }
    return input;
}



int main(){
    string ae,io;
    ae=getInputStr("请输入证书文件位置","./server.crt");
    io=getInputStr("请输入私钥文件位置","./server.key");
    int port;
    auto pt=getInputStr("请输入使用的端口","53000");
    Server svr(stoi(pt),ae,io);
    svr.run();
    return 0;
}