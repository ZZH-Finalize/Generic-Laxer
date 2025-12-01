set_version('0.1')
add_rules('mode.release', 'mode.debug')
add_rules('plugin.compile_commands.autoupdate', {outputdir = '$(builddir)'})

add_requires('nlohmann_json')

set_defaultplat('mingw')
set_defaultmode('debug')
set_toolchains('gcc')
set_languages('c++latest')
-- set_warnings('everything')
set_warnings('error')

includes('regex')

target('vreg')
    set_kind('binary')
    add_deps('regex-engine')
    add_files('visualize_regexp.cpp')

target('laxer-engine')
    set_kind('shared')
    add_includedirs('.', {public=true})
    add_deps('regex-engine')

target('laxer')
    set_kind('binary')
    add_deps('laxer-engine')

    add_files('main.cpp')

target('demo')
    set_kind('binary')
    add_deps('laxer-engine')

    add_files('simple_grammar.cpp')

for _, file in ipairs(os.files('testcases/*.cpp')) do
    local name = path.basename(file)
    target('laxer_' .. name)
        set_kind('binary')
        set_default(false)
        add_deps('laxer-engine')
        add_files(file)
        add_tests('laxer', {timeout = 1})
end
