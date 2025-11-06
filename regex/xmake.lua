set_version('0.1')
add_rules('mode.release', 'mode.debug')
add_rules('plugin.compile_commands.autoupdate', {outputdir = '$(builddir)'})

set_defaultplat('mingw')
set_defaultmode('debug')
set_toolchains('gcc')
set_languages('c++latest')

target('regex-engine')
    set_kind('shared')
    add_includedirs('.', {public=true})

    add_files(
        'nfa.cpp',
        'nfa_builder.cpp',
        'dfa.cpp',
        'dfa_builder.cpp'
    )

for _, file in ipairs(os.files('testcases/*.cpp')) do
    local name = path.basename(file)
    target('regex_' .. name)
        set_kind('binary')
        set_default(false)
        add_deps('regex-engine')
        add_files(file)
        add_tests('regex', {timeout = 1})
end
