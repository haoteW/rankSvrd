#include "RankService.hpp"
#include "JWTHelper.hpp"
#include "Log.hpp"
#include <crow.h>
#include <dotenv.h>
#include <optional>

// #define USE_JWT 1

RankService service("tcp://127.0.0.1:6379");

std::optional<std::string> extractUserFromJWT(const crow::request& req);

int main() {
    dotenv::init(".rank_server.env");

    crow::SimpleApp app;

#ifdef USE_JWT
    CROW_ROUTE(app, "/login")
    ([](const crow::request& req){
        auto user = req.url_params.get("user");
        auto secret = req.url_params.get("secret");

        if (!user || !secret || std::string(secret) != "123456") {
            return crow::response(401, "unauthorized");
        }

        std::string token = JWTHelper::Sign(user);
        crow::json::wvalue res;
        res["token"] = token;
        return crow::response(res);
    });
#endif


    CROW_ROUTE(app, "/submit")
    ([](const crow::request& req){
#ifdef USE_JWT
        auto userOpt = extractUserFromJWT(req);
        if (!userOpt) return crow::response(401, "Invalid token");
#endif

        auto user = req.url_params.get("user");
        auto score = req.url_params.get("score");

        if (!user || !score) {
            return crow::response(400, "Missing user or score");
        }

        double s = std::stod(score);
        service.SubmitScore(user, s);
        return crow::response("Score submitted.");
    });

    CROW_ROUTE(app, "/get_score")
    ([](const crow::request& req){
#ifdef USE_JWT
        auto userOpt = extractUserFromJWT(req);
        if (!userOpt) return crow::response(401, "Invalid token");
#endif

        auto user = req.url_params.get("user");
        if (!user) return crow::response(400, "Missing user");

        double score = service.GetScore(user);
        return crow::response(std::to_string(score));
    });

    CROW_ROUTE(app, "/top")
    ([](const crow::request& req){
#ifdef USE_JWT
        auto userOpt = extractUserFromJWT(req);
        if (!userOpt) return crow::response(401, "Invalid token");
#endif

        int page = std::stoi(req.url_params.get("page") ?: "1");
        int size = std::stoi(req.url_params.get("size") ?: "10");

        auto data = service.GetRankPage(page, size);
        crow::json::wvalue result;
        int rank = (page - 1) * size + 1;

        crow::json::wvalue::list player_list;

        for (const auto& [user, score] : data) {
            crow::json::wvalue item;
            item["rank"] = rank++;
            item["user"] = user;
            item["score"] = score;
            player_list.push_back(item);
        }

        result["players"] = std::move(player_list);
        return crow::response(result);
    });

    CROW_ROUTE(app, "/backup")
    ([]{
#ifdef USE_JWT
        auto userOpt = extractUserFromJWT(req);
        if (!userOpt) return crow::response(401, "Invalid token");
#endif

        service.BackupToDisk();
        return crow::response("Backup completed.");
    });

    app.port(18080).multithreaded().run();
}


std::optional<std::string> extractUserFromJWT(const crow::request& req) {
    auto auth = req.get_header_value("Authorization");
    if (auth.substr(0, 7) == "Bearer ") {
        try {
            return JWTHelper::Verify(auth.substr(7));
        } catch (...) {
            return std::nullopt;
        }
    }
    return std::nullopt;
}