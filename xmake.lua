set_version('0.1')
add_rules('mode.release', 'mode.debug')
add_rules('plugin.compile_commands.autoupdate', {outputdir = '$(builddir)'})

add_requires('nlohmann_json')

set_defaultplat('mingw')
set_defaultmode('debug')
set_toolchains('gcc')
set_languages('c++latest')

target('edl-laxer')
    set_kind('shared')

    add_packages('nlohmann_json')
    set_default(true)
    add_files('laxer.cpp')

target('laxer-test')
    set_kind('binary')
    add_deps('edl-laxer')
    add_files('main.cpp')

target('regex-test')
    set_kind('binary')
    add_files('regex_demo/regex_test.cpp')

target('simple-test')
    set_kind('binary')
    add_files('regex_demo/simple_test.cpp')

target('test-complex')
    set_kind('binary')
    add_files('regex_demo/test_complex.cpp')

target('test-dfa')
    set_kind('binary')
    add_files('regex_demo/test_dfa.cpp')
