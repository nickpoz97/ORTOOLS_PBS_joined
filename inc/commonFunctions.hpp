#ifndef CMAPD_COMMONFUNCTIONS_HPP
#define CMAPD_COMMONFUNCTIONS_HPP

#include <filesystem>

std::filesystem::path getInstancesDir(int nAgents, int nTasks, const std::filesystem::path &instancesRoot);
std::filesystem::path
getFilePath(int nAgents, int nTasks, int instanceId, const std::filesystem::path& instancesRoot, std::string_view extension,
            std::string_view methodFolder = "");
bool fileExists(int nAgents, int nTasks, int instanceId, std::string_view instancesRoot);
bool fileExists(int nAgents, int nTasks, int instanceId, std::string_view instancesRoot, std::string_view method);
bool fileExists(const std::filesystem::path& filePath);

#endif //CMAPD_COMMONFUNCTIONS_HPP
