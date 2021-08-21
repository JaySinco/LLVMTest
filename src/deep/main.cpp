#include "prec.h"

int main(int argc, char **argv)
{
    torch::Tensor tensor = torch::rand({2, 3});
    std::cout << tensor << std::endl;
    std::cout << torch::cuda::is_available() << std::endl;
}
