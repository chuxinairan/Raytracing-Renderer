#include "utils/profile.hpp"

#include <iostream>

Profile::Profile(const std::string& name) : name(name), start(std::chrono::high_resolution_clock::now()) {}

Profile::~Profile()
{
    auto duration = std::chrono::high_resolution_clock::now() - start;
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    std::cout << "Profile \""<< name << "\": " << milliseconds << "ms" <<std::endl;
}