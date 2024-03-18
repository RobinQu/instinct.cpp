# How to build

## ubuntu jammy

Update to gcc 13 

```shell
sudo apt install software-properties-common
sudo add-apt-repository ppa:ubuntu-toolchain-r/test
sudo apt update
sudo apt install gcc-12 g++-12 gcc-13 g++-13 -y

sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-12 12 --slave /usr/bin/g++ g++ /usr/bin/g++-12
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-13 13 --slave /usr/bin/g++ g++ /usr/bin/g++-13


```


Initialize Cmake presets with conan:

```shell
# detect profile
conan profile detect --force
# make sure compiler.cppstd=gnu20

# install deps
conan install conanfile.py --build=missing
```

