project('bsfs', 'c')

bsfs_src = [
        'main.c',
]
bsfs_inc = [include_directories('.')]
bsfs_mod = []

subdir('rt')

subdir('bs')
subdir('cmd')
subdir('io')
subdir('map')

bsfs = executable('bsfs', bsfs_src, link_with: bsfs_mod, include_directories: bsfs_inc, native: true)