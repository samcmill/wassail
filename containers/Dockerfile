ARG IMAGE=rockylinux:8
ARG SHORT_IMAGE=${IMAGE//:/}
ARG CC=gcc
ARG CFLAGS=
ARG CXX=g++
ARG CXXFLAGS=
ARG LDFLAGS=
ARG PYTHON=
ARG BRANCH=main
ARG REPO=https://github.com/samcmill/wassail

FROM ${IMAGE} AS base

######
### Amazon Linux 2023
#####
FROM base AS base-amazonlinux2023
RUN yum install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python3 \
        python3-devel \
        rdma-core-devel \
        systemd-devel \
        tbb && \
    rm -rf /var/cache/yum/*

FROM base-amazonlinux2023 AS clang-amazonlinux2023
RUN yum install -y \
        clang && \
    rm -rf /var/cache/yum/*

FROM base-amazonlinux2023 AS gcc-amazonlinux2023
RUN yum install -y \
        gcc \
        gcc-c++ && \
    rm -rf /var/cache/yum/*

######
### OneAPI (Ubuntu 26.04)
######
FROM base AS base-oneapi
RUN apt-get update -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libibumad-dev \
        libpciaccess-dev \
        libpci-dev \
        libssh-dev \
        libtool \
        libudev-dev \
        make \
        python3 \
        python3-dev && \
    rm -rf /var/lib/apt/lists/*

FROM base-oneapi AS icx-oneapi

######
### OpenSUSE Leap 15.6
######
FROM base AS base-opensuse15
RUN zypper install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python39 \
        python39-devel \
        rdma-core-devel \
        systemd-devel \
        tbb-devel && \
    zypper clean -a && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 1

FROM base-opensuse15 AS clang-opensuse15
RUN zypper install -y \
        clang && \
    zypper clean -a

FROM base-opensuse15 AS gcc-opensuse15
RUN zypper install -y \
        gcc \
        gcc-c++ \
        openmpi4-devel && \
    zypper clean -a
ENV PATH="/usr/lib64/mpi/gcc/openmpi4/bin:$PATH" \
    LD_LIBRARY_PATH="/usr/lib64/mpi/gcc/openmpi4/lib64"

######
### OpenSUSE Leap 16.0
######
FROM base AS base-opensuse16
RUN zypper install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python3 \
        python3-devel \
        rdma-core-devel \
        systemd-devel \
        tbb-devel && \
    zypper clean -a

FROM base-opensuse16 AS clang-opensuse16
RUN zypper install -y \
        clang && \
    zypper clean -a

FROM base-opensuse16 AS gcc-opensuse16
RUN zypper install -y \
        gcc \
        gcc-c++ \
        openmpi5-devel && \
    zypper clean -a
ENV PATH="/usr/lib64/mpi/gcc/openmpi5/bin:$PATH" \
    LD_LIBRARY_PATH="/usr/lib64/mpi/gcc/openmpi5/lib64"

######
### NVHPC (Ubuntu 24.04)
######
FROM base AS base-nvhpc
RUN apt-get update -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libibumad-dev \
        libpciaccess-dev \
        libssh-dev \
        libtool \
        libudev-dev \
        make \
        python3 \
        python3-dev && \
    rm -rf /var/lib/apt/lists/*

FROM base-nvhpc AS nvc-nvhpc

######
### Rockylinux 8
######
FROM base AS base-rockylinux8
RUN yum install -y dnf-utils && \
    yum-config-manager --set-enabled powertools && \
    yum install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python39 \
        python39-devel \
        rdma-core-devel \
        systemd-devel \
        tbb && \
    rm -rf /var/cache/yum/*

FROM base-rockylinux8 AS clang-rockylinux8
RUN yum install -y \
        clang && \
    rm -rf /var/cache/yum/*

FROM base-rockylinux8 AS gcc-rockylinux8
RUN yum install -y \
        gcc \
        gcc-c++ && \
    rm -rf /var/cache/yum/*

######
### Rockylinux 9
######
FROM base AS base-rockylinux9
RUN  dnf install -y dnf-utils && \
     dnf config-manager --set-enabled crb && \
     dnf install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python \
        python-devel \
        rdma-core-devel \
        systemd-devel \
        tbb && \
    dnf clean all

FROM base-rockylinux9 AS clang-rockylinux9
RUN dnf install -y \
        clang && \
    dnf clean all

FROM base-rockylinux9 AS gcc-rockylinux9
RUN dnf install -y \
        gcc \
        gcc-c++ && \
    dnf clean all

######
### Rockylinux 10
######
FROM base AS base-rockylinux10
RUN  dnf install -y dnf-utils && \
     dnf config-manager --set-enabled crb && \
     dnf install -y \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libpciaccess-devel \
        libssh-devel \
        libtool \
        make \
        pciutils-devel \
        procps \
        python \
        python-devel \
        rdma-core-devel \
        systemd-devel \
        tbb && \
    dnf clean all

FROM base-rockylinux10 AS clang-rockylinux10
RUN dnf install -y \
        clang && \
    dnf clean all

FROM base-rockylinux10 AS gcc-rockylinux10
RUN dnf install -y \
        gcc \
        gcc-c++ && \
    dnf clean all

######
### Ubuntu 20.04
######
FROM base AS base-ubuntu20.04
RUN apt-get update -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        autoconf \
        autoconf-archive \
        automake \
        ca-certificates \
        diffutils \
        file \
        git \
        libibumad-dev \
        libpciaccess-dev \
        libssh-dev \
        libtool \
        libudev-dev \
        make \
        python3.9 \
        python3.9-dev && \
    rm -rf /var/lib/apt/lists/* && \
    update-alternatives --install /usr/bin/python3 python3 /usr/bin/python3.9 1

FROM base-ubuntu20.04 AS clang-ubuntu20.04
RUN apt-get update -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        clang \
        libomp-dev && \
    rm -rf /var/lib/apt/lists/*

FROM base-ubuntu20.04 AS gcc-ubuntu20.04
RUN apt-get update -y && \
    DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        g++ \
        gcc \
        libopenmpi-dev \
        openmpi-bin && \
    rm -rf /var/lib/apt/lists/*

######
### wassail
######
FROM ${CC}-${SHORT_IMAGE} AS build
ARG BRANCH
ARG CC
ARG CFLAGS
ARG CXX
ARG CXXFLAGS
ARG LDFLAGS
ARG PYTHON
ARG REPO

RUN mkdir -p /var/tmp && cd /var/tmp && git clone --depth=1 --branch ${BRANCH} ${REPO} wassail && cd - && \
    cd /var/tmp/wassail && \
    ./autogen.sh && \
    cd /var/tmp/wassail &&  CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} CXXFLAGS=${CXXFLAGS} LDFLAGS=${LDFLAGS} PYTHON=${PYTHON} ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make -j$(nproc) check && \
    make -j$(nproc) install && \
    LD_LIBRARY_PATH="${LD_LIBRARY_PATH:+${LD_LIBRARY_PATH}:}/usr/local/lib" make -j$(nproc) installcheck && \
    rm -rf /var/tmp/wassail
