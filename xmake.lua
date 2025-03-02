set_version('0.1')
add_rules('mode.release', 'mode.debug')

set_defaultplat('mingw')
set_defaultmode('debug')
set_toolchains('gcc')

target('edl-laxer')
    set_kind('shared')
    set_default(true)
    add_files('laxer.cpp')

target('laxer-test')
    set_kind('binary')
    add_deps('edl-laxer')
    add_files('main.cpp')
