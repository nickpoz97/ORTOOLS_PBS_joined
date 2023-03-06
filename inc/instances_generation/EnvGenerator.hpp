#ifndef PBS_CPP_ENVGENERATOR_HPP
#define PBS_CPP_ENVGENERATOR_HPP

#include <string>
#include <iostream>
#include <vector>
#include <list>
#include <utility>
#include <random>
#include <forward_list>
#include <filesystem>
#include "BaseEnvGenerator.hpp"
#include "typeDefs.hpp"

class EnvGenerator : public BaseEnvGenerator{
public:
    explicit EnvGenerator(const std::string& gridPath);
    [[nodiscard]] const CoordVector& getEndpoints() const;

    // maps path, tasks path
    std::filesystem::path generateEnvs(int nInstances, int nAgents, int nTasks, std::string_view instancesPath);

private:
    static std::random_device rd;
    static std::mt19937 g;

    unsigned nEndpoints = 0;
    CoordVector endpoints;

    void initializeMatrix(std::fstream& data);
    void fillMatrix(std::fstream& data);

    CoordVector agentsCoords;
    TaskVector tasks;

    [[nodiscard]] static std::filesystem::path
    createInstancesDir(int nAgents, int nTasks, std::string_view instancesRoot) ;

    void saveMatrix(int instanceIndex, std::string_view path, int nAgents) const;
    void saveTasksInfo(int instanceIndex, const std::filesystem::path &path) const;

    void saveAgents(int instanceIndex, const std::filesystem::path &path);
};

#endif //PBS_CPP_ENVGENERATOR_HPP
