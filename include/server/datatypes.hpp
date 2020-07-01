#pragma once
#include <string>
#include <vector>

enum class role{admin, user, muted, banned};
struct message{
    std::string user_name;
    std::string message;
    std::vector<std::string> mentions;
};
