project('XRootGIF', 'c', license : 'MIT', version : '1.4.2', default_options : [ 'optimization=2', 'c_link_args=-lgif' ] )
incdir = include_directories('src/')
fsmod = import('fs')

#Dependencies
x11_dep = dependency('x11')
imlib2_dep = dependency('imlib2')
if fsmod.exists('/usr/include/gif_lib.h') == true #Really got to figure out a macro for the include directory. But it seems consistant across distributions that gif_lib.h is found in /usr/include
	message('giflib found.')
else
	error('giflib not found.')
endif

#Multimonitor
multimonitor_support = 0
if get_option('multimonitor_feature') == true
	message('Attempting to enable multimonitor support...')
	xrandr_dep = dependency('xrandr', required : false)
	if xrandr_dep.found()
		message('Xrandr will be used for multimonitor support.')
		multimonitor_support = 1
	else
		message('Xrandr not found, multimonitor support will not be included.')
		multimonitor_support = 0
	endif
else
	message('Multimonitor support is disabled in meson_options.txt, compiling without multimonitor support.')
endif

src_files = ['src/xrootgif.c', 'src/globals.c', 'src/output.c', 'src/sample.c', 'src/gif.c', 'src/pixmap_allocate.c', 'src/daemon.c']
# Autocompletion files
install_data(['src/utils/zsh-completion.in', 'src/utils/bash-completion.in'], rename : ['zsh/site-functions/_xrootgif', 'bash-completion/completions/xrootgif'], install_dir : 'share')
# License
install_data('src/LICENSE.txt',  install_dir : 'share/licenses/XRootGIF')

#Build
if multimonitor_support == 1
	executable('xrootgif', src_files, dependencies : [x11_dep, imlib2_dep, xrandr_dep], include_directories : incdir, install : true, c_args : [ '-zrelro', '-znow', '-D HAVE_XRANDR' ] )
else
	executable('xrootgif', src_files, dependencies : [x11_dep, imlib2_dep], include_directories : incdir, install : true, c_args : [ '-zrelro', '-znow' ] )
endif

