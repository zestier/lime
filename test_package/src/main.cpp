#include <iostream>
#include <algorithm>
#include "lime/execution.h"

struct Position : Cell {
    float x, y, z, w;
    //float4 v;
};

struct Offset : Cell {
    float x, y, z, w;
    //float4 v;
};

struct Velocity : Cell {
    float x, y, z, w;
    //float4 v;
};

struct ApplyGravity : Cell {
};

#define CATCH_CONFIG_MAIN
#include "catch2/catch.hpp"

TEST_CASE("Database can insert multiple values") {
    const auto insertionCount = GENERATE(0, 1, 2, 5, 10, 26, 100, 1'000, 1'000'000);

    ExecutionContext ec;
    for (auto i = 0; i < insertionCount; ++i)
        ec.db.Insert(Position{ .x = 10 });

    std::size_t timesCalled = 0;
    ec.Append(EachRow([&timesCalled](const Position& p, IterationIndex index) {
        ++timesCalled;
        REQUIRE(p.x == 10);
        REQUIRE(p.y == 0);
        REQUIRE(p.z == 0);
        REQUIRE(p.z == 0);
    }));
    REQUIRE(timesCalled == insertionCount);
}

TEST_CASE("Database can be updated") {
    ExecutionContext ec;
    for (auto i = 0; i < 10; ++i)
        ec.db.Insert(Position{ .x = -1 });

    ec.Append(EachRow([](Position& p, IterationIndex index) {
        p.x = index.value * 1.0f;
    }));

    float total = 0.0f;
    ec.Append(EachRow([&total](const Position& p) {
        total += p.x;
    }));

    REQUIRE(total == Approx(45.0f));
}