#ifndef PBS_CPP_ABSTRACTENV_HPP
#define PBS_CPP_ABSTRACTENV_HPP

#include <utility>
#include <vector>
#include "typeDefs.hpp"

class BaseEnvGenerator{
public:
    explicit BaseEnvGenerator() : matrix() {};
    explicit BaseEnvGenerator(Matrix  matrix) : matrix(std::move(matrix)) {};
    void printGrid() const;
    [[nodiscard]] const Matrix& getMatrix() const;

protected:
    static constexpr char ENDPOINT = 'G';
    static constexpr char OBSTACLE = '@';
    static constexpr char FLOOR = '.';
    static constexpr char ROBOT = 'r';

    unsigned nCols = 0;
    unsigned nRows = 0;

    Matrix matrix;
};

#endif //PBS_CPP_ABSTRACTENV_HPP
