#include "instances_evaluation/OrtoolsEnv.hpp"
#include "commonFunctions.hpp"
#include "Instance.h"
#include "PBS.h"
#include <boost/program_options.hpp>
#include <string>
#include <chrono>
#include <boost/algorithm/string/join.hpp>

int main(int argc, char** argv){
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message (use absolute paths or paths relative to working directory)")
        ("grid_path", po::value<std::string>()->default_value("./data/grid.txt"), "instancesPath of grid file")
        ("dm_path", po::value<std::string>()->default_value("./data/distance_matrix.npy"), "instancesPath of distance matrix")
        ("a", po::value<std::string>()->required(), "Agents file")
        ("t", po::value<std::string>()->required(), "Tasks file")
        ("c", po::value<int>()->default_value(3), "Capacity of each Agent");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 1;
    }

    po::notify(vm);

    const std::filesystem::path tasksFile = vm["t"].as<std::string>();
    const std::filesystem::path agentsFile = vm["a"].as<std::string>();

    // Record start time
    auto start = std::chrono::steady_clock::now();

    OrtoolsEnv env(
            agentsFile,
            tasksFile,
            vm["dm_path"].as<std::string>()
    );

    auto result = env.solve(vm["c"].as<int>());

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    for (const auto& agentRoute : result){
        for(const auto& coord : agentRoute){
            std::cout << '(' << coord.first << ',' << coord.second << "),";
        }
        std::cout << '\n';
    }

    Instance instance(
        env.getGrid(vm["grid_path"].as<string>()),
        result,
        env.getNRows(),
        env.getNCols()
    );

    instance.printMap();

    PBS pbs(instance, true, 1);

    srand(0);
    pbs.solve(7200);

    if (pbs.solution_found){
        pbs.printPaths();
    }

    pbs.clearSearchEngines();

//    std::cout
//        << vm["a"].as<int>() << ","
//        << vm["t"].as<int>() << ","
//        << elapsed.count() << "\n";

    return 0;
}
