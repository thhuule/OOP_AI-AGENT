#include "../include/SkillLoader.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

SkillLoader::SkillLoader(const std::string &skills_dir)
    : skills_dir_(skills_dir) {}

void SkillLoader::loadAll() {
  // Dùng std::filesystem để scan thư mục (C++17)
  for (const auto &entry : std::filesystem::directory_iterator(skills_dir_)) {
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

std::vector<std::string> SkillLoader::getLoadedSkills() const {
  return loaded_skills_;
}