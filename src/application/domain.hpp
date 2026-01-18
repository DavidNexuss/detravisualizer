#pragma once
#include <string>
#include <memory>
#include "util.hpp"

namespace domain {
std::vector<std::string> graphlist();
std::shared_ptr<Graph>   graphload(const std::string& filepath);
bool                     graphsave(std::shared_ptr<Graph> graph, const std::string& filepath, bool overwrite = true);
} // namespace domain
