project('edl-laxer', 'cpp',
    version: '0.1',
    license: 'GPL-2.0',
    # license_files: 'LICENSE',
    default_options: ['warning_level=everything',],
)

system_name = target_machine.system()
build_dir = meson.current_build_dir()
src_dir = meson.current_source_dir()

lib_srcs = [
    'laxer.cpp'
]

test_cases = [
    'main.cpp'
]

include_directories = [
    'D:/mingw64/include/',
    'D:/mingw64/x86_64-w64-mingw32/include/',
    'D:/mingw64/lib/gcc/x86_64-w64-mingw32/13.2.0/include/',   
    'D:/mingw64/lib/gcc/x86_64-w64-mingw32/13.2.0/include/c++/',
    'D:/mingw64/lib/gcc/x86_64-w64-mingw32/13.2.0/include/c++/x86_64-w64-mingw32/',
]

cpp_args = []
ld_args = []

fs = import('fs')

build_setup = {
    'debug': {
        'c_args': [
            '-g3',
            '-O0'
        ]
    },
    'release': {
        'c_args': [
            '-O2'
        ]
    }
}

foreach setup_name, options: build_setup

    lib = shared_library('edl-laxer-' + setup_name,
        sources: lib_srcs,
        build_by_default: true,
        include_directories: include_directories
    )

    exe = executable('laxer-test-' + setup_name, test_cases,
        link_with: lib,
        include_directories: include_directories
    )

    test('main-test-' + setup_name, exe)

endforeach
