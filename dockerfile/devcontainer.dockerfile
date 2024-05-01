FROM mcr.microsoft.com/devcontainers/miniconda:3

RUN conda install -y -c conda-forge clang==15.0.7 conan

SHELL ["conda", "run", "/bin/bash", "-c"]
