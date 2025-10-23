# 正则表达式到自动机转换器

这是一个完整的从正则表达式转换为有限自动机的C++实现，支持将正则表达式转换为NFA（非确定性有限自动机）和DFA（确定性有限自动机），支持基本的正则表达式操作。

## 功能特性

- **基本字符匹配** - 匹配单个字符，如 `a`, `b`, `1`, 等
- **连接操作** - 匹配序列，如 `ab`, `abc`, 等
- **选择操作** - 匹配任一选项，如 `a|b`, `cat|dog`, 等
- **量词操作** -
  - `?` 零次或一次
  - `*` 零次或多次
  - `+` 一次或多次
- **分组操作** - 使用括号进行分组，如 `(ab)*`, `(a|b)+`, 等
- **复杂表达式** - 支持嵌套和组合的复杂表达式
- **NFA到DFA转换** - 使用子集构造算法将NFA转换为DFA

## 实现原理

### 1. 数据结构
- `State`: 表示NFA中的状态，包含状态ID、是否为终态、以及字符到状态的转换映射
- `NFA`: 表示非确定性有限自动机，包含状态列表、起始状态和终态
- `DFAState`: 表示DFA中的状态
- `DFA`: 表示确定性有限自动机

### 2. 核心算法
- **Thompson构造算法**: 用于将正则表达式转换为NFA
- **子集构造算法**: 用于将NFA转换为DFA
- **epsilon闭包算法**: 用于处理NFA中的空转换

### 3. 主要组件
- `RegexToNFA`: 正则表达式解析和NFA构造器
- `NFAtoDFAConverter`: NFA到DFA的转换器
- `NFA_Matcher`: NFA匹配器
- `DFA_Matcher`: DFA匹配器

## 使用示例

```cpp
#include "regex_demo.hpp"
#include <iostream>

int main() {
    Regex::RegexToNFA converter;
    
    // 转换正则表达式为NFA
    Regex::NFA nfa = converter.convert("a|b");  // 匹配'a'或'b'
    
    // 将NFA转换为DFA
    Regex::DFA dfa = Regex::NFAtoDFAConverter::convert(nfa);
    
    // 测试NFA匹配
    bool nfa_match = Regex::NFA_Matcher::match(nfa, "a");  // 返回 true
    std::cout << "NFA match 'a': " << nfa_match << std::endl;
    
    // 测试DFA匹配
    bool dfa_match = Regex::DFA_Matcher::match(dfa, "a");  // 返回 true
    std::cout << "DFA match 'a': " << dfa_match << std::endl;
    
    return 0;
}
```

## 算法细节

### Thompson构造算法（正则表达式到NFA）
- **基本字符**: 创建两个状态，从起始到终态有对应字符的转换
- **选择操作 (|)**: 对于 `e1|e2`:
  1. 创建新的起始状态和终态
  2. 将 e1 和 e2 的NFA复制到结果中
  3. 添加epsilon转换从新起始状态到 e1 和 e2 的起始状态
  4. 添加epsilon转换从 e1 和 e2 的终态到新终态
- **量词操作**:
  - `e?`: 添加epsilon转换绕过e的NFA，允许0次匹配
  - `e*`: 添加epsilon循环和绕过转换，允许0次或多次匹配
  - `e+`: 添加epsilon循环，但必须至少匹配1次
- **连接操作**: 对于 `e1e2`: 将 e2 的起始状态连接到 e1 的终态（通过epsilon转换）

### 子集构造算法（NFA到DFA）
- 将NFA的状态集合映射为DFA的单个状态
- 使用epsilon闭包来处理空转换
- 为每个输入字符计算可达的状态集合

## 性能特点

- **NFA**: 构造简单，但匹配时需要处理epsilon转换和状态集合
- **DFA**: 匹配速度快（每个输入字符只需一次状态转换），但状态数量可能指数级增长
- 使用 `std::shared_ptr` 管理状态，避免深拷贝问题
- 支持复杂的嵌套表达式

## 编译和运行

使用xmake构建系统：

```bash
# 构建所有测试
xmake

# 运行简单测试
xmake run simple-test

# 运行复杂测试
xmake run test-complex

# 运行完整测试
xmake run regex-test

# 运行DFA转换测试
xmake run test-dfa
```

## 限制

- 目前支持基本的ASCII字符匹配
- 不支持字符类（如 [a-z]）和特殊字符（如 ., ^, $）
- DFA构造可能产生大量状态（状态爆炸问题）
