rt_src = [
	'rt.c'
]

rt = static_library('rt', rt_src, include_directories: bsfs_inc, native: true)

bsfs_inc += include_directories('.')
bsfs_mod += rt