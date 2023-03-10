#include <boost/program_options.hpp>
#include <filesystem>
#include <string>
#include <chrono>

#include "instances_evaluation/OrtoolsEnv.hpp"
#include "Instance.h"
#include "PBS.h"

int main(int argc, char** argv){
    namespace po = boost::program_options;
    namespace fs = std::filesystem;

    fs::path exeCommand = fs::path(argv[0]);
    fs::path exeDir = (exeCommand.is_absolute() ? exeCommand : fs::current_path() / exeCommand).remove_filename();

    auto defaultGridPath = exeDir / "data" / "grid.txt";
    auto defaultDMPath = exeDir / "data" / "distance_matrix.npy";

    po::options_description desc("Allowed options");
    desc.add_options()
        ("help", "produce help message (use absolute paths or paths relative to working directory)")
        ("grid_path", po::value<std::string>()->default_value(defaultGridPath), "instancesPath of grid file")
        ("dm_path", po::value<std::string>()->default_value(defaultDMPath), "instancesPath of distance matrix")
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

    Instance instance(
        env.getGrid(vm["grid_path"].as<string>()),
        env.solve(vm["c"].as<int>()),
        env.getNRows(),
        env.getNCols()
    );

    PBS pbs(instance, true, 0);

    srand(0);
    pbs.solve(7200);

    auto end = std::chrono::steady_clock::now();
    std::chrono::duration<double> elapsed = end - start;

    std::cout << "Time:\t" << elapsed.count() << "\n";
    std::cout << "nA:\t" << env.getNAgents() << "\n";
    std::cout << "nT:\t" << env.getNTasks() << "\n";


    if (pbs.solution_found){
        pbs.printPaths();
    }


    pbs.clearSearchEngines();

    return 0;
}
