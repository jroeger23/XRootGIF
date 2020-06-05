project('XRootGIF', 'c', license : 'MIT', version : '1.4.2', default_options : [ 'optimization=g', 'c_args=-zrelro,-znow', 'c_link_args=-lgif' ] )
incdir = include_directories('src/')
fsmod = import('fs')
#Dependencies
x11_dep = dependency('x11')
imlib2_dep = dependency('imlib2')
if fsmod.exists('/usr/include/gif_lib.h') == true
	message('gitlib found.')
else
	message('giflib not found.')
endif
src_files = ['src/xrootgif.c', 'src/globals.c', 'src/output.c', 'src/sample.c', 'src/gif.c', 'src/pixmap_allocate.c', 'src/daemon.c']
executable('xrootgif', src_files, dependencies : [x11_dep, imlib2_dep,], include_directories : incdir)
