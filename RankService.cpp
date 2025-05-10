#include "RankService.hpp"
#include "Log.hpp"
#include <fstream>
#include <iostream>

RankService::RankService(const std::string& redisUri) {
    redis_ = std::make_shared<sw::redis::Redis>(redisUri);
}

void RankService::SubmitScore(const std::string& userId, double score) {
    redis_->zadd(key_, userId, score);
}

double RankService::GetScore(const std::string& userId) {
    auto score = redis_->zscore(key_, userId);
    return score.value_or(0.0);
}

std::vector<std::pair<std::string, double>> RankService::GetRankPage(int page, int pageSize) {
    std::vector<std::pair<std::string, double>> results;

    int start = (page - 1) * pageSize;
    int end = start + pageSize - 1;

#if defined(SW_REDIS_VERSION_AT_LEAST_1_3_5)
    // 使用新版本接口（>= 1.3.5）
    redis_->zrevrange_with_scores(key_, start, end, std::back_inserter(results));

#else
    // 老版本 fallback：zrevrange + zscore
    std::vector<std::string> members;
    redis_->zrevrange(key_, start, end, std::back_inserter(members));

    for (const auto& member : members) {
        auto score = redis_->zscore(key_, member);
        results.emplace_back(member, score.value_or(0.0));
    }
#endif

    return results;
}

void RankService::BackupToDisk() {
    std::vector<std::pair<std::string, double>> data;
#if defined(SW_REDIS_VERSION_AT_LEAST_1_3_5)
    // 使用新版本接口（>= 1.3.5）
    redis_->zrevrange_with_scores(key_, 0, -1, std::back_inserter(data));

#else
    // 老版本 fallback：zrevrange + zscore
    std::vector<std::string> members;
    redis_->zrevrange(key_, 0, -1, std::back_inserter(members));

    for (const auto& member : members) {
        auto score = redis_->zscore(key_, member);
        data.emplace_back(member, score.value_or(0.0));
    }
#endif

    std::ofstream file("rank_backup.csv");
    file << "user,score\n";
    for (const auto& [user, score] : data) {
        file << user << "," << score << "\n";
    }
    file.close();
}
