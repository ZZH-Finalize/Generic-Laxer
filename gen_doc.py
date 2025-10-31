#! python3
# -*- coding: utf-8 -*-

import subprocess

DOC_FMT = """# 正则表达式可视化

[TOC]

"""

SECTIONS = {
    '单字符': ['a'],
    '多字符': ['abc'],
    '量词': {
        '?': ['a?'],
        '+': ['a+'],
        '*': ['a*'],
    },
    '字符集': ['[abc]'],
    '选择': ['a|b|c'],
    '复杂案例': ['([abc]abc)*|(d|e|f)ghi[2-5]+'],
}

def run_visualizer(exp: str) -> str:
    result = subprocess.run(['xmake', 'run', 'vreg', exp], capture_output=True, text=True)
    return result.stdout

def main():
    doc = ''

    section_num, sub_section_num = 1, 1

    for section, content in SECTIONS.items():
        doc += f'## {section_num}. {section}\n\n'

        if(isinstance(content, dict)):
            for sub_section, sub_content in content.items():
                doc += f'### {section_num}.{sub_section_num}. {sub_section}\n\n'

                for regexp in sub_content:
                    doc += f'{run_visualizer(regexp)}'

                sub_section_num += 1
        else:
            for regexp in content:
                doc += f'{run_visualizer(regexp)}'

        section_num += 1
    
    with open('regex_visualize.md', 'w', encoding='utf-8') as f:
        f.write(DOC_FMT + doc)

if __name__ == '__main__':
    main()
