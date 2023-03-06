#ifndef CMAPD_BASEENV_HPP
#define CMAPD_BASEENV_HPP

#include "instances_evaluation/BaseEnv.hpp"
#include <sstream>
#include <memory>
#include <fstream>
#include <cassert>
#include <numeric>
#include <filesystem>
#include "typeDefs.hpp"
#include "cnpy.h"
#include <vector>
#include <string_view>
#include <string>
#include <iostream>
#include <list>
#include <utility>
#include <random>
#include <forward_list>

class BaseEnv{
public:
    static int64_t from2Dto1D(int64_t x, int64_t y, size_t nCols);
    bool solve(int capacity, int makespan, const std::filesystem::path &outputDirPath = "") const;
    void solve(int capacity, const std::filesystem::path &outputDirPath = "") const;

protected:
    BaseEnv(const std::filesystem::path &agentsFilePath, const std::filesystem::path &taskFilePath,
            const std::filesystem::path &distanceMatrixPath);

    size_t nRows = 0;
    size_t nCols = 0;

    const CompressedCoordVector agents;
    const CompressedTasksVector tasks;
    const CompressedDistanceMatrix distanceMatrix;

    int64_t getMaxDistance() const;
private:
    static CompressedCoordVector extractRobotPositions(const std::filesystem::path &agentsFilePath, size_t &nRows, size_t &nCols);
    static CompressedTasksVector extractTasks(const std::filesystem::path &taskFilePath, size_t nCols);

    static CompressedDistanceMatrix loadReducedDistanceMatrix(const std::filesystem::path &distanceMatrixPath, const CompressedCoordVector &agents,
                                                              const CompressedTasksVector &tasks);
    static CompressedDistanceMatrix loadDistanceMatrix(const std::filesystem::path &distanceMatrixPath);
    static CompressedDistanceMatrix reduceMatrix(const CompressedCoordVector &agents, const CompressedTasksVector &tasks,
                                                 CompressedDistanceMatrix && distanceMatrix);
};

#endif //CMAPD_BASEENV_HPP
