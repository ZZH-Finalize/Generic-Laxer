set_version('0.1')
add_rules('mode.release', 'mode.debug')
add_rules('plugin.compile_commands.autoupdate', {outputdir = '$(builddir)'})

add_requires('nlohmann_json')

set_defaultplat('mingw')
set_defaultmode('debug')
set_toolchains('gcc')
set_languages('c++latest')

add_includedirs('.')

target('edl-laxer')
    set_kind('shared')

    add_packages('nlohmann_json')
    set_default(true)
    add_files('laxer.cpp')

target('laxer-test')
    set_kind('binary')
    add_deps('edl-laxer')
    add_files('main.cpp')

for _, file in ipairs(os.files('regex/testcases/*.cpp')) do
    local name = path.basename(file)
    target(name)
        set_kind('binary')
        set_default(false)
        add_includedirs('regex')
        add_files(file)
        add_tests('default', {timeout = 1})
        -- add_tests("args", {runargs = {"foo", "bar"}})
        -- add_tests("pass_output", {trim_output = true, runargs = "foo", pass_outputs = "hello foo"})
        -- add_tests("fail_output", {fail_outputs = {"hello2 .*", "hello xmake"}})
end
