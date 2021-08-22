# HPC Container Maker recipe
# https://github.com/NVIDIA/hpc-container-maker

Stage0 += comment('wassail container')

image = USERARG.get('baseimage', 'ubuntu:20.04')
toolset = USERARG.get('toolset', 'gnu')
apt = []

if toolset == 'nvhpc':
  Stage0 += baseimage(image='nvcr.io/nvidia/nvhpc:21.7-devel-cuda11.4-ubuntu20.04')
  toolchain = hpccm.toolchain(CC='nvc', CXX='nvc++', CXXFLAGS='-O1')
elif toolset == 'oneapi':
  Stage0 += baseimage(image='intel/oneapi-hpckit:2021.3-devel-ubuntu18.04')
  toolchain = hpccm.toolchain(CC='icx', CXX='icpx')
else:
  if image == 'ubuntu:20.04':
    Stage0 += baseimage(image='ubuntu:20.04')
  elif image == 'ubuntu:18.04':
    Stage0 += baseimage(image='ubuntu:18.04')
    apt = ['pciutils-dev']
  elif image == 'ubuntu:16.04':
    Stage0 += baseimage(image='ubuntu:16.04')
    apt = ['pciutils-dev']
  elif image == 'centos:8':
    Stage0 += baseimage(image='centos:8')
  elif image == 'rockylinux:8':
    Stage0 += baseimage(image='rockylinux/rockylinux:8')
  elif image == 'centos:7':
    Stage0 += baseimage(image='centos:7')
  else:
    raise RuntimeError('unrecognized baseimage')

  if toolset == 'gnu':
    if image == 'centos:7':
      compiler = gnu(fortran=False, version='7')
    else:
      compiler = gnu(fortran=False)
      apt.extend(['libopenmpi-dev', 'openmpi-bin'])
  elif toolset == 'llvm':
    compiler = llvm()
  else:
    raise RuntimeError('unrecognized toolset')

  Stage0 += compiler
  toolchain = compiler.toolchain

# Python development environment
Stage0 += python(python2=False, devel=True)

# Development tools
Stage0 += packages(ospackages=['autoconf', 'autoconf-archive', 'automake',
                               'ca-certificates', 'diffutils', 'file', 'git',
                               'libtool', 'make'],
                   powertools=True)

# wassail dependencies
apt.extend(['libdispatch-dev', 'libibumad-dev', 'libpciaccess-dev',
            'libssh-dev', 'libudev-dev'])
Stage0 += packages(apt=apt,
                   yum=['libpciaccess-devel', 'libssh-devel', 'pciutils-devel',
                        'rdma-core-devel', 'systemd-devel', 'tbb'])

Stage0 += generic_autotools(branch='master', check=True,
                            preconfigure=['./autogen.sh'],
                            repository='https://github.com/samcmill/wassail',
                            toolchain=toolchain)
Stage0 += environment(variables={'LD_LIBRARY_PATH':
                                 '/usr/local/lib:$LD_LIBRARY_PATH'})
