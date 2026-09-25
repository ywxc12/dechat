#include "Config.hpp"
#include <iostream>
#include <string>
#include <type_traits>
#pragma once
std::string GetData(std::string&& Info,string Default){
    std::cout<<Info<<"["<<(Default)<<"]:";
    std::string tmp;
    std::getline(std::cin,tmp);
    return tmp;
}
