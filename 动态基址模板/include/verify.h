#ifndef SEA2_VERIFY_H
#define SEA2_VERIFY_H

#include <cstring>
#include <functional>
#include <string>
#include <unordered_map>

extern std::string verify_host;// verify host 域名

extern std::string verify_project_id;    // 程序PID
extern std::string verify_project_key;   // 程序KEY
extern std::string verify_project_base64;// 程序KEY base64

extern std::string verify_project_public_key;// rsa 公钥(私钥在服务端,请勿泄露)

extern std::string verify_notice_link;// 公告链接
extern std::string verify_notice_code;// 公告返回code

extern std::string verify_update_link;   // 更新链接
extern std::string verify_update_version;// 版本号
extern std::string verify_update_code;   // 更新返回code

extern std::string verify_bind_link;// 绑定链接
extern std::string verify_bind_code;// 绑定返回code

extern std::string verify_unbind_link;// 解绑/换绑链接

extern std::string verify_heart_link;// 心跳验证 返回数据无加密 无需设置code
extern std::string verify_heart_code;// 心跳返回code

extern std::string verify_variable_link;// 变量链接
extern std::string verify_variable_code;// 变量返回code

extern std::string verify_web_link;// 网页登录
extern std::string verify_web_code;// 网页登录返回code

extern std::string user_login_link;// 用户登录
extern std::string user_login_code;// 用户登录code

extern std::string user_heart_link;// 用户登录
extern std::string user_heart_code;// 用户登录code

extern std::string file_link;// 文件下载

extern int verify_encryption;       // 加密方式 1 base64自定义编码集 2 rsa非对称加密(推荐)
extern std::string main_method_name;// 主方法

extern std::unordered_map<std::string, std::function<void()>> method_map;
namespace sverify {

    struct verify_json {
        bool success;// 是否成功
        bool expire; // 是否过期

        bool have_update;// 是否有更新
        long status_code;// 本次网络请求的状态码 常见200成功 401认证失败 302重定向
        long available;  // 剩余可用时间秒

        int card_id;          //卡密ID
        int unbind_number;    //解绑次数
        int bind_number;      //绑定次数
        std::string end_time; // 到期时间
        std::string card_type;// 卡类型
        std::string notice;   // 获取的公告
        std::string variables;// 获取的变量
        std::string core;     //核心数据
        std::string token;    // 心跳token

        std::string username;// 获取的账号
        std::string password;// 获取的密码

        std::string update_url;    // 更新链接
        std::string update_message;// 更新信息
        bool update_must;          // 是否强制更新
        time_t timestamp;          // 服务器时间戳
        std::string error_message; // 失败的错误信息
        FILE *file; // 文件句柄
        size_t file_size; // 文件大小
    };


    bool get_notice(verify_json &json, bool log = false);                                                                                       // 获取程序公告
    bool get_update(verify_json &json, bool log = false);                                                                                       // 获取程序更新信息
    bool get_variables(verify_json &json, bool log = false);                                                                                    // 获取程序变量
    bool bind_card(const std::string &kami_, const std::string &imei_, sverify::verify_json &json_, bool log = false);                          // 单码卡密绑定
    bool unbind_card(const std::string &kami_, const std::string &imei_, sverify::verify_json &json_, bool log = false);                        // 单码卡密换绑
    bool heart_beat(const std::string &cid, const std::string &imei_, const std::string &token_, sverify::verify_json &json_, bool log = false);// 单码卡密换绑
    bool web_bind(const std::string &code_, const std::string &imei_, sverify::verify_json &json_, bool log = false);                           // 网页登录
    bool user_login(const std::string &username_, const std::string &password_, sverify::verify_json &json_, bool log = false);                 // 用户登录
    bool user_heart_beat(const std::string &cid, const std::string &token_, sverify::verify_json &json_, bool log = false);                     // 用户心跳
    bool file_download(const std::string &fid, const std::string &md5,const std::string &imei, sverify::verify_json &json_, bool log = false);                     // 用户心跳
    void init_method();                                                                                                                         // 初始化逻辑方法

}


#endif//SEA2_VERIFY_H
