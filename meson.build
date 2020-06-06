project('XRootGIF', 'c', license : 'MIT', version : '1.4.2', default_options : [ 'optimization=2', 'c_args=-zrelro,-znow', 'c_link_args=-lgif' ] )
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
xrandr_dep = dependency('xrandr', required : false)
if xrandr_dep.found()
	message('Xrandr will be used for multimonitor support.')
else
	message('Xrandr not found, multimonitor support will not be included.')
endif
src_files = ['src/xrootgif.c', 'src/globals.c', 'src/output.c', 'src/sample.c', 'src/gif.c', 'src/pixmap_allocate.c', 'src/daemon.c']
executable('xrootgif', src_files, dependencies : [x11_dep, imlib2_dep, xrandr_dep], include_directories : incdir, install : true)


