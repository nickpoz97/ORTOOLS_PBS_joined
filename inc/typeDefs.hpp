//
// Created by nicco on 17/10/2022.
//

#ifndef TA_TYPEDEFS_HPP
#define TA_TYPEDEFS_HPP

#include <vector>
#include <unordered_map>
#include <utility>
#include <string>

using Matrix = std::vector<std::vector<char>>;

using Coord2D = std::pair<int64_t, int64_t>;

using CompressedCoordVector = std::vector<int64_t>;
using CompressedTasksVector = std::vector<std::pair<int64_t, int64_t>>;
using CompressedDistanceMatrix = std::vector<std::vector<int64_t>>;
using CoordToIndexMap = std::unordered_map<int64_t, int64_t>;

using CoordVector = std::vector<std::pair<int64_t, int64_t>>;
using TaskVector = std::vector<std::pair<std::pair<int64_t, int64_t>, std::pair<int64_t, int64_t>>>;

#endif //TA_TYPEDEFS_HPP
