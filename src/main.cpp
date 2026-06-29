#include "../include/SkillLoader.h"
#include <iostream>

int main() {
  // Đường dẫn tương đối tới thư mục skills/
  SkillLoader loader("skills");

  // Load tất cả file .md trong thư mục skills/
  loader.loadAll();

  // In danh sách skill đã load
  std::cout << "\n=== Loaded Skills ===\n";
  for (const auto &name : loader.getLoadedSkills()) {
    std::cout << "  - " << name << "\n";
  }

  // In system prompt (nội dung tất cả skill ghép lại)
  std::cout << "\n=== System Prompt ===\n";
  std::cout << loader.getSystemPrompt() << "\n";

  return 0;
}
