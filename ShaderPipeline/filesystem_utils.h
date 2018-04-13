#pragma once
#include <vector>
#include <string>
#include "string_utils.h"

#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;

std::vector<std::string> getFilesInDirectory(const std::string &directory) {
  std::vector<std::string> names;
  for (auto & p : fs::directory_iterator(directory)) {
    names.push_back(p.path().string());
  }
  return names;
}

bool doesFileExist(const std::string &path) {
  return fs::exists(path) && fs::is_regular_file(path);
}

bool doesDirectoryExist(const std::string &path) {
  return fs::exists(path) && fs::is_directory(path);
}

bool deleteFile(const std::string &path) {
  return fs::is_regular_file(path) && fs::remove(path);
}

bool deleteDirectory(const std::string &path) {
  return fs::is_directory(path) && fs::remove(path);
}

bool makeDirectory(const std::string &path) {
  return fs::create_directory(path);
}

bool makeDirectoryRecursive(const std::string &path) {
  return fs::create_directories(path);
}

std::string makeFullPath(const std::string &path) {
  return fs::absolute(path).string();
}
