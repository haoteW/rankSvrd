#pragma once

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <sw/redis++/redis++.h>

class RankService {
public:
    explicit RankService(const std::string& redisUri);
    void SubmitScore(const std::string& userId, double score);
    double GetScore(const std::string& userId);
    std::vector<std::pair<std::string, double>> GetRankPage(int page, int pageSize);
    void BackupToDisk();
private:
    std::shared_ptr<sw::redis::Redis> redis_;
    const std::string key_ = "game_rank";
};
