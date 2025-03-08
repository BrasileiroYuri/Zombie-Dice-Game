#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

class IniParser {
public:
  IniParser(const std::string& t_filename = "") {
    std::ifstream file{ t_filename };
    auto idx = t_filename.rfind(".");

    if (idx == std::string::npos) {
      exit(1);
    }

    auto ext = t_filename.substr(idx);
    if (ext != ".ini") {
      std::cerr << "Extensão " << ext << " não é válida.\n";
      exit(1);
    }

    if (not file.is_open()) {
      std::cerr << "Não foi possível abrir o arquivo " << t_filename << ".\n";
      exit(1);
    }

    std::string line, key, value;

    while (getline(file, line)) {

      line = trim(line);
      if (line[0] == ';' or line[0] == '#' or line[0] == '[' or line.empty()) {
        continue;
      }

      auto idx = line.find('=');

      if (idx == std::string::npos) {
        continue;
      }

      key = trim(line.substr(0, idx));
      value = trim(line.substr(idx + 1));

      if (value[0] == '"' and value.back() == '"') {
        value = value.substr(1, value.size() - 2);
      }

      m_config[key] = value;
    }
  }

  std::string get(const std::string& t_key) const {
    auto it = m_config.find(t_key);
    return (it != m_config.end() ? it->second : "");
  }

  std::unordered_map<std::string, std::string> get_map() const { return m_config; }

private:
  std::unordered_map<std::string, std::string> m_config;

  std::string trim(const std::string& t_line) {
    auto begin = t_line.find_first_not_of(" \t\r\n");
    auto end = t_line.find_last_not_of(" \t\r\n");

    return (begin != std::string::npos ? t_line.substr(begin, end - begin + 1) : "");
  }
};
