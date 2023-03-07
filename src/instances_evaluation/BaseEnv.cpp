#include <sstream>
#include <memory>
#include "cnpy.h"
#include <random>
#include <cstdint>
#include <algorithm>
#include <boost/tokenizer.hpp>
#include "instances_evaluation/BaseEnv.hpp"

int64_t BaseEnv::from2Dto1D(int64_t x, int64_t y, size_t nCols) {
    return y * static_cast<int64_t>(nCols) + x;
}

CompressedDistanceMatrix BaseEnv::loadDistanceMatrix(const std::filesystem::path &distanceMatrixPath) {
    const cnpy::NpyArray distanceMatrixObj = cnpy::npy_load(distanceMatrixPath);

    unsigned startCoordsSize = distanceMatrixObj.shape[0] * distanceMatrixObj.shape[1];
    unsigned endCoordsSize = distanceMatrixObj.shape[2] * distanceMatrixObj.shape[3];

    // distance matrix is considered double
    const auto* data = distanceMatrixObj.data<double>();
    CompressedDistanceMatrix distanceMatrix(startCoordsSize, std::vector<int64_t>(endCoordsSize));

    for (unsigned i = 0 ; i < startCoordsSize ; ++i){
        for (unsigned j = 0 ; j < endCoordsSize ; ++j){
            distanceMatrix[i][j] = static_cast<int>(data[i*startCoordsSize + j]);
        }
    }

    return distanceMatrix;
}

CompressedCoordVector BaseEnv::extractRobotPositions(const std::filesystem::path &agentsFilePath, size_t &nRows, size_t &nCols) {
    std::fstream fs;
    fs.open(agentsFilePath, std::ios::in);

    std::string line;
    std::getline(fs, line);

    // nRows,nCols line
    std::string nRowsString, nColsString;
    auto matrixSizeInfoStream = std::stringstream(line);
    std::getline(matrixSizeInfoStream, nRowsString, ',');
    std::getline(matrixSizeInfoStream, nColsString, ',');

    nRows = std::stoi(nRowsString);
    nCols = std::stoi(nColsString);

    // nAgents line
    std::getline(fs, line);

    CompressedCoordVector agents;
    size_t nAgents = std::stoi(line);
    agents.reserve(nAgents);

    for (size_t i = 0 ; i < nAgents; ++i){
        std::getline(fs, line);

        std::string xCoordString, yCoordString;
        auto coordStream = std::stringstream(line);
        std::getline(coordStream, yCoordString, ',');
        std::getline(coordStream, xCoordString, ',');

        agents.push_back(from2Dto1D(std::stoi(xCoordString), std::stoi(yCoordString), nCols));
    }

    return agents;
}

CompressedTasksVector BaseEnv::extractTasks(const std::filesystem::path &taskFilePath, size_t nCols) {
    std::fstream fs;
    fs.open(taskFilePath, std::ios::in);

    std::string line;
    std::getline(fs, line);

    int nTasks = std::stoi(line);
    CompressedTasksVector tasks;
    tasks.reserve(nTasks);


    for (int i = 0 ; i < nTasks ; ++i){
        std::getline(fs, line);

        std::stringstream coordsString{line};
        std::string coord;

        std::getline(coordsString, coord, ',');
        int yBegin = std::stoi(coord);

        std::getline(coordsString, coord, ',');
        int xBegin = std::stoi(coord);

        std::getline(coordsString, coord, ',');
        int yEnd = std::stoi(coord);

        std::getline(coordsString, coord, ',');
        int xEnd = std::stoi(coord);

        tasks.push_back({from2Dto1D(xBegin, yBegin, nCols), from2Dto1D(xEnd, yEnd, nCols)});
    }

    return tasks;
}

BaseEnv::BaseEnv(const std::filesystem::path &agentsFilePath, const std::filesystem::path &taskFilePath,
                 const std::filesystem::path &distanceMatrixPath) :
    agents{extractRobotPositions(agentsFilePath, nRows, nCols)},
    tasks{extractTasks(taskFilePath, nCols)},
    distanceMatrix{loadReducedDistanceMatrix(distanceMatrixPath, agents, tasks)}
    {}

CompressedDistanceMatrix BaseEnv::loadReducedDistanceMatrix(const std::filesystem::path &distanceMatrixPath, const CompressedCoordVector &agents,
                                                            const CompressedTasksVector &tasks) {
    return reduceMatrix(agents, tasks, loadDistanceMatrix(distanceMatrixPath));
}

CompressedDistanceMatrix BaseEnv::reduceMatrix(const CompressedCoordVector &agents, const CompressedTasksVector &tasks,
                                               CompressedDistanceMatrix && distanceMatrix) {
    // extract useful indices
    std::vector<int64_t> reducedIndices;
    reducedIndices.reserve(agents.size() + tasks.size() * 2);

    for(auto coord : agents){
        reducedIndices.push_back(coord);
    }

    for(auto [coordBegin,coordEnd] : tasks ){
        reducedIndices.push_back(coordBegin);
        reducedIndices.push_back(coordEnd);
    }

    size_t reducedSize = reducedIndices.size();

    CompressedDistanceMatrix reducedMatrix;

    reducedMatrix.reserve(reducedSize+1);

    for(auto i : reducedIndices){
        std::vector<int64_t> reducedRow;
        reducedRow.reserve(reducedSize+1);

        for(auto j : reducedIndices){
            reducedRow.push_back(distanceMatrix[i][j]);
        }
        // end location = actual agent location
        reducedRow.push_back(0);
        reducedMatrix.push_back(reducedRow);
    }
    reducedMatrix.push_back(std::vector<int64_t>(reducedMatrix.size()+1, 0));

    return reducedMatrix;
}

int64_t BaseEnv::getMaxDistance() const {
    int64_t maxDistance = 0;

    for(const auto& row : distanceMatrix){
        maxDistance = std::max(maxDistance, *std::max_element(row.cbegin(), row.cend()));
    }

    return maxDistance;
}

std::vector<bool> BaseEnv::getGrid(const std::filesystem::path &mapPath) const {
    using namespace boost;

    std::vector<bool> grid;

    std::ifstream myfile(mapPath, std::ios::in);
    if (!myfile.is_open())
        throw std::runtime_error("wrong grid file");

    std::string line;
    grid.reserve(nRows * nCols);

    // read map (&& start/goal locations)
    for (int i = 0; i < nRows; i++) {
        getline(myfile, line);
        for (int j = 0; j < nCols; j++) {
            grid[linearizeCoord({i, j})] = (line[j] == '@');
        }
    }

    return grid;
}

int BaseEnv::linearizeCoord(const Coord2D &coord) const {
    return coord.second + nCols * coord.first;
}

size_t BaseEnv::getNRows() const {
    return nRows;
}

size_t BaseEnv::getNCols() const {
    return nCols;
}
