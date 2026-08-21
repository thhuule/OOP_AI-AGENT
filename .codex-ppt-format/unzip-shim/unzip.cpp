#include <cstdlib>
#include <string>

static std::string quote(const char* value) {
  std::string out = "\"";
  for (const char* p = value; *p; ++p) out += (*p == '"' ? '\'' : *p);
  return out + "\"";
}

int main(int argc, char** argv) {
  if (argc < 3) return 2;
  std::string cmd;
  if (std::string(argv[1]) == "-Z1") cmd = "tar.exe -tf " + quote(argv[2]);
  else if (std::string(argv[1]) == "-p" && argc >= 4)
    cmd = "tar.exe -xOf " + quote(argv[2]) + " " + quote(argv[3]);
  else return 2;
  return std::system(cmd.c_str());
}
