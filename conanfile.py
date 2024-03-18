from conan import ConanFile
from conan.tools.cmake import cmake_layout


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "CMakeToolchain"

    def requirements(self):
        self.requires("crossguid/0.2.2")
        self.requires("protobuf/3.21.12")
        self.requires("reactiveplusplus/2.0.0")
        self.requires("icu/74.2")
        self.requires("tsl-ordered-map/1.1.0")
        self.requires("fmt/10.2.1")
        self.requires("fmtlog/2.2.1")
        self.requires("nlohmann_json/3.11.3")
        self.requires("base64/0.5.2")
        # test deps
        self.requires("gtest/1.14.0")

    def layout(self):
        cmake_layout(self)