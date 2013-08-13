def options(opt):
	opt.load("compiler_c")
	opt.add_option("--debug",action="store_true",default=False,help="Debug traces")

def configure(cnf):
	cnf.load("compiler_c")
	cnf.env.append_value("CFLAGS",["-Wall"])

	if cnf.options.debug:
		cnf.env.append_value("CFLAGS",["-DDEBUG"])

	cnf.check(
		features='c cprogram', 
		lib=[
			'avcodec', 'avformat', 'iconv', 'm', 'mp3lame', 'bz2', 'z', 'm', 'avutil', 'jansson'
		],
		uselib_store='AV'
	)

def build(bld):
	bld(
		features="c cprogram", 
		source=[
			'extern/ffsnd/src/ffsndin.c','extern/ffsnd/src/ffsndutil.c','extern/ffsnd/src/ffsndout.c',
			'src/clmix.c',

			'src/audioencoder.c',		'src/audiosource_mix.c',	'src/ladspa_load.c',
			'src/audiosource.c',		'src/audiosource_pan.c',	'src/envelope.c',			'src/ladspa_search.c',
			'src/audiosource_file.c',	'src/audiosource_repos.c',	'src/envelope_constant.c',	'src/mixtree.c',
			'src/audiosource_ladspa.c',	'src/audiosource_trim.c',	'src/envelope_linear.c',	'src/util.c',
			'src/audiosource_vol.c',	'src/ladspa_default.c'
		],
		target='clmix',
		use=['AV'],
		includes='extern/ffsnd/src'
	)