import os


cflags_debug = '-g -O0'
cflags_release = '-O2'

modes = ('release', 'debug')
mode = ARGUMENTS.get('mode', 'debug')
if mode not in ('release', 'debug'):
	print "Expected mode in " + str(modes)
	Exit(1)

cflags = cflags_debug if mode == 'debug' else cflags_release

env = Environment(
		CXX = 'clang++',
		CCFLAGS = cflags,
		CXXFLAGS = cflags + ' -std=c++11 -stdlib=libc++ -Wall -Werror ' +
			# ...and because WordNet is so ghetto...
			'-Wno-deprecated-writable-strings',
		LINKFLAGS = ['-stdlib=libc++'],
		CPPPATH = ['.', '/opt/local/include']
		)

env['ENV']['PATH'] = os.environ['PATH']

src = [ 'main.cpp' ]

Default(env.Program('rat_trap_parts', src,
			LIBS=['WN', 'hunspell-1.3', 'ncurses'], LIBPATH='/opt/local/lib'))
