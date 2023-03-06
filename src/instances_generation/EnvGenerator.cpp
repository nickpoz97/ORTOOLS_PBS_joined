#include "instances_generation/EnvGenerator.hpp"
#include "commonFunctions.hpp"
#include <fstream>
#include <random>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <filesystem>
#include <cstdio>
#include <boost/algorithm/string.hpp>

using std::vector;
using std::pair;

std::random_device EnvGenerator::rd{};
std::mt19937 EnvGenerator::g{rd()};

EnvGenerator::EnvGenerator(const std::string& gridPath) : BaseEnvGenerator() {
    std::fstream fs;
    fs.open(gridPath.c_str(), std::ios::in);

    if(!fs.is_open()){
        std::cerr << "Wrong path for grid\n";
        exit(1);
    }

    initializeMatrix(fs);
    fillMatrix(fs);

    fs.close();
}

void EnvGenerator::initializeMatrix(std::fstream &data){
    std::string line;
    std::getline(data, line);

    if(line.empty()){
        exit(1);
    }

    // TODO check empty file
    using namespace boost::algorithm;

    trim(line);
    nCols = line.size();

    ++nRows;
    while(std::getline(data, line)){
        for (char c : line) {
            nEndpoints += (c == ENDPOINT);
        }
        ++nRows;
    }

#ifndef NDEBUG
    std::cout << "nRows: " << nRows << " ,nCols: " << nCols << " ,nEndpoints: " << nEndpoints << '\n';
#endif

    matrix = vector<vector<char>>(nRows, vector<char>(nCols));
    endpoints.reserve(nEndpoints);

#ifndef NDEBUG
    std::cout << "matrix rows: " << matrix.size() << " ,matrix cols: " << matrix[0].size() << '\n';
#endif
}

void EnvGenerator::fillMatrix(std::fstream& data) {
    data.clear();
    data.seekg(0, std::ios::beg);
    std::string line;

    for (int row = 0 ; row < nRows ; ++row){
        std::getline(data, line);
        for (int col = 0 ; col < nCols ; ++col){
            matrix[row][col] = line[col];
            if (line[col] == ENDPOINT){
                endpoints.emplace_back(pair{row, col});
            }
        }
    }

    assert(nEndpoints == endpoints.size());
}

const CoordVector &EnvGenerator::getEndpoints() const{
    return endpoints;
}

std::filesystem::path EnvGenerator::generateEnvs(int nInstances, int nAgents, int nTasks, std::string_view instancesPath) {
    const auto& saveDirPath = createInstancesDir(nAgents, nTasks, instancesPath);

    for (int i = 0 ; i < nInstances ; ++i){
        vector<int> indices(endpoints.size());
        std::iota(indices.begin(), indices.end(), 0);

        std::shuffle(indices.begin(), indices.end(), g);

//        for (int a = 0 ; a < nAgents ; ++a){
//            const auto& agentCoord = endpoints[indices[a]];
//            matrix[agentCoord.first][agentCoord.second] = ROBOT;
//        }

        agentsCoords.reserve(nAgents);

        for (int a = 0 ; a < nAgents ; ++a){
            agentsCoords.push_back(endpoints[indices[a]]);
        }
        saveAgents(i, saveDirPath);
        agentsCoords.clear();

//        for (int a = 0 ; a < nAgents ; ++a){
//            const auto& agentCoord = endpoints[indices[a]];
//            matrix[agentCoord.first][agentCoord.second] = ENDPOINT;
//        }

        tasks.reserve(nTasks);
        for (int t = nAgents ; t < nTasks * 2 + nAgents ; t+=2){
            const auto& taskBeginCoord = endpoints[indices[t]];
            const auto& taskEndCoord = endpoints[indices[t+1]];
            tasks.push_back(std::pair{taskBeginCoord, taskEndCoord});
        }
        saveTasksInfo(i, saveDirPath);
        tasks.clear();
    }

    return saveDirPath;
}

std::filesystem::path
EnvGenerator::createInstancesDir(int nAgents, int nTasks, std::string_view instancesRoot) {
    namespace fs = std::filesystem;

    auto dirPath = getInstancesDir(nAgents, nTasks, instancesRoot);

    fs::create_directories(dirPath);

    return dirPath;
}

void EnvGenerator::saveMatrix(int instanceIndex, std::string_view path, int nAgents) const {
    FILE* stream;
    const std::string filename{std::string{path} + "/" + std::to_string(instanceIndex) + ".map"};

#ifdef WIN32
    freopen_s(&stream, filename.data(), "w", stdout);
#else
    stream = freopen64(filename.data(), "w", stdout);
#endif

    std::cout << nRows << "," << nCols << '\n';
    std::cout << nEndpoints - nAgents << '\n';
    std::cout << nAgents << '\n';
    std::cout << 5000 << '\n';
    printGrid();

    fclose(stream);
}

void EnvGenerator::saveTasksInfo(int instanceIndex, const std::filesystem::path &path) const {
    FILE* stream;
    const auto filename{path / (std::to_string(instanceIndex) + ".tasks")};

#ifdef WIN32
    freopen_s(&stream, filename.generic_string().data(), "w", stdout);
#else
    stream = freopen64(filename.generic_string().data(), "w", stdout);
#endif

    std::cout << tasks.size() << '\n';
    for(const auto& task : tasks){
        const auto& taskBegin = task.first;
        const auto& taskEnd = task.second;

        std::cout << taskBegin.first << ',' << taskBegin.second << ','
            << taskEnd.first << ',' << taskEnd.second << '\n';

//        std::cout << "0\t" << taskBegin.first * nCols + taskBegin.second << '\t'
//            << taskEnd.first * nCols + taskEnd.second << "\t0\t0" << '\n';
    }
    fclose(stream);
}

void EnvGenerator::saveAgents(int instanceIndex, const std::filesystem::path &path) {
    FILE* stream;
    const auto filename{path / (std::to_string(instanceIndex) + ".agents")};

#ifdef WIN32
    freopen_s(&stream, filename.generic_string().data(), "w", stdout);
#else
    stream = freopen64(filename.generic_string().data(), "w", stdout);
#endif

    std::cout << matrix.size() << ',' << matrix[0].size() << '\n';
    std::cout << agentsCoords.size() << '\n';
    for(const auto& agentCoord : agentsCoords) {
        std::cout << agentCoord.first << ',' << agentCoord.second << '\n';
    }

    fclose(stream);
}
