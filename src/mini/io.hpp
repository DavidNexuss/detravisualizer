#include <vector>
#include <string>
namespace io {
std::vector<char> fileread(const std::string& path);
size_t            filesize(const std::string& path);
} // namespace io
