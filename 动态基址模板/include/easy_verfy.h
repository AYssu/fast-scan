 //
 // Created by root on 7/19/25.
 //

 #ifndef SEAV3_EASY_VERFY_H
 #define SEAV3_EASY_VERFY_H
 #include <functional>
 #include <iostream>
 #include <obfuscate.h>
 #include <unordered_map>

 std::string verify_host = OBFUSCATE("https://www.easyverify.cn/api");// 域名
 
 std::string verify_project_id = OBFUSCATE("10018");                                                               // 程序PID
 std::string verify_project_key = OBFUSCATE("KWN1KBXDE9FA");                                                       //程序密钥
 std::string verify_project_base64 = OBFUSCATE("UCPm8YnZsldF1pwVyhoKu3MkAJW2cDREfLSgxqT/iHabN0Qt5z6Ivj4B+OeGX7r9");// base64自定义编码集

 std::string verify_project_public_key = OBFUSCATE("");

 std::string verify_notice_link = OBFUSCATE("MISZX3JLIWZ1");// 获取公告的链接
 std::string verify_notice_code = OBFUSCATE("200");         // 获取公告的code

 std::string verify_update_link = OBFUSCATE("EH7K9I01FTMJ");// 更新链接
 std::string verify_update_version = OBFUSCATE("100");      // 版本号
 std::string verify_update_code = OBFUSCATE("200");         // 更新返回code

 std::string verify_bind_link = OBFUSCATE("XK28IPSO66A8");// 绑定链接
 std::string verify_bind_code = OBFUSCATE("200");         // 绑定返回code

 std::string verify_unbind_link = OBFUSCATE("SL19ZU9K9IKE");// 解绑/换绑链接 返回数据无加密 无需设置code

 std::string verify_variable_link = OBFUSCATE("9OBL5CHT35OP");// 变量链接
 std::string verify_variable_code = OBFUSCATE("200");         // 变量返回code

 std::string verify_heart_link = OBFUSCATE("QRL5NK61OO50") ;// 心跳验证 返回数据无加密 无需设置code
 std::string verify_heart_code = OBFUSCATE("200");// 心跳返回code

 std::string verify_web_link = OBFUSCATE("2IFWKTCDCCJP");// 网页登录 返回数据无加密 无需设置code
 std::string verify_web_code = OBFUSCATE("200");// 网页登录返回code

 std::string user_login_link = OBFUSCATE("EKPWLMHVPW50");// 用户登录
 std::string user_login_code = OBFUSCATE("200");// 用户登录code

 std::string user_heart_link = OBFUSCATE("EKPWLMHVPW50");// 用户心跳
 std::string user_heart_code = OBFUSCATE("200");// 用户心跳code

 std::string file_link = OBFUSCATE("9FHTGWCW40A0");// 文件下载

 std::unordered_map<std::string, std::function<std::any(std::vector<std::any>)>> method_map;

 bool time_enable = true;       // 函数碰撞 会保护变量 速度会慢一点
 int verify_encryption = 1;// 默认1 base64 自定义编码集 2 rsa非对称加密 3 国密(未支持) 4AES (未支持)
 #endif //SEAV3_EASY_VERFY_H


