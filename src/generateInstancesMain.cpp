#include <boost/program_options.hpp>
#include <iostream>
#include "instances_generation/EnvGenerator.hpp"

int main(int argc, char** argv){
    namespace po = boost::program_options;
    po::options_description desc("Allowed options");
    desc.add_options()
            ("help", "produce help message (use absolute paths or paths relative to working directory)")
            ("grid_path", po::value<std::string>()->default_value("./data/grid.txt"), "path of grid file")
            ("instances_root", po::value<std::string>()->default_value("./instances"), "path where instances will be placed")
            ("a", po::value<int>()->required(), "Number of Agents")
            ("t", po::value<int>()->required(), "Number of Tasks")
            ("n", po::value<int>()->required(), "Number of Instances");

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);

    if (vm.count("help")) {
        std::cout << desc << '\n';
        return 1;
    }

    po::notify(vm);

    const std::string instancesRoot = vm["instances_root"].as<std::string>();

    EnvGenerator grid(vm["grid_path"].as<std::string>() + "");
    auto path = grid.generateEnvs(
            vm["n"].as<int>(),
            vm["a"].as<int>(),
            vm["t"].as<int>(),
            instancesRoot
    );

    return 0;
}