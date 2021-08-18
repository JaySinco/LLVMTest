#include <random>
#include <iostream>
#define EXPORT extern "C" __declspec(dllexport)

EXPORT double random()
{
    static std::random_device rd;
    static std::mt19937 e2(rd());
    static std::uniform_real_distribution<double> dist(0, 1);
    std::cout << "exported random called" << std::endl;
    return dist(e2);
}
