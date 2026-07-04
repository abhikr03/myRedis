#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <cassert>
#include <unistd.h>
#include <vector>
#include <chrono>

void die(const char* msg);

int recv_exactly(int fd, char* buf, size_t n_bytes);

int write_exactly(int fd, const char* buf, size_t n_bytes);

std::string to_lower(const std::string& str);

class IncorrectProtocol : public std::runtime_error {
public:
    explicit IncorrectProtocol(const std::string& message) : std::runtime_error(message) {}
};

class SysCallFailure : public std::runtime_error {
public:
    explicit SysCallFailure(const std::string& message) : std::runtime_error(message) {}
};

class RedisServerError : public std::runtime_error {
public:
    explicit RedisServerError(const std::string& message) : std::runtime_error(message) {}
};