//
// Created by nicco on 02/11/2022.
//

#ifndef CMAPD_PARAMETERS_HPP_IN
#define CMAPD_PARAMETERS_HPP_IN

#include "ortools/constraint_solver/routing_enums.pb.h"

static constexpr auto firstSolutionAlg = operations_research::FirstSolutionStrategy::FSS;
static constexpr auto localSearchAlg = operations_research::LocalSearchMetaheuristic::LS;

static constexpr std::string_view cuoptHost{"localhost"};
static constexpr std::string_view cuoptPort{"5000"};

#endif //CMAPD_PARAMETERS_HPP_IN
