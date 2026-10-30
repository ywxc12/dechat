#include "Server.hpp"
#include <iostream>

int main(){
    std::string cert,pkey;
    std::cout<<"Please input certificates:";std::cin>>cert;
    std::cout<<"Please input private key:";std::cin>>pkey;
    int port;std::cout<<"Please input port:";
    std::cin>>port;
    Server svr(port,cert,pkey);
    std::cout<<"Server start.\n";
    svr.run();
    return 0;
}