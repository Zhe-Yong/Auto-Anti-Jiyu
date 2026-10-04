#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <algorithm>
#include <cctype>
using namespace std;

string usr = "";
string eula ="FALSE";

const string hash_sha256_JiyuTrainer = "03231F35406136A3AA0C41B2E89375451177FE8CB025ED1341443B4BD4C21C7F"; // SHA256 hash of JiYuTrainer.exe
const string hash_sha256_ini = "7F5E811926BF651DAE5DFAD637AE3F820E0CE117112A0404AACBA976E0F7C3B3"; // SHA256 hash of JiYuTrainerHooks.dll

string get_windows_username();
int delete_file(const std::string& filepath);
int GetConfigItem(const std::string& path,
                    const std::string& key,
                    std::string* out);

enum Error {
    OK              = 0,   // 成功
    ERROR_NULL_PTR          = 1,   // 传入指针为 nullptr
    ERROR_EMPTY_KEY         = 2,   // 配置项名为空
    ERROR_FILE_OPEN         = 3,   // 配置文件打开失败
    ERROR_KEY_NOT_FOUND     = 4,   // 配置项未找到

    USR_ERR_UNABLE_TO_GET_USERNAME = 5, // 无法获取用户名

    FILE_ERR_UNABLE_TO_DELETE = 6, // 无法删除文件

    EULA_NOT_AGREED = 7, // EULA 未同意
};

int main() {
    cout << "Anti-JiYu Setup" << endl;
    cout << "Please wait while collecting information..." << endl;

    if (GetConfigItem("eula.config","agree",&eula)==ERROR_FILE_OPEN){
        cout << "Error while opening eula.config!\n" << "Check if the file exists and is accessible." << endl;
        system("pause");
        return ERROR_FILE_OPEN;
    }
    else{
        if (eula == "TRUE") {
            cout << "EULA has been accepted." << endl;
        } else {
            cout << "EULA has not been accepted.\nPlease read and accept the EULA in eula.config to continue." << endl;
            system("pause");
            return EULA_NOT_AGREED;
        }
    }

    usr = get_windows_username();
    if (usr == "") {
        cout << "Error while getting user name!" << endl;
        system("pause");
        return USR_ERR_UNABLE_TO_GET_USERNAME;
    }
    else {
        cout << "User name: " << usr << endl;
    }

    cout << "Cleaning Trash" << endl;
    if(delete_file("C:\\Users\\Windows\\AppData\\Local\\Temp\\JiYuTrainer")) {
        cout << "Failed to delete temp file!" << endl;
        system("pause");
        return FILE_ERR_UNABLE_TO_DELETE;
    }
    cout << "Starting EXE" << endl;

    system("\\SRC\\JiYuTrainer.exe");

    cout << "Setup completed successfully." << endl;

    return OK;
}

string get_windows_username() {
    DWORD size = 0;
    
    // 第一次调用获取所需缓冲区大小
    GetUserNameW(nullptr, &size);
    std::vector<wchar_t> buf(size);
    
    if (GetUserNameW(buf.data(), &size)) {
        // 计算转换为 UTF-8 后所需的字节数
        int len = WideCharToMultiByte(CP_UTF8, 0, buf.data(), -1, nullptr, 0, nullptr, nullptr);
        
        std::string result(len, 0); 
        
        WideCharToMultiByte(CP_UTF8, 0, buf.data(), -1, &result[0], len, nullptr, nullptr);
        
        // 返回时去掉末尾的 \0（因为 string 会自动管理长度，包含 \0 可能导致后续比较或输出异常）
        // 不过由于分配了 len，返回的 string 也会包含 \0。通常用 c_str() 时会自动处理，但为了严谨可以截断
        if (!result.empty() && result.back() == '\0') {
            result.pop_back();
        }
        
        return result;
    }
    return "";
}

int delete_file(const std::string& filepath) {
    try {
        // remove 返回是否真正删除了文件
        // 文件不存在时返回 false（不抛异常）
        bool deleted = std::filesystem::remove(filepath);
        if (deleted) {
            cout << "Success" << endl;
        } else {
            cout << "Temp File Does Not Exist" << endl;
        }
    } catch (const std::filesystem::filesystem_error& e) {
        // 权限不足、文件被占用等情况会抛异常
        std::cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return OK;
}

// 读取配置文件 path 中名为 key 的项，值写入 *out
// 返回 ERROR_OK 表示成功，其余值表示不同错误
int GetConfigItem(const std::string& path,
                    const std::string& key,
                    std::string* out)
{
    if (out == nullptr)  return ERROR_NULL_PTR;
    if (key.empty())     return ERROR_EMPTY_KEY;

    std::ifstream file(path);
    if (!file.is_open()) return ERROR_FILE_OPEN;

    auto trim = [](const std::string& s) -> std::string {
        size_t b = s.find_first_not_of(" \t");
        size_t e = s.find_last_not_of(" \t");
        if (b == std::string::npos) return "";
        return s.substr(b, e - b + 1);
    };

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();

        size_t first = line.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        if (line[first] == '#' || line[first] == ';') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string k = trim(line.substr(0, eq));
        if (k != key) continue;

        *out = trim(line.substr(eq + 1));
        return OK;
    }

    return ERROR_KEY_NOT_FOUND;
}