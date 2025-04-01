#ifndef FOL_DATA_ACCESS_LAYER_H
#define FOL_DATA_ACCESS_LAYER_H

#include <vector>
#include <string>
#include <json.h>

using namespace std;

namespace DAL {
    // Database
    std::vector<std::string> get_tables();
    void get_class_ids(const unsigned int user_id);
    void get_note_ids(const unsigned int user_id);
    std::string get_note_file_path(const unsigned int note_id);

    // Files
    std::string read_file(const std::string& file_path);
    bool write_file(const std::string& file_path, const std::string& data);

    // Txt files
    std::string read_txt_file(const std::string& file_path);
    
    // JSON files
    nlohmann::json read_json_file(const std::string& file_path);
    void write_json_file(const nlohmann::json& data);
}

#endif // FOL_DATA_ACCESS_LAYER_H