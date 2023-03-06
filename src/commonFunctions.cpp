#include <string_view>
#include <string>
#include <filesystem>
#include <array>
#include <fstream>
#include "commonFunctions.hpp"

namespace fs = std::filesystem;

std::filesystem::path getInstancesDir(int nAgents, int nTasks, const std::filesystem::path &instancesRoot) {
    std::string dirId{"a" + std::to_string(nAgents) + "_t" + std::to_string(nTasks)};

    return instancesRoot / dirId;
}

std::filesystem::path
getFilePath(int nAgents, int nTasks, int instanceId, const std::filesystem::path& instancesRoot, std::string_view extension,
            std::string_view methodFolder) {

    std::string fileId{std::to_string(instanceId) + "." + extension.data()};

    return getInstancesDir(nAgents, nTasks, instancesRoot) / methodFolder / fileId;
}

bool fileExists(int nAgents, int nTasks, int instanceId, std::string_view instancesRoot) {
    bool exists = true;

    std::array extensions{"agents", "tasks"};

    for (const auto& ex : extensions){
        std::ifstream file{getFilePath(nAgents, nTasks, instanceId, instancesRoot, ex)};
        exists = exists && file.is_open();
    }

    return exists;
}

bool fileExists(int nAgents, int nTasks, int instanceId, std::string_view instancesRoot, std::string_view method){
    std::ifstream file{getFilePath(nAgents, nTasks, instanceId, instancesRoot, "instance", method)};
    return file.is_open();
}

bool fileExists(const std::filesystem::path& filePath){
    std::ifstream file{filePath};
    return file.is_open();
}