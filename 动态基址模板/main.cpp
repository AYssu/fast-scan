#include "http.h"
#include "include/rv.h"
#include "smemory.h"
#include "smmap.h"
#include "sutils.h"
#include "utils.h"
#include "verify.h"
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>
#include <string>
#include <thread>
// 需要引用这个 因为配置文件只需要引用一次
#include "include/easy_verfy.h"
#include "json.hpp"

std::string generate_uuid_v4() {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 36; ++i) {
        if (i == 8 || i == 13 || i == 18 || i == 23) {
            ss << '-';
        } else if (i == 14) {
            ss << '4';// UUID version 4
        } else if (i == 19) {
            ss << "0123456789abcdef"[dis(gen) & 0x3 | 0x8];// variant 10x
        } else {
            ss << "0123456789abcdef"[dis(gen)];
        }
    }
    return ss.str();
}

int main(int argc, char *argv[]) {
    std::cout << "欢迎使用libSea3 多工具集成静态库" << std::endl;
    std::cout << "1、内存工具类" << std::endl;
    std::cout << "2、内存映射测试" << std::endl;
    std::cout << "3、网络请求" << std::endl;
    std::cout << "4、测试网络验证-公告" << std::endl;
    std::cout << "5、测试网络验证-更新" << std::endl;
    std::cout << "6、测试网络验证-变量" << std::endl;
    // 卡密绑定里面也有变量
    std::cout << "7、测试网络验证-卡密绑定" << std::endl;
    std::cout << "8、测试网络验证-卡密换绑" << std::endl;
    std::cout << "9、测试网络验证-变量进阶" << std::endl;
    std::cout << "10、测试网络验证-绑定进阶" << std::endl;
    std::cout << "11、测试网络验证-网页登录" << std::endl;
    std::cout << "12、测试网络验证-心跳验证" << std::endl;
    std::cout << "13、测试网络验证-用户登录" << std::endl;
    std::cout << "14、测试网络验证-用户心跳" << std::endl;
    std::cout << "15、测试网络验证-文件下载" << std::endl;
    std::cout << "16、工具类测试" << std::endl;


    auto test_num = custom_sutils::get_input_int("请输入调试的内容:");
    switch (test_num) {
        case 1: {
            auto read_flag = smemory::set_read<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
                return smemory_readv::readv(address, data, size);
            });// 调用自定义读写 实现低耦合 理论支持全读写 即使未来出了什么无敌读写也能快速适配

            auto write_flag = smemory::set_write<uintptr_t>([&](uintptr_t address, void *data, size_t size) -> long {
                return smemory_readv::writev(address, data, size);
            });

            // 设置完成 下次静态库调用的为编译的读写 小白啥也不用管 默认即可
            if (!read_flag || !write_flag) {
                std::cout << "设置自定义读写失败!" << std::endl;
                return 0;
            }

            auto pid = smemory::get_package_pid("gg.pointers");
            target_pid = pid;// 自己实现的读写需要自己提供PID
            std::cout << "进程ID:" << pid << std::endl;
            auto base = smemory::get_module_base_str("libgame.so", 1, "Xa");
            std::cout << "模块基址:" << std::hex << base << std::dec << std::endl;
            auto i = pointer::read_int(base);
            std::cout << "读取int:" << i << std::endl;
            auto f = pointer::read_float(base);
            std::cout << "读取float:" << f << std::endl;
            auto d = pointer::read_double(base);
            std::cout << "读取double:" << d << std::endl;
            auto w = pointer::read_word(base);
            std::cout << "读取word:" << w << std::endl;
            auto byte = pointer::read_char(base);
            std::cout << "读取char:" << byte << std::endl;
            auto q = pointer::read_qword(base);
            std::cout << "读取qword:" << q << std::endl;
            struct Vector3A {
                float x, y, z;
            };
            Vector3A v = {};
            smemory_readv::readv(base, &v, sizeof(Vector3A));
            std::cout << "读取Vector3A:" << v.x << " " << v.y << " " << v.z << std::endl;
        } break;
        case 2: {
            struct demo {
                int x, y, z;
            };

            auto demo_ptr = new demo();// 内存映射需要地址 和长度
            demo_ptr = static_cast<demo *>(smmap::build_mmap_void("mmap_struct", false, sizeof(demo)));
            // 将结构体映射到内存中 当其他进程调用smmap::build_mmap_void("mmap_struct", false, sizeof(demo)); 即可实现共享内存
            demo_ptr->x++;
            demo_ptr->y++;
            demo_ptr->z++;
            std::cout << "结构体映射:" << demo_ptr->x << " " << demo_ptr->y << " " << demo_ptr->z << std::endl;
            munmap(demo_ptr, sizeof(demo));
            // 关闭映射 能干嘛?跨进程通信啊同学,配置文件也能使用,不香嘛
        } break;
        case 3: {
            auto http = shttp::http_get("https://www.easyverify.top");
            std::cout << "请求是否成功: " << http.success << std::endl;
            std::cout << "状态码: " << http.code << std::endl;
            std::cout << "响应数据: " << http.data << std::endl;
            std::cout << "错误信息: " << http.error << std::endl;
        } break;
        case 4: {
            std::cout << "-- 测试公告" << std::endl;
            sverify::verify_json json = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::get_notice(json);
            if (json.success) {
                std::cout << "获取到的公告: " << json.notice << std::endl;
                std::cout << "服务器时间戳:" << json.timestamp << std::endl;

            } else {
                std::cout << "获取公告失败: " << json.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json.status_code << std::endl;

        } break;
        case 5: {
            sverify::verify_json json2 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::get_update(json2);
            if (json2.success) {
                if (json2.have_update) {
                    std::cout << "是否强制更新: " << (json2.update_must ? "强制更新" : "不强制") << std::endl;
                    std::cout << "更新公告: " << json2.update_message << std::endl;
                    std::cout << "更新链接: " << json2.update_url << std::endl;
                } else {
                    std::cout << "当前最新版本，公告(更新增强): " << json2.notice << std::endl;
                }
            } else {
                std::cout << "获取更新失败: " << json2.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json2.status_code << std::endl;

        } break;
        case 6: {
            std::cout << "-- 测试变量" << std::endl;
            sverify::verify_json json3 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::get_variables(json3);
            if (json3.success) {
                std::cout << "获取到的变量: " << json3.variables << std::endl;
                std::cout << "服务器时间戳:" << json3.timestamp << std::endl;

            } else {
                std::cout << "获取变量失败: " << json3.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json3.status_code << std::endl;
        } break;
        case 7: {
            std::cout << "-- 测试单码卡密绑定" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::bind_card("K706CZVJXB23NG85H4L9M1", sutils::get_imei(3), json4);
            if (json4.success) {
                std::cout << "卡密ID: " << json4.card_id << std::endl;
                std::cout << "到期时间: " << json4.end_time << std::endl;
                std::cout << "到期时间戳: " << json4.available << std::endl;
                std::cout << "卡密类型: " << json4.card_type << std::endl;
                std::cout << "绑定次数: " << json4.bind_number << std::endl;
                std::cout << "解绑次数: " << json4.unbind_number << std::endl;
                std::cout << "核心数据: " << json4.core << std::endl;
                std::cout << "程序变量: " << json4.variables << std::endl;
            } else {
                std::cout << "绑定卡密失败: " << json4.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json4.status_code << std::endl;

        } break;
        case 8: {
            std::cout << "-- 测试单码卡密换绑" << std::endl;
            sverify::verify_json json5 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::unbind_card("1ZCNK502B97GM348JLXVH6", sutils::get_imei(3), json5);
            if (json5.success) {
                std::cout << "换绑成功: 换绑成功" << std::endl;
            } else {
                std::cout << "换绑失败: " << json5.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json5.status_code << std::endl;

        } break;
        case 9: {
            std::cout << "-- 测试变量" << std::endl;

            struct variable_struct {
                uintptr_t 矩阵头;
                std::vector<uintptr_t> 对象数组;
                std::vector<int> 载具ID;
                long 子弹防抖;
                int 开镜值;
                float 某个修改值;
                bool 某个功能开关;
            };

            variable_struct struct_data = {};

            sverify::verify_json json3 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::get_variables(json3);

            if (json3.success) {
                nlohmann::json response = nlohmann::json::parse(json3.variables);
                std::cout << response.dump() << std::endl;
                auto success = !response["success"].is_null() && response["success"].get<bool>();
                if (success) {
                    auto data = response["PUBG2"];
                    sutils::string_to_uintptr(struct_data.矩阵头, data["矩阵头"].get<std::string>());
                    sutils::string_to_long(struct_data.子弹防抖, data["子弹防抖"].get<std::string>());
                    sutils::string_load_vector(struct_data.对象数组, data["对象数组"].get<std::string>());
                    sutils::string_load_vector(struct_data.载具ID, data["载具ID"].get<std::string>());
                    struct_data.开镜值 = data["开镜值"].get<int>();
                    struct_data.某个修改值 = data["某个修改值"].get<float>();
                    struct_data.某个功能开关 = data["某个功能开关"].get<bool>();

                    std::cout << "变量解析成功--打印" << std::endl;
                    std::cout << "矩阵头: 0x" << std::hex << std::uppercase << struct_data.矩阵头 << std::dec << std::endl;
                    std::cout << "子弹防抖: " << struct_data.子弹防抖 << std::endl;// 默认输出10进制
                    std::cout << "开镜值: " << struct_data.开镜值 << std::endl;
                    std::cout << "对象数组: ";
                    for (auto &i: struct_data.对象数组) {
                        std::cout << "0x" << std::hex << std::uppercase << i << " ";
                    }
                    std::cout << std::dec << std::endl;
                    std::cout << "载具ID: ";
                    for (auto &i: struct_data.载具ID) {
                        std::cout << i << " ";
                    }
                    std::cout << std::endl;
                    std::cout << "某个修改值: " << struct_data.某个修改值 << std::endl;
                    std::cout << "某个功能开关：" << (struct_data.某个功能开关 ? "开启" : "关闭") << std::endl;
                } else {
                    std::cout << "变量解析失败" << std::endl;
                }

            } else {
                std::cout << "获取变量失败: " << json3.error_message << std::endl;
            }
            std::cout << "请求状态码: " << json3.status_code << std::endl;

        } break;
        case 10: {
            auto function_core = []() -> void {
                std::cout << "调用受保护的方法" << std::endl;
            };
            method_map.insert({ "main", function_core });

            sverify::init_method();//尝试没登录调用
            std::cout << "-- 测试单码卡密绑定" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::bind_card("1ZCNK502B97GM348JLXVH6", sutils::get_imei(3), json4);
            if (json4.success) {
                std::cout << "到期时间: " << json4.end_time << std::endl;

            } else {
                std::cout << "绑定卡密失败: " << json4.error_message << std::endl;
            }
            sverify::init_method();//登录后调用
        } break;
        case 11: {
            std::cout << "-- 网络验证" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            std::string uuid = generate_uuid_v4();
            uuid = "122b598a-1f28-4086-897f-cceb50586d13";
            std::string username = "2997036064@qq.com";
            std::string password = "xxxxxxxx";
            std::string url = "https://www.easyverify.cn";
            std::string open_url = url + "/user_login/" + sutils::base64_encode(verify_project_id) + "?code=" + uuid + "&username=" + username + "&password=" + password;
            sutils::execute_shell_command("am start -a android.intent.action.VIEW -d " + open_url + " > /dev/null 2>&1");
            std::cout << "打开网页: " << open_url << std::endl;

            for (int i = 0; i < 20; i++) {
                sverify::web_bind(uuid, sutils::get_imei(3), json4, false);
                if (json4.success) {
                    if (json4.expire) {
                        std::cout << "当前用户登录成功: " << json4.username << std::endl;
                        std::cout << "到期时间: " << json4.end_time << std::endl;
                    } else {
                        std::cout << "验证失败: " << json4.error_message << std::endl;
                    }
                    break;
                }
                std::cout << "监听次数: " << i << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
            std::cout << "验证完成: " << json4.expire << std::endl;

        } break;
        case 12: {
            std::cout << "-- 测试单码卡密绑定" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            // NOLINTNEXTLINE
            sverify::bind_card("1ZCNK502B97GM348JLXVH6", sutils::get_imei(3), json4, true);
            if (json4.success) {

                auto error = 0;
                sverify::verify_json json = {};// 清空结构体数据
                while (error <= 3) {
                    json = {};
                    sverify::heart_beat(std::to_string(json4.card_id), sutils::get_imei(3), json4.token, json, true);
                    if (json.success && json.status_code == 200) {
                        std::cout << "心跳验证成功！" << std::endl;
                    } else {
                        error++;
                        std::cout << "心跳验证失败: " << error << std::endl;
                    }

                    std::this_thread::sleep_for(std::chrono::seconds(30));
                }
                std::cout << "心跳验证失败! 退出程序" << std::endl;
                exit(0);
            } else {
                std::cout << "绑定卡密失败: " << json4.error_message << std::endl;
            }
        } break;
        case 13: {
            std::cout << "-- 网络验证-用户登录" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            std::string username = "2997036064@qq.com";
            std::string password = "xxxxxxx";

            sverify::user_login(username, password, json4, true);
            if (json4.success) {
                if (json4.expire) {
                    std::cout << "当前用户登录成功: " << json4.username << std::endl;
                    std::cout << "到期时间: " << json4.end_time << std::endl;
                    std::cout << "心跳token：" << json4.token << std::endl;
                } else {
                    std::cout << "验证失败: " << json4.error_message << std::endl;
                }
            }else {
                std::cout << "验证失败: " << json4.error_message << std::endl;
            }
        } break;
        case 14: {


            std::cout << "-- 网络验证-用户登录" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            std::string username = "2997036064@qq.com";
            std::string password = "xxxxxxxx";

            sverify::user_login(username, password, json4, true);
            if (json4.success) {
                if (json4.expire) {
                    std::cout << "当前用户登录成功: " << json4.username << std::endl;
                    std::cout << "到期时间: " << json4.end_time << std::endl;
                    std::cout << "心跳token：" << json4.token << std::endl;

                    auto error = 0;
                    sverify::verify_json json = {};// 清空结构体数据
                    while (error <= 3) {
                        json = {};
                        sverify::user_heart_beat(std::to_string(json4.card_id), json4.token, json, true);
                        if (json.success && json.status_code == 200) {
                            std::cout << "心跳验证成功！" << std::endl;
                        } else {
                            error++;
                            std::cout << "心跳验证失败: " << error << std::endl;
                        }

                        std::this_thread::sleep_for(std::chrono::seconds(30));
                    }
                    std::cout << "心跳验证失败! 退出程序" << std::endl;
                    exit(0);
                } else {
                    std::cout << "验证失败: " << json4.error_message << std::endl;
                }
            }else {
                std::cout << "验证失败: " << json4.error_message << std::endl;
            }
        } break;
        case 15: {
            std::cout << "-- 网络验证-文件下载" << std::endl;
            sverify::verify_json json4 = {};// 清空结构体数据
            std::string fid = "69a71aff437f3308423a2f1ca5d91a65";
            std::string md5 = "1821258587c2467f";
            std::string imei = sutils::get_imei(3);

            sverify::file_download(fid, md5,imei, json4, true);
            if (json4.success) {
                if (json4.file == nullptr)
                {
                    // 文件是最新的
                    std::cout << "文件是最新的,反正信息是这样" << std::endl;
                    std::cout << json4.error_message << std::endl;
                }else {
                    auto file = fopen("test.pdf", "wb");
                    rewind(json4.file);

                    char buffer[1024];
                    size_t bytesRead;

                    while ((bytesRead = fread(buffer, 1, sizeof(buffer), json4.file)) > 0) {
                        fwrite(buffer, 1, bytesRead, file);
                    }
                    fclose(file);
                    fclose(json4.file);
                    std::cout << "文件下载成功！" << std::endl;
                }

            }else {
                std::cout << "验证失败: " << json4.error_message << std::endl;
            }
        } break;
        case 16: {
            std::string str = "abc阿夜6哔";
            std::cout << "原字符串: " << str << std::endl;
            auto md5 = sutils::to_md5(str);
            std::cout << "默认小写 MD5: " << md5 << std::endl;
            md5 = sutils::to_md5(str, true);
            std::cout << "大写 MD5: " << md5 << std::endl;
            std::string b64_chars = "164pqrGF3bZwMfh/Ty29j7KHVnsxJDlX8EBegWC5+OzRuiNSmaPt0QLIkUYAdcov";
            std::string encoded = sutils::base64_encode(str, b64_chars);
            std::string decoded = sutils::base64_decode(encoded, b64_chars);

            std::cout << "自定义base64加密: " << encoded << std::endl;
            std::cout << "自定义base64解密: " << decoded << std::endl;

            auto hex = sutils::to_hex(str);
            std::cout << "默认小写 HEX: " << hex << std::endl;
            hex = sutils::to_hex(str, true);
            std::cout << "大写 HEX: " << hex << std::endl;

            std::cout << "当前时间戳: " << sutils::get_timestamp() << std::endl;

            auto sha = sutils::sha256(str);
            std::cout << "默认小写 sha256: " << sha << std::endl;
            sha = sutils::sha256(str, true);
            std::cout << "大写 sha256: " << sha << std::endl;

            std::string public_key = "MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAw0lDF9vj2o0KdiGeguV9fig+DCf+4WTf6hs5KDw7tFtqDFk9C6giFCP2K5SHNjH/PFYQ/Arn+ccBZwO/5wpz/HzKF41zgoF5gyEV1T1FoqjgC40HO7iXM1WO0ORTS4/T7U1obwHf42pfm4kjBkl/yzngPSdK7aBffI50kLc4gDlRQLffEP3ilKiEJ/HSOqtunadAXWUrnecn0CRdu+gZ1FlfLO7D1qcqaOl4dPtEgn/1tzr77fUMs7rElxaQvbV51R3HTuLz9HZfSg4ZNOqMzEx424T9XTvht0WKisTyhBd/xWUWevwYOMaarHDlJoqU/Hn04ALrrWpTeWAYW0rthQIDAQAB";
            std::string private_key = "MIIEvQIBADANBgkqhkiG9w0BAQEFAASCBKcwggSjAgEAAoIBAQDDSUMX2+PajQp2IZ6C5X1+KD4MJ/7hZN/qGzkoPDu0W2oMWT0LqCIUI/YrlIc2Mf88VhD8Cuf5xwFnA7/nCnP8fMoXjXOCgXmDIRXVPUWiqOALjQc7uJczVY7Q5FNLj9PtTWhvAd/jal+biSMGSX/LOeA9J0rtoF98jnSQtziAOVFAt98Q/eKUqIQn8dI6q26dp0BdZSud5yfQJF276BnUWV8s7sPWpypo6Xh0+0SCf/W3Ovvt9QyzusSXFpC9tXnVHcdO4vP0dl9KDhk06ozMTHjbhP1dO+G3RYqKxPKEF3/FZRZ6/Bg4xpqscOUmipT8efTgAuutalN5YBhbSu2FAgMBAAECggEABItvbXOMwBjOg1pGm0A7pT8gK93rFL6+u+7Ulob0Ur8FNEgICiiTBnShys4tlAl3fvrNHA3QQTlW4y6sqB6k/V1qweqPo3Ffl7l3jcYGbqx5w6W4Dd/pW49PAAstlGbCpVVLAWOsnK5sjteTAkHyoJL2ZiXvHKGXUfQ/aotrsVwu9rf1zAL8MRz0yHGJI7HYb8J/Agl2Es74yc8t8ZZ3QjQByDWR26AGi9nNNk8jP9lbCdcK1UElQ9FOFlUBnn7Hu5RMJH1PHaIhwKHMqQPGSqL3y9U6z7d7f3eAOMohZbL4GFcV2SRilzfYjpBjYl+wLUks7lwJqRypfl2yNPd4HwKBgQD9WzgboPMxWhnvgmCO3YFXotl/nLzXA0r5MaCC01trZCgzhAWKXrLf5vs6UIetJtSCg2JgYAJcNzGRrblOw8dywh8LC0/PwlNqOB0c2ZYRIEUEBi3aHIiVMBtwMUWu1g+U/nzckbfH1odYAmhxk6e2bAggA/BdZLrO3yv8QsSYKwKBgQDFUuwi5LTbylL0iMFsHOh+f3HkzpD7EP438ZQYGdtYW8gZPm9MPDDpieatzOMBlOrGyYb5qzaqZOKfizZha4aJmf9s7zXFV426uch8GB3ogo3xorqmGefT/es9PeHt2laBdG8spAuY7w5s+kZXTY1B0v6CfsC5rRcJOj7BioWJDwKBgDxOqlxpQ3pL6ECK2FEpiNBDg2JM/OZxcjc/COYbSXeWr7sTgf4d7JY/dS28XY9p3svyWkh2khlBShtTLvkAwUkfzCPk3Wqm5xQxpzxmzsHk3IjIr8FraVeZy+44zE2BxtPAgBhA125KMo1QhWwDqbhSntyAE4cnow/7L40sl39hAoGARZ4V9dGfBZSCZAgDxfMFC+xG8KT4fbvnFTHEQCSPQMNQ+6cNeWYbE6o1msgYpyw1EOF1H40Kgl+JnSRukTxwgQe2GGblH7TeKiz10OJpWukEz2XMWGH3atHBNyAoH9TShGXh+2v9M33UJxq9yntwBPM+HChAMsIgyH6mOX530n0CgYEA014CtMmpWVZau+etxm7lKAubC414o6XwZuNxodQo88M0LJxQMCeiUPEzq03Igcaf5qAGothqjlx2+7fEDvTgLuT6FRgUcXtmu8Fs7PWKVi/0g0mct3ctPn28WrlZ1NORo5YtFeGB44/yYoje7WdNpR0zWLTblhpC8bW8i98gXcA=";

            auto pub_key = sutils::pem_format(public_key, false);
            auto pri_key = sutils::pem_format(private_key, true);

            // 公钥加密-私钥解密
            auto encrypt_text = sutils::rsa_public_encrypt(str, pub_key);
            encrypt_text = sutils::base64_encode(encrypt_text);
            std::cout << "公钥加密: " << encrypt_text << std::endl;
            auto decrypt_text = sutils::rsa_private_decrypt(sutils::base64_decode(encrypt_text), pri_key);
            std::cout << "私钥解密: " << decrypt_text << std::endl;

            // 私钥加密-公钥解密
            encrypt_text = sutils::rsa_private_encrypt(str, pri_key);
            encrypt_text = sutils::base64_encode(encrypt_text);
            std::cout << "私钥加密: " << encrypt_text << std::endl;
            decrypt_text = sutils::rsa_public_decrypt(sutils::base64_decode(encrypt_text), pub_key);
            std::cout << "公钥解密: " << decrypt_text << std::endl;

            /**
            * get_imei(int,bool) int 1-3 log
            * 在Android设备su环境和普通环境的设备码会不一样
            * 如果没有特别需求你最好使用Java获取设备的imei
            * 这个imei为可执行文件设计(伪imei)
            */
            auto imei = sutils::get_imei();
            std::cout << "IMEI(默认md5): " << imei << std::endl;
            imei = sutils::get_imei(2);
            std::cout << "IMEI(开启sha256): " << imei << std::endl;
            imei = sutils::get_imei(3);
            std::cout << "IMEI(base64): " << imei << std::endl;

        } break;
        default:
            break;
    }


    std::cout << "欢迎下次使用! GoodNight! --阿夜" << std::endl;
    return 0;
}
