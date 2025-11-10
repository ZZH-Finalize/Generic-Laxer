#include <iostream>
#include <format>
#include <string_view>
#include "regex.hpp"

void print_match(const regex::dfa& dfa, const std::string_view& str,
                 const std::string& test_name, bool expected)
{
    auto result        = dfa.match(str);
    bool has_match = result.has_value();
    std::string status = (has_match == expected) ? "PASS" : "FAIL";
    std::cout << std::format("[{}] {}: {} ({})\n", status, test_name, has_match, str);

    if (has_match != expected) {
        std::exit(1); // 测试失败，退出程序
    }
}

int main(const int argc, const char** argv)
{
    int total_tests  = 0;
    int passed_tests = 0;

    std::cout << "=== 复杂正则表达式功能测试 ===" << std::endl;

    try {
        // 测试1: 简单连接
        std::cout << "\n--- 测试1: 简单连接 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("abc");
            print_match(dfa, "abc", "abc连接", true);
            print_match(dfa, "ab", "abc匹配ab", false);
            print_match(dfa, "abcd", "abc匹配abcd", false);
        }

        // 测试2: 选择操作 (|)
        std::cout << "\n--- 测试2: 选择操作 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("cat|dog");
            print_match(dfa, "cat", "cat|dog匹配cat", true);
            print_match(dfa, "dog", "cat|dog匹配dog", true);
            print_match(dfa, "bat", "cat|dog匹配bat", false);
            print_match(dfa, "cats", "cat|dog匹配cats", false);
        }

        // 测试3: 通配符 (.)
        std::cout << "\n--- 测试3: 通配符 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("a.c");
            print_match(dfa, "abc", "a.c匹配abc", true);
            print_match(dfa, "a1c", "a.c匹配a1c", true);
            print_match(dfa, "ac", "a.c匹配ac", false);     // 需要中间有一个字符
            print_match(dfa, "abbc", "a.c匹配abbc", false); // 只匹配3个字符
        }

        // 测试4: 字符集 [...]
        std::cout << "\n--- 测试4: 字符集 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("a[0-9]c"); // a后跟一个数字再跟c
            print_match(dfa, "a1c", "a[0-9]c匹配a1c", true);
            print_match(dfa, "a5c", "a[0-9]c匹配a5c", true);
            print_match(dfa, "a0c", "a[0-9]c匹配a0c", true);
            print_match(dfa, "a9c", "a[0-9]c匹配a9c", true);
            print_match(dfa, "axc", "a[0-9]c匹配axc", false); // x不是数字
            print_match(dfa, "abc", "a[0-9]c匹配abc", false); // b不是数字
        }

        // 测试5: 否定字符集 [^...]
        std::cout << "\n--- 测试5: 否定字符集 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("a[^0-9]c"); // a后跟一个非数字再跟c
            print_match(dfa, "axc", "a[^0-9]c匹配axc", true);
            print_match(dfa, "a.c", "a[^0-9]c匹配a.c", true);
            print_match(dfa, "a1c", "a[^0-9]c匹配a1c", false); // 1是数字
            print_match(dfa, "a5c", "a[^0-9]c匹配a5c", false); // 5是数字
        }

        // 测试6: 量词 * (零次或多次)
        std::cout << "\n--- 测试6: 量词* ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("ab*c");      // a后跟零个或多个b，然后是c
            print_match(dfa, "ac", "ab*c匹配ac", true);     // 零个b
            print_match(dfa, "abc", "ab*c匹配abc", true);   // 一个b
            print_match(dfa, "abbc", "ab*c匹配abbc", true); // 两个b
            print_match(dfa, "abbbbc", "ab*c匹配abbbbc", true); // 多个b
            print_match(dfa, "ab", "ab*c匹配ab", false);        // 没有结尾的c
        }

        // 测试7: 量词 + (一次或多次)
        std::cout << "\n--- 测试7: 量词+ ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("ab+c");      // a后跟一个或多个b，然后是c
            print_match(dfa, "ac", "ab+c匹配ac", false);    // 需要至少一个b
            print_match(dfa, "abc", "ab+c匹配abc", true);   // 一个b
            print_match(dfa, "abbc", "ab+c匹配abbc", true); // 两个b
            print_match(dfa, "abbbbc", "ab+c匹配abbbbc", true); // 多个b
        }

        // 测试8: 量词 ? (零次或一次)
        std::cout << "\n--- 测试8: 量词? ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("ab?c");       // a后跟零个或一个b，然后是c
            print_match(dfa, "ac", "ab?c匹配ac", true);      // 零个b
            print_match(dfa, "abc", "ab?c匹配abc", true);    // 一个b
            print_match(dfa, "abbc", "ab?c匹配abbc", false); // 两个b，超出范围
        }

        // 测试9: 转义字符
        std::cout << "\n--- 测试9: 转义字符 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("a\\.c");     // a后跟字面量点号再跟c
            print_match(dfa, "a.c", "a\\.c匹配a.c", true);  // 匹配字面量点号
            print_match(dfa, "abc", "a\\.c匹配abc", false); // 不匹配其他字符
        }

        // 测试10: 复杂组合 - 选择和量词
        std::cout << "\n--- 测试10: 复杂组合 - 选择和量词 ---" << std::endl;
        {
            regex::dfa dfa = regex::build_dfa("(cat|dog)s?"); // cat或dog，后跟可选的s
            print_match(dfa, "cat", "(cat|dog)s?匹配cat", true);
            print_match(dfa, "dog", "(cat|dog)s?匹配dog", true);
            print_match(dfa, "cats", "(cat|dog)s?匹配cats", true);
            print_match(dfa, "dogs", "(cat|dog)s?匹配dogs", true);
            print_match(dfa, "catt", "(cat|dog)s?匹配catt", false);
            print_match(dfa, "catsx", "(cat|dog)s?匹配catsx", false);
        }

        // 测试11: 复杂组合 - 字符集和通配符
        std::cout << "\n--- 测试11: 复杂组合 - 字符集和通配符 ---" << std::endl;
        {
            regex::dfa dfa =
                regex::build_dfa("[a-z].[0-9]"); // 小写字母 + 任意字符 + 数字
            print_match(dfa, "a12", "[a-z].[0-9]匹配a12", true);
            print_match(dfa, "x.5", "[a-z].[0-9]匹配x.5", true);
            print_match(dfa, "A.5", "[a-z].[0-9]匹配A.5", false);   // A不是小写
            print_match(dfa, "a.55", "[a-z].[0-9]匹配a.55", false); // 太长
        }

        std::cout << "\n=== 所有测试通过! ===" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "异常: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
