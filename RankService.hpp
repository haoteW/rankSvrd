#pragma once

#include <string>
#include <vector>
#include <utility>
#include <memory>
#include <sw/redis++/redis++.h>
#include "RankUpdater.hpp"

#define USE_TOP_K_COUNT 1000

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
#ifdef USE_TOP_K_COUNT
    RankUpdater* pRankUpdater_;
#endif
};
