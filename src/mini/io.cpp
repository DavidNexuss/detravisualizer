#include <vector>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <cerrno>
#include <cstring>
#include <stdexcept>

namespace io {

size_t filesize(const std::string& path) {
  struct stat st;
  if (stat(path.c_str(), &st) < 0) {
    throw std::runtime_error("Failed to stat file: " + path + " (" + std::strerror(errno) + ")");
  }
  return static_cast<size_t>(st.st_size);
}

std::vector<char> fileread_posix(const std::string& path) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw std::runtime_error("Failed to open file: " + path + " (" + std::strerror(errno) + ")");

  size_t            size = filesize(path);
  std::vector<char> buffer(size);

  ssize_t total_read = 0;
  while (total_read < static_cast<ssize_t>(size)) {
    ssize_t r = read(fd, buffer.data() + total_read, size - total_read);
    if (r < 0) {
      close(fd);
      throw std::runtime_error("Failed to read file: " + path + " (" + std::strerror(errno) + ")");
    }
    if (r == 0) break; // EOF
    total_read += r;
  }

  close(fd);
  return buffer;
}

std::vector<char> fileread_mmap(const std::string& path) {
  int fd = open(path.c_str(), O_RDONLY);
  if (fd < 0) throw std::runtime_error("Failed to open file: " + path + " (" + std::strerror(errno) + ")");

  size_t size = filesize(path);
  if (size == 0) {
    close(fd);
    return {}; // empty file
  }

  void* mapped = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    close(fd);
    throw std::runtime_error("Failed to mmap file: " + path + " (" + std::strerror(errno) + ")");
  }

  std::vector<char> buffer(size);
  std::memcpy(buffer.data(), mapped, size);

  munmap(mapped, size);
  close(fd);
  return buffer;
}

std::vector<char> fileread(const std::string& path) {
  size_t           size           = filesize(path);
  constexpr size_t MMAP_THRESHOLD = 16 * 1024; // 16 KB threshold
  if (size >= MMAP_THRESHOLD) {
    return fileread_mmap(path);
  } else {
    return fileread_posix(path);
  }
}

} // namespace io
