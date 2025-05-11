// RankUpdater.cpp
#include "RankUpdater.hpp"
#include <iostream>
#include <initializer_list>

RankUpdater::RankUpdater(std::shared_ptr<sw::redis::Redis> redis, const std::string& key, size_t k)
    : redis_(std::move(redis)), key_(key), k_(k) {
    lua_script_ = R"(
        redis.call("ZADD", KEYS[1], ARGV[2], ARGV[1])
        local count = redis.call("ZCARD", KEYS[1])
        if count > tonumber(ARGV[3]) then
            redis.call("ZREMRANGEBYRANK", KEYS[1], 0, count - tonumber(ARGV[3]) - 1)
        end
        return 1
    )";
}

std::mutex& RankUpdater::get_sharded_mutex(const std::string& user) {
    size_t hash = std::hash<std::string>{}(user);
    return lock_shards_[hash % SHARD_COUNT];
}

void RankUpdater::submit(const std::string& user, double score) {
    std::mutex& user_mutex = get_sharded_mutex(user);
    std::lock_guard<std::mutex> lock(user_mutex);

    std::cout << "[Submit] " << user << " -> " << score << std::endl;
    redis_->eval<std::string>(lua_script_,     
                                { key_ }, // 这是 initializer_list<StringView>
                                { user, std::to_string(score), std::to_string(k_) }
                            );
}

void RankUpdater::submit_batch(const std::vector<std::pair<std::string, double>>& entries) {
    auto pipe = redis_->pipeline();
    std::vector<std::mutex*> locked;

    for (const auto& [user, score] : entries) {
        std::mutex& user_mutex = get_sharded_mutex(user);
        user_mutex.lock();
        locked.push_back(&user_mutex);

        std::cout << "[Batch Submit] " << user << " -> " << score << std::endl;
        pipe.eval(lua_script_, {key_}, {user, std::to_string(score), std::to_string(k_)});
    }

    pipe.exec();

    for (auto* mtx : locked) {
        mtx->unlock();
    }
}

std::vector<std::pair<std::string, double>> RankUpdater::get_top_k() {
    std::vector<std::pair<std::string, double>> results;
    
#if defined(SW_REDIS_VERSION_AT_LEAST_1_3_5)
    // 使用新版本接口（>= 1.3.5）
    redis_->zrevrange_with_scores(key_, 0, static_cast<long long>(k_ - 1), std::back_inserter(results));

#else
    // 老版本 fallback：zrevrange + zscore
    std::vector<std::string> members;
    redis_->zrevrange(key_, 0, static_cast<long long>(k_ - 1), std::back_inserter(members));

    for (const auto& member : members) {
        auto score = redis_->zscore(key_, member);
        results.emplace_back(member, score.value_or(0.0));
    }
#endif
    return results;
}

// template<typename Result>
// Result RankUpdater::eval_script(const sw::redis::StringView& script,
//                                        const std::vector<std::string>& keys,
//                                        const std::vector<std::string>& args) {
//     std::vector<sw::redis::StringView> keys_view, args_view;
//     for (const auto& k : keys) keys_view.emplace_back(k);
//     for (const auto& a : args) args_view.emplace_back(a);
//     return redis_->eval<Result>(script, keys_view, args_view);
// }