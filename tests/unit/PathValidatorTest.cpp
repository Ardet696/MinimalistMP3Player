#include <catch2/catch_all.hpp>
#include "../../src/util/PathValidator.h"

TEST_CASE("PathValidator rejects empty path", "[pathvalidator]") {
    PathValidator pv;
    CHECK_FALSE(pv.validate("").valid);
    auto r = pv.validate("");
    CHECK(r.error == PathValidator::ValidationError::EmptyPath);
}

TEST_CASE("PathValidator rejects null bytes", "[pathvalidator]") {
    PathValidator pv;
    std::string withNull = "/home/user";
    withNull += '\0';
    withNull += "/../etc/passwd";
    CHECK_FALSE(pv.validate(withNull).valid);
    auto r = pv.validate(withNull);
    CHECK(r.error == PathValidator::ValidationError::ContainsNullByte);
}

TEST_CASE("PathValidator rejects path traversal", "[pathvalidator]") {
    PathValidator pv;
    CHECK_FALSE(pv.validate("/home/../etc/passwd").valid);
    CHECK_FALSE(pv.validate("../secret").valid);
    CHECK_FALSE(pv.validate("/home/user/../../root").valid);
}

TEST_CASE("PathValidator rejects control characters", "[pathvalidator]") {
    PathValidator pv;
    std::string withCtrl = "/home/user/\x01music";
    CHECK_FALSE(pv.validate(withCtrl).valid);
}

TEST_CASE("PathValidator rejects shell metacharacters in strict mode", "[pathvalidator]") {
    PathValidator::Options opts;
    opts.strictMode = true;
    opts.requireExists = false;
    opts.requireDirectory = false;
    PathValidator pv(opts);

    CHECK_FALSE(pv.validate("/home/user/$(rm -rf /)").valid);
    CHECK_FALSE(pv.validate("/home/user/`whoami`").valid);
    CHECK_FALSE(pv.validate("/tmp/test;ls").valid);
}

TEST_CASE("PathValidator accepts valid existing directory", "[pathvalidator]") {
    PathValidator pv;
    auto r = pv.validate("/tmp");
    CHECK(r.valid);
    CHECK(r.error == PathValidator::ValidationError::None);
}

TEST_CASE("PathValidator rejects path too long", "[pathvalidator]") {
    PathValidator::Options opts;
    opts.maxPathLength = 10;
    opts.requireExists = false;
    opts.requireDirectory = false;
    PathValidator pv(opts);

    CHECK_FALSE(pv.validate("/this/is/a/very/long/path").valid);
    auto r = pv.validate("/this/is/a/very/long/path");
    CHECK(r.error == PathValidator::ValidationError::PathTooLong);
}

TEST_CASE("PathValidator rejects nonexistent path", "[pathvalidator]") {
    PathValidator pv;
    CHECK_FALSE(pv.validate("/nonexistent_dir_12345").valid);
    auto r = pv.validate("/nonexistent_dir_12345");
    CHECK(r.error == PathValidator::ValidationError::PathDoesNotExist);
}
