#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

std::size_t count_files(const std::string &folder);

void prepare_logdir();

std::string file_contents(const std::string &filename);

// std::size_t count_lines(const std::string &filename);
std::size_t count_lines(const std::filesystem::path &filename);

void require_message_count(const std::filesystem::path &filename, const std::size_t messages);

std::size_t get_filesize(const std::string &filename);

bool ends_with(std::string const &value, std::string const &ending);