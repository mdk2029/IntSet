#include "benchmark/benchmark.h"
#include <kset/kset.h>
#include <iostream>
#include <unordered_set>
#include <random>
#include <ctime>

#include <cstdlib>
#include <map>

class SetFixture : public ::benchmark::Fixture {

public:

    SetFixture() : ::benchmark::Fixture(),
        gen_(time(nullptr)),
        dis_{0,std::numeric_limits<int64_t>::max()}
    {}

    void SetUp(::benchmark::State& st) override {
        int size = static_cast<int>(st.range(0));
        for (int i = 0; i < size; ++i) {
            data_.insert(dis_(gen_));
        }
    }

    void TearDown(::benchmark::State&) override {
        data_.clear();
    }

    std::mt19937_64 gen_;
    std::uniform_int_distribution<int64_t> dis_;
    std::set<int64_t> data_;
};

BENCHMARK_DEFINE_F(SetFixture, Lookup)(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            benchmark::DoNotOptimize(data_.find(dis_(gen_)));
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(SetFixture, Lookup)->RangeMultiplier(2)->Range(1000000, 32000000);

BENCHMARK_DEFINE_F(SetFixture, Successor)(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            int64_t randval = dis_(gen_);
            auto itr = data_.find(randval);
            if(itr != std::end(data_)) {
                ++itr;
            }
            benchmark::DoNotOptimize(itr);
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(SetFixture, Successor)->RangeMultiplier(2)->Range(1000000, 32000000);

//////////////////////////////////////////////////////////////////////////////////////////

class KSetFixture : public ::benchmark::Fixture {
public:
    KSetFixture() : ::benchmark::Fixture(),
        gen_(time(nullptr)),
        dis_{0,std::numeric_limits<int64_t>::max()}
    {}

    void SetUp(::benchmark::State& st) override {
        data_ = new Kset::Node{};
        int size = static_cast<int>(st.range(0));
        for (int i = 0; i < size; ++i) {
            Kset::insert(data_, dis_(gen_));
        }
    }

    void TearDown(::benchmark::State&) override {
        delete data_;
    }

    std::mt19937_64 gen_;
    std::uniform_int_distribution<int64_t> dis_;
    Kset::Node* data_;
};


BENCHMARK_DEFINE_F(KSetFixture, Lookup)(benchmark::State& state) {
  const int size = static_cast<int>(state.range(0));
  for (auto _ : state) {
    for (int i = 0; i < size; ++i) {
      benchmark::DoNotOptimize(find(data_, dis_(gen_)));
    }
  }
  state.SetItemsProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(KSetFixture, Lookup)->RangeMultiplier(2)->Range(1000000, 32000000);

BENCHMARK_DEFINE_F(KSetFixture, Successor)(benchmark::State& state) {
    const int size = static_cast<int>(state.range(0));
    for (auto _ : state) {
        for (int i = 0; i < size; ++i) {
            int64_t randval = dis_(gen_);
            Kset::Node *dest{nullptr};
            int loc{-1};
            bool found{false};
            std::tie(dest,loc,found) = find(data_,randval);
            if(found) {
                std::tie(dest,std::ignore,std::ignore) = successor(dest,loc);
            }
            benchmark::DoNotOptimize(dest);
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
}

BENCHMARK_REGISTER_F(KSetFixture, Successor)->RangeMultiplier(2)->Range(1000000, 32000000);


////////////////////////////////////////////////////////////////////////////////////////////

BENCHMARK_MAIN();
