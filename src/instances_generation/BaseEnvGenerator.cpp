#include "instances_generation/BaseEnvGenerator.hpp"
#include <iostream>

void BaseEnvGenerator::printGrid() const{
    for (const auto& row : matrix){
        for (char cell : row){
            std::cout << cell;
        }
        std::cout << '\n';
    }
}

const Matrix &BaseEnvGenerator::getMatrix() const{
    return matrix;
}