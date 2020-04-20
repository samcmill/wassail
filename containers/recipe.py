# HPC Container Maker recipe
# https://github.com/NVIDIA/hpc-container-maker

Stage0 += comment('wassail container')

image = USERARG.get('baseimage', 'ubuntu:18.04')

if image == 'ubuntu:18.04':
  Stage0 += baseimage(image='ubuntu:18.04')
elif image == 'ubuntu:16.04':
  Stage0 += baseimage(image='ubuntu:16.04')
elif image == 'centos:8':
  Stage0 += baseimage(image='centos:8')
elif image == 'centos:7':
  Stage0 += baseimage(image='centos:7')
else:
  raise RuntimeError('unrecognized baseimage')

Stage0 += packages(ospackages=['autoconf', 'autoconf-archive', 'automake',
                               'ca-certificates', 'diffutils', 'file', 'git',
                               'libtool', 'make'],
                   powertools=True)

Stage0 += python(python2=False, devel=True)

if image == 'centos:7':
  compiler = gnu(fortran=False, version='7')
else:
  compiler = gnu(fortran=False)
Stage0 += compiler

Stage0 += packages(apt=['libdispatch-dev', 'libibumad-dev', 'libpciaccess-dev',
                        'libssh-dev', 'libudev-dev', 'pciutils-dev'],
                   yum=['libpciaccess-devel', 'libssh-devel', 'pciutils-devel',
                        'rdma-core-devel', 'systemd-devel', 'tbb'])

Stage0 += generic_autotools(branch='master', check=True,
                            preconfigure=['autoreconf -iv'],
                            repository='https://github.com/samcmill/wassail',
                            toolchain=compiler.toolchain)
Stage0 += environment(variables={'LD_LIBRARY_PATH':
                                 '/usr/local/lib:$LD_LIBRARY_PATH'})
