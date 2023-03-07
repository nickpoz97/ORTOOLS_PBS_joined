#include "instances_evaluation/OrtoolsEnv.hpp"
#include "commonFunctions.hpp"
#include "Instance.h"
#include "PBS.h"
#include <boost/program_options.hpp>
#include <string>
#include <chrono>

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

    Instance instance(
        env.getGrid(vm["grid_path"].as<string>()),
        result,
        env.getNRows(),
        env.getNCols()
    );

    PBS pbs(instance, true, 0);

    srand(0);
    pbs.solve(7200);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Time:\t" << elapsed.count() << "\n";
    if (pbs.solution_found){
        pbs.printPaths();
    }


    pbs.clearSearchEngines();

    return 0;
}
