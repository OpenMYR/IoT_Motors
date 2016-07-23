import os
base_env = Environment(
	ENV = {'PATH' : os.environ['PATH']},
	CC = 'xtensa-lx106-elf-gcc', 
	CPPPATH = ['.', './include'],
	CCFLAGS = '-mlongcalls',
	LINKFLAGS = '-Teagle.app.v6.ld',
	LIBS = ['main', 'net80211', 'wpa', 'lwip', 'pp', 'phy'],
	LINKCOM = '$LINK $LINKFLAGS $SOURCES -nostdlib -Wl,--start-group $_LIBFLAGS -Wl,--end-group -lgcc -o $TARGET')

Export('base_env')

cmocka_env = Environment(
	ENV = {'PATH' : os.environ['PATH']},
	CC = 'gcc', 
	CPPPATH = ['./cmocka/src', './cmocka/include'],
	CCFLAGS = '-DHAVE_SIGNAL_H')

Export('cmocka_env')

test_env = Environment(
	ENV = {'PATH' : os.environ['PATH']},
	CC = 'gcc',
	CPPPATH = ['.', './include', './include/test/esp8266', '../../ext/cmocka/include'],
	CCFLAGS = '-DTEST_MODE=1'
	)

Export('test_env')

SConscript('ext/SConscript', variant_dir = './build/out/test', duplicate = 0)
SConscript('src/SConscript', variant_dir = './build/out', duplicate = 0)
SConscript('build/SConscript')
