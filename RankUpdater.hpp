// RankUpdater.hpp
#pragma once

#include <string>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <utility>
#include <sw/redis++/redis++.h>

class RankUpdater {
public:
    RankUpdater(std::shared_ptr<sw::redis::Redis> redis, const std::string& key, size_t k);
    void submit(const std::string& user, double score);
    void submit_batch(const std::vector<std::pair<std::string, double>>& entries);
    std::vector<std::pair<std::string, double>> get_top_k();

private:
    std::shared_ptr<sw::redis::Redis> redis_;
    std::string key_;
    size_t k_;
    std::string lua_script_;

    static constexpr size_t SHARD_COUNT = 1024;
    std::array<std::mutex, SHARD_COUNT> lock_shards_;
    std::mutex& get_sharded_mutex(const std::string& user);

    // template<typename Result>
    // Result eval_script(const sw::redis::StringView& script,
    //                                    const std::vector<std::string>& keys,
    //                                    const std::vector<std::string>& args);
};