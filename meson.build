project('relax', 'cpp', default_options : ['warning_level=3'])

cpp = meson.get_compiler('cpp')

boost_dep = dependency('boost', modules : ['program_options'])
curl_dep = cpp.find_library('curl')
jsoncpp_dep = cpp.find_library('jsoncpp')

executable('relax', 'init.cpp', dependencies : [boost_dep, curl_dep, jsoncpp_dep])

