//
// Created by qlu on 2019/3/23.
//

#include <iostream>
#include <nlohmann/json.hpp>

double rounded(double x)
{
    char out[64];
    int n = 3;
    double xrounded;
    sprintf(out, "%.*f", n, x);
    xrounded = strtod(out, 0);
    return xrounded;
}

int main()
{
    double x = 1.0;
    nlohmann::json j;
    j["double"] = x;
    std::cout << j.dump() << std::endl;
    double y = 1111111111.11111111111;
    nlohmann::json k;
    k["double"] = rounded(y);
    std::cout << k.dump() << std::endl;
}