#include <filesystem>
#include <cassert>
#include <random>
#include "instances_evaluation/OrtoolsEnv.hpp"
#include "instances_evaluation/BaseEnv.hpp"
#include "parameters.hpp"
#include <sstream>

OrtoolsEnv::OrtoolsEnv(const std::filesystem::path &mapFilePath, const std::filesystem::path &taskFilePath, const std::filesystem::path &distanceMatrixPath) :
        BaseEnv(mapFilePath, taskFilePath, distanceMatrixPath),
    coordToIndexMap{buildCoordToIndexMap(agents, tasks)},
    demands{computeDemands(agents.size(), tasks.size())},
    manager{
        buildRoutingIndexManager(static_cast<int>(agents.size()), static_cast<int>(distanceMatrix.size()))
    }
    {}

OrtoolsEnv::RoutingIndexManager OrtoolsEnv::buildRoutingIndexManager(int nAgents, int nMatrixRows) {
    using NodeIndex = operations_research::RoutingIndexManager::NodeIndex;

    auto generateNodeSequence = [](int64_t begin, int64_t size){
        std::vector<NodeIndex> sequence;
        sequence.reserve(size);

        for (auto i = begin ; i < size ; ++i){
            sequence.emplace_back(i);
        }

        return sequence;
    };

    using operations_research::RoutingIndexManager;
    using operations_research::RoutingModel;

    return {
        nMatrixRows,
        nAgents,
        generateNodeSequence(0, nAgents),
        std::vector(nAgents, NodeIndex{nMatrixRows-1})
    };
}

int OrtoolsEnv::buildDistanceCallback(RoutingModel &routingModel, const RoutingIndexManager &manager,
                                      const CompressedDistanceMatrix &distanceMatrix) {
    int callbackIndex = routingModel.RegisterTransitCallback(
        [&manager, &distanceMatrix](int64_t fromIndex, int64_t toIndex) -> int64_t {
            auto fromNode = manager.IndexToNode(fromIndex).value();
            auto toNode = manager.IndexToNode(toIndex).value();
            return distanceMatrix[fromNode][toNode];
        }
    );

    routingModel.SetArcCostEvaluatorOfAllVehicles(callbackIndex);
    return callbackIndex;
}

std::vector<int64_t> OrtoolsEnv::computeDemands(size_t nAgents, size_t nTasks) {
    std::vector<int64_t> demands(nAgents, 0);
    demands.reserve(nAgents + nTasks * 2);

    for (size_t i = 0 ; i < nTasks ; ++i){
        demands.push_back(1);
        demands.push_back(-1);
    }

    return demands;
}

TASolution OrtoolsEnv::solve(int capacity, int makespan) const {
    RoutingModel routingModel{manager};
    auto transitCallbackIndex{buildDistanceCallback(routingModel, manager, distanceMatrix)};
    auto demandCallbackIndex{buildDemandCallback(routingModel, manager, demands)};

    addDistanceDimension(makespan, routingModel, transitCallbackIndex);
    addCapacityDimension(capacity, routingModel, demandCallbackIndex, agents.size());
    configurePickupAndDeliveries(routingModel);

    auto searchParameters{addSearchParameters()};

    const auto* solutionPtr = routingModel.SolveWithParameters(searchParameters);
    return getSolution(solutionPtr, routingModel);
}

void OrtoolsEnv::addCapacityDimension(int capacity, RoutingModel &routingModel, int demandCallbackIndex, size_t nAgents) {
    std::vector<int64_t> capacities(nAgents, capacity);

    routingModel.AddDimensionWithVehicleCapacity(
        demandCallbackIndex,
        0,
        capacities,
        true,
        capacityDimensionString
    );
}

void OrtoolsEnv::addDistanceDimension(int makespan, RoutingModel &routingModel, int transitCallbackIndex) {
    routingModel.AddDimension(
        transitCallbackIndex,
        0,
        makespan,
        true,
        distanceDimensionString
    );

    auto* distanceDimensionPtr = routingModel.GetMutableDimension(distanceDimensionString);
    distanceDimensionPtr->SetGlobalSpanCostCoefficient(100);
    assert(distanceDimensionPtr->global_span_cost_coefficient() == 100);
}

int OrtoolsEnv::buildDemandCallback(RoutingModel &routingModel, const OrtoolsEnv::RoutingIndexManager &manager,
                                    const std::vector<int64_t> &demands) {

    return routingModel.RegisterUnaryTransitCallback(
        [&manager, &demands](int64_t fromIndex) -> int64_t {
            int fromNode = manager.IndexToNode(fromIndex).value();
            return demands[fromNode];
        }
    );
}

OrtoolsEnv::RoutingSearchParameters OrtoolsEnv::addSearchParameters() {
    auto routingSearchParams = operations_research::DefaultRoutingSearchParameters();

    routingSearchParams.set_first_solution_strategy(
        firstSolutionAlg
    );
    routingSearchParams.set_local_search_metaheuristic(
        localSearchAlg
    );
    routingSearchParams.mutable_time_limit()->set_seconds(30);
    return routingSearchParams;
}

Coord2D OrtoolsEnv::get2DCoord(size_t globalIndex) const{
    auto taskCompressedCoord = [this](size_t index){
        // to refer to tasks vector
        index -= agents.size();

        // to check if the index refers to start or end of task
        bool isStart = (index % 2 == 0);

        // normalize index
        index /= 2;

        return isStart ? tasks[index].first : tasks[index].second;
    };

    auto compressedCoord = (globalIndex < agents.size()) ? agents[globalIndex] :  taskCompressedCoord(globalIndex);
    return {compressedCoord / nCols, compressedCoord % nCols};
}

TASolution OrtoolsEnv::getSolution(const operations_research::Assignment *pSolution, RoutingModel &routingModel) const{
    if(pSolution == nullptr){
        return {};
    }

    TASolution solution{};
    solution.reserve(agents.size());

    for(int vehicleId = 0 ; vehicleId < agents.size() ; ++vehicleId ){
        std::list<Coord2D> values{};

        auto index = routingModel.Start(vehicleId);
        while(!routingModel.IsEnd(index)){
            auto nodeIndex = manager.IndexToNode(index).value();
            values.push_back(get2DCoord(nodeIndex));
            index = pSolution->Value(routingModel.NextVar(index));
        }
        solution.push_back(values);
    }

    return solution;
}

void OrtoolsEnv::configurePickupAndDeliveries(RoutingModel &routingModel) const{
    using NodeIndex = RoutingIndexManager::NodeIndex;
    auto* const solver = routingModel.solver();
    const auto& distanceDimension = routingModel.GetDimensionOrDie(distanceDimensionString);

    for (const auto& taskCoords : tasks){
        auto pickupIndex = manager.NodeToIndex(NodeIndex {static_cast<int>(coordToIndexMap.at(taskCoords.first))});
        auto deliveryIndex = manager.NodeToIndex(NodeIndex {static_cast<int>(coordToIndexMap.at(taskCoords.second))});

        routingModel.AddPickupAndDelivery(pickupIndex, deliveryIndex);

        //same robot
        solver->AddConstraint(
            solver->MakeEquality(routingModel.VehicleVar(pickupIndex), routingModel.VehicleVar(deliveryIndex))
        );

        // each item must be picked up before it is delivered
        solver->AddConstraint(
            solver->MakeLessOrEqual(
                distanceDimension.CumulVar(pickupIndex),
                distanceDimension.CumulVar(deliveryIndex)
            )
        );
    }
}

CoordToIndexMap OrtoolsEnv::buildCoordToIndexMap(const CompressedCoordVector& agents, const CompressedTasksVector& tasks) {
    CoordToIndexMap coordToIndexMap;
    int i = 0;

    for(auto a : agents){
        coordToIndexMap.insert({a, i++});
    }

    for(const auto& t : tasks){
        coordToIndexMap.insert({t.first, i++});
        coordToIndexMap.insert({t.second, i++});
    }

    return coordToIndexMap;
}

TASolution OrtoolsEnv::solve(int capacity) const {
    auto maxMakespan = static_cast<int>(tasks.size() * 2 * getMaxDistance());
    TASolution solution{};

    for (int makespan = 20; makespan <= maxMakespan && solution.empty(); ++makespan) {
        solution = solve(capacity, makespan);
    }

    return solution;
}
