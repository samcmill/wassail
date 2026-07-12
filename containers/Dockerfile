ARG IMAGE=rockylinux:8
ARG CC=gcc
ARG CFLAGS=
ARG CXX=g++
ARG CXXFLAGS=
ARG LDFLAGS=
ARG BRANCH=master
ARG REPO=https://github.com/samcmill/wassail

FROM ${IMAGE} AS base

######
### Amazon Linux 2
#####
FROM base AS base-amazonlinux2
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

FROM base-amazonlinux2 AS clang-amazonlinux2
RUN yum install -y \
        clang && \
    rm -rf /var/cache/yum/*

FROM base-amazonlinux2 AS gcc-amazonlinux2
RUN yum install -y \
        gcc \
        gcc-c++ && \
    rm -rf /var/cache/yum/*

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
ENV CPATH=/usr/include/python3.9:$CPATH

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
        python3 \
        python3-dev && \
    rm -rf /var/lib/apt/lists/*

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
FROM ${CC}-${IMAGE//:/} AS build
ARG BRANCH
ARG CC
ARG CFLAGS
ARG CXX
ARG CXXFLAGS
ARG LDFLAGS
ARG REPO

RUN mkdir -p /var/tmp && cd /var/tmp && git clone --depth=1 --branch ${BRANCH} ${REPO} wassail && cd - && \
    cd /var/tmp/wassail && \
    ./autogen.sh && \
    cd /var/tmp/wassail &&  CC=${CC} CXX=${CXX} CFLAGS=${CFLAGS} CXXFLAGS=${CXXFLAGS} LDFLAGS=${LDFLAGS} ./configure --prefix=/usr/local && \
    make -j$(nproc) && \
    make -j$(nproc) check && \
    make -j$(nproc) install && \
    rm -rf /var/tmp/wassail
