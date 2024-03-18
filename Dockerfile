FROM mcr.microsoft.com/devcontainers/base:ubuntu

# install gcc 13
RUN --mount=type=cache,target=/var/cache/apt && \
    sudo apt update && \
    sudo apt install software-properties-common && \
    sudo add-apt-repository ppa:ubuntu-toolchain-r/test && \
    sudo apt update && \
    sudo apt install gcc-12 g++-12 gcc-13 g++-13 -y && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 --slave /usr/bin/g++ g++ /usr/bin/g++-12 && \
    sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 13 --slave /usr/bin/g++ g++ /usr/bin/g++-13

# install miniconda
RUN mkdir -p ~/miniconda3  && \
    wget https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh -O ~/miniconda3/miniconda.sh  && \
    bash ~/miniconda3/miniconda.sh -b -u -p ~/miniconda3  && \
    rm -rf ~/miniconda3/miniconda.sh  && \
    ~/miniconda3/bin/conda init bash

# create env with cmake and conan
RUN conda env create -n instinct -c conda-forge python=3.11 cmake && \
    pip install conan

SHELL ["conda", "run", "-n", "instinct", "/bin/bash", "-c"]
