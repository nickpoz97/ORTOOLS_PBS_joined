#ifndef PBS_CPP_ENV_HPP
#define PBS_CPP_ENV_HPP

#include <forward_list>
#include <random>
#include <utility>
#include <list>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include "instances_generation/EnvGenerator.hpp"
#include "ortools/constraint_solver/routing.h"
#include "ortools/constraint_solver/routing_enums.pb.h"
#include "ortools/constraint_solver/routing_index_manager.h"
#include "ortools/constraint_solver/routing_parameters.h"
#include "cnpy.h"
#include "typeDefs.hpp"
#include "instances_evaluation/BaseEnv.hpp"

using TASolution = std::vector<std::vector<Coord2D>>;

class OrtoolsEnv : public BaseEnv{
public:
    using RoutingModel = operations_research::RoutingModel;
    using RoutingIndexManager = operations_research::RoutingIndexManager;
    using RoutingSearchParameters = operations_research::RoutingSearchParameters;

    static constexpr char methodString[] = "ta_ortools";

    OrtoolsEnv(const std::filesystem::path &mapFilePath, const std::filesystem::path &taskFilePath, const std::filesystem::path &distanceMatrixPath);
    TASolution solve(int capacity, int makespan) const;
    TASolution solve(int capacity) const;

    [[nodiscard]] Coord2D get2DCoord(size_t globalIndex) const;
private:
    static constexpr char distanceDimensionString[] = "Distance";
    static constexpr char capacityDimensionString[] = "Capacity";

    const CoordToIndexMap coordToIndexMap;
    const std::vector<int64_t> demands;

    const RoutingIndexManager manager;

    static CoordToIndexMap buildCoordToIndexMap(
        const CompressedCoordVector& agents,
        const CompressedTasksVector& tasks
    );

    static std::vector<int64_t> computeDemands(size_t nAgents, size_t nTasks);

    static OrtoolsEnv::RoutingIndexManager buildRoutingIndexManager(int nAgents, int nMatrixRows);
    static int buildDistanceCallback(RoutingModel &routingModel, const RoutingIndexManager &manager,
                                     const CompressedDistanceMatrix &distanceMatrix);

    static int buildDemandCallback(RoutingModel &routingModel, const OrtoolsEnv::RoutingIndexManager &manager,
                                   const std::vector<int64_t> &demands);

    static RoutingSearchParameters addSearchParameters();

    static void addDistanceDimension(int makespan, RoutingModel &routingModel, int transitCallbackIndex);

    static void addCapacityDimension(int capacity, RoutingModel &routingModel, int demandCallbackIndex, size_t nAgents);

    TASolution getSolution(const operations_research::Assignment *pSolution, RoutingModel &routingModel) const;

    void configurePickupAndDeliveries(RoutingModel &routingModel) const;
};


#endif //PBS_CPP_ENV_HPP
