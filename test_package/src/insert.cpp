
#include <catch2/catch.hpp>
#include <lime/execution.h>
#include <algorithm>

struct Position : Cell {
    float x, y, z, w;
};

struct Offset : Cell {
    float x, y, z, w;
};

struct Velocity : Cell {
    float x, y, z, w;
};

struct Tag : Cell {};

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

TEST_CASE("Database supports multiple value types") {
    ExecutionContext ec;
    for (auto i = 0; i < 100'000; ++i)
        ec.db.Insert(Position{ .x = -1 });
    for (auto i = 0; i < 200'000; ++i)
        ec.db.Insert(Offset{ .x = 10 });

    std::size_t positions = 0;
    ec.Append(
        All<Position>(),
        Once([&positions](RowCount rowCount) {
            positions = rowCount.value;
        })
    );

    std::size_t offsets = 0;
    ec.Append(
        All<Offset>(),
        Once([&offsets](RowCount rowCount) {
            offsets = rowCount.value;
        })
    );

    REQUIRE(positions == 100'000);
    REQUIRE(offsets == 200'000);
}

TEST_CASE("Database supports upsert") {
    ExecutionContext ec;
    const auto recordId = ec.db.Insert(Position{ });
    ec.db.Upsert(recordId, Velocity{ });

    std::size_t count = 0;
    ec.Append(
        All<Position, Velocity>(),
        Once([&count](RowCount rowCount) {
            count = rowCount.value;
        })
    );
    REQUIRE(count == 1);
}