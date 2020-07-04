#pragma once
#include <string>
#include <vector>

enum class role{admin, user, muted, banned};

struct message{
    std::string data;
    static const size_t header_len = sizeof(uint32_t);
    char header[header_len];
};
