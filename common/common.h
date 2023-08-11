#ifndef COMMON_HEADER
#define COMMON_HEADER

#include <random>
#include <vector>
#include "absl/types/span.h"

using Rng = std::mt19937_64;
using RngDistrib = std::uniform_int_distribution<>;
using ByteArray = std::vector<uint8_t>;
using ByteArrayList = std::vector<ByteArray>;
using ByteSpan = absl::Span<const uint8_t>;


#endif // COMMON_HEADER