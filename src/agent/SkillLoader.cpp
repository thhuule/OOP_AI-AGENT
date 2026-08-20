#include "SkillLoader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

namespace oop_agent {

namespace {
std::string lower(std::string text) {
  std::ranges::transform(text, text.begin(), [](unsigned char ch) {
    return static_cast<char>(std::tolower(ch));
  });
  return text;
}

std::vector<std::string> parseKeywords(const std::string &content) {
  std::istringstream lines(content);
  std::string line;
  while (std::getline(lines, line)) {
    const std::string normalized = lower(line);
    const auto marker = normalized.find("keywords:");
    if (marker == std::string::npos)
      continue;

    std::vector<std::string> keywords;
    std::istringstream values(line.substr(marker + 9));
    std::string value;
    while (std::getline(values, value, ',')) {
      value.erase(value.begin(), std::find_if(value.begin(), value.end(), [](unsigned char ch) {
        return !std::isspace(ch);
      }));
      value.erase(std::find_if(value.rbegin(), value.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
      }).base(), value.end());
      if (!value.empty())
        keywords.push_back(lower(value));
    }
    return keywords;
  }
  return {};
}
} // namespace

SkillLoader::SkillLoader(const std::string &skills_dir)
    : skills_dir_(skills_dir) {}

void SkillLoader::loadAll() {
  const std::filesystem::path skills_path(skills_dir_);
  if (!std::filesystem::exists(skills_path) || !std::filesystem::is_directory(skills_path)) {
    std::cerr << "[SkillLoader] Skills directory not found: " << skills_dir_ << "\n";
    return;
  }

  // Dùng std::filesystem để scan thư mục (C++17)
  for (const auto &entry : std::filesystem::directory_iterator(skills_path)) {
    if (entry.path().extension() == ".md") {
      loadSkill(entry.path().stem().string());
    }
  }
}

bool SkillLoader::loadSkill(const std::string &skill_name) {
  std::string filepath = skills_dir_ + "/" + skill_name + ".md";
  std::ifstream file(filepath);

  if (!file.is_open()) {
    std::cerr << "[SkillLoader] Cannot open: " << filepath << "\n";
    return false;
  }

  // Đọc toàn bộ nội dung file
  std::ostringstream content;
  content << file.rdbuf();

  loaded_skills_.push_back(skill_name);
  skill_contents_.push_back(content.str());
  skill_keywords_.push_back(parseKeywords(skill_contents_.back()));

  std::cout << "[SkillLoader] Loaded: " << skill_name << "\n";
  return true;
}

std::string SkillLoader::getSystemPrompt() const {
  // Ghép tất cả skill lại, ngăn cách bằng dấu phân cách rõ ràng
  std::string prompt;
  for (size_t i = 0; i < skill_contents_.size(); i++) {
    prompt += "=== SKILL: " + loaded_skills_[i] + " ===\n";
    prompt += skill_contents_[i] + "\n\n";
  }
  return prompt;
}

std::string SkillLoader::getSystemPromptForTask(const std::string &task) const {
  const std::string normalized_task = lower(task);
  std::vector<std::size_t> selected;
  bool has_metadata = false;

  for (std::size_t i = 0; i < skill_keywords_.size(); ++i) {
    has_metadata = has_metadata || !skill_keywords_[i].empty();
    if (std::ranges::any_of(skill_keywords_[i], [&](const std::string &keyword) {
          return normalized_task.find(keyword) != std::string::npos;
        })) {
      selected.push_back(i);
    }
  }

  if (selected.empty() && has_metadata) {
    const auto planner = std::find(loaded_skills_.begin(), loaded_skills_.end(),
                                   std::string("task_planner"));
    if (planner != loaded_skills_.end())
      selected.push_back(static_cast<std::size_t>(planner - loaded_skills_.begin()));
  }
  if (selected.empty() && !has_metadata) {
    for (std::size_t i = 0; i < skill_contents_.size(); ++i)
      selected.push_back(i); // backward compatibility for metadata-free skills
  }

  std::string prompt;
  for (const std::size_t i : selected) {
    prompt += "=== SKILL: " + loaded_skills_[i] + " ===\n";
    prompt += skill_contents_[i] + "\n\n";
  }
  return prompt;
}

std::vector<std::string> SkillLoader::getLoadedSkills() const {
  return loaded_skills_;
}

} // namespace oop_agent
