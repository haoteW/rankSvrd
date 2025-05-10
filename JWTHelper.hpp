#pragma once
#include <string>
#include <cstdlib>   // for getenv
#include <jwt-cpp/jwt.h>

#define USE_JWT_ENV 1

class JWTHelper {
public:
    static std::string Sign(const std::string& user) {
        auto secret = getSecret();

        auto token = jwt::create()
            .set_issuer("rank-server")
            .set_subject(user)
            .set_expires_at(std::chrono::system_clock::now() + std::chrono::minutes{60})
            .sign(jwt::algorithm::hs256{secret});

        return token;
    }

    static std::string Verify(const std::string& token) {
        auto secret = getSecret();

        auto decoded = jwt::decode(token);
        auto verifier = jwt::verify()
            .allow_algorithm(jwt::algorithm::hs256{secret})
            .with_issuer("rank-server");

        verifier.verify(decoded);
        return decoded.get_subject();
    }

private:
    static std::string getSecret() {
#ifdef USE_JWT_ENV
        const char* env = std::getenv("JWT_SECRET");
        if (!env) {
            throw std::runtime_error("JWT_SECRET 环境变量未设置");
        }
        return std::string(env);
#else
        return std::string("my_secret");
#endif
    }
};
