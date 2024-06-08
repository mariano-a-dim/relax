project('relax', 'cpp', default_options : ['warning_level=3'])

cpp = meson.get_compiler('cpp')

boost_dep = dependency('boost', modules : ['program_options'])
curl_dep = cpp.find_library('curl')
jsoncpp_dep = cpp.find_library('jsoncpp')

# Incluir ambas posibles rutas de jsoncpp
jsoncpp_inc1 = include_directories('/usr/include/json')
# jsoncpp_inc2 = include_directories('/usr/include/jsoncpp/json')

executable('relax', 
    ['init.cpp', 'chatgptclient.cpp'], 
    include_directories: [jsoncpp_inc1],
    dependencies : [boost_dep, curl_dep, jsoncpp_dep]
)





