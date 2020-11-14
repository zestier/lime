#pragma once

#include "cell.h"
#include <limits>
#include <cstdint>
#include <stack>
#include <vector>

struct RecordId : Cell {
    std::uint32_t index;
    std::uint32_t generation;
};

/**
 * Manages creation and destruction of record ids to enforce packing
 * and uniqueness within the context of a factory.
 */
class RecordIdFactory {
    std::stack<std::uint32_t> unusedIndices;
    std::vector<std::uint32_t> generations;

public:
    RecordId Alloc() {
        RecordId ret;
        if (unusedIndices.empty()) {
            ret.index = (std::uint32_t)generations.size();
            ret.generation = std::numeric_limits<std::uint32_t>::max();
            generations.push_back(ret.generation);
        }
        else {
            ret.index = unusedIndices.top();
            unusedIndices.pop();
            ret.generation = generations[ret.index];
        }
        return ret;
    }

    void Free(RecordId recordId) {
        if (!IsActive(recordId) && --generations[recordId.index])
            unusedIndices.push(recordId.index);
    }

    bool IsActive(RecordId recordId) {
        return recordId.index < generations.size() && generations[recordId.index] == recordId.generation;
    }
};