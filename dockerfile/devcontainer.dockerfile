ARG UBUNTU_VERSION=23.04
FROM ubuntu:$UBUNTU_VERSION
ARG GCC_VERSION=12

# install system packages
ENV DEBIAN_FRONTEND=noninteractive
RUN sed -i s@/archive.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list \
    && sed -i s@/ports.ubuntu.com/@/mirrors.aliyun.com/@g /etc/apt/sources.list \
    && apt-get -y update --no-install-recommends \
    && apt-get -y install --no-install-recommends \
    build-essential \
    curl \
    ca-certificates \
    apt-utils \
    dialog \
    git \
    vim \
    gcc-$GCC_VERSION \
    g++-$GCC_VERSION \
    wget \
    clang \
    && apt-get autoremove -y \
    && apt-get clean -y \
    && update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-$GCC_VERSION $GCC_VERSION --slave /usr/bin/g++ g++ /usr/bin/g++-$GCC_VERSION
ENV DEBIAN_FRONTEND=dialog

# install miniconda
RUN mkdir -p ~/miniconda3  && \
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-$(uname -s)-$(uname -m).sh -O ~/miniconda3/miniconda.sh  && \
    bash ~/miniconda3/miniconda.sh -b -u -p ~/miniconda3  && \
    rm -rf ~/miniconda3/miniconda.sh  && \
    ~/miniconda3/bin/conda init bash
ENV PATH=~/miniconda3/bin:$PATH
RUN ~/miniconda3/bin/conda install -y -c conda-forge conan cmake

COPY ./dockerfile/conan-profile/ /tmp/
RUN --mount=type=cache,id=id=conan-$TARGETARCH$TARGETVARIANT,target=/root/.conan2,sharing=locked \
    mkdir -p /root/.conan2/profiles/ && cp /tmp/$(uname -m)/* /root/.conan2/profiles/

SHELL ["conda", "run", "/bin/bash", "-c"]
ENTRYPOINT /bin/bash