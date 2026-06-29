#pragma once
#include <filesystem>
#include <string>
#include <vector>


// Load tất cả skill file .md từ thư mục skills/
// Inject nội dung vào system prompt của agent
class SkillLoader {
public:
  // Constructor: truyền vào đường dẫn thư mục skills/
  explicit SkillLoader(const std::string &skills_dir);

  // Scan thư mục, load tất cả file .md
  void loadAll();

  // Load 1 file cụ thể theo tên (ví dụ: "task_planner")
  bool loadSkill(const std::string &skill_name);

  // Trả về toàn bộ nội dung các skill đã load
  // dùng để inject vào system prompt
  std::string getSystemPrompt() const;

  // Trả về danh sách tên các skill đã load
  std::vector<std::string> getLoadedSkills() const;

private:
  std::string skills_dir_;
  std::vector<std::string> loaded_skills_;  // tên file đã load
  std::vector<std::string> skill_contents_; // nội dung từng file
};