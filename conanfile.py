import os

from conan import ConanFile
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import copy, get


class InstinctCppRecipe(ConanFile):
    name = "instinct-cpp"
    version = "0.1.5"
    settings = "os", "compiler", "build_type", "arch"
    no_copy_source = True
    options = {
        # module-level switches (instinct-proto and instinct-core cannot be switched off)
        "with_transformer_module": [True, False],
        "with_server_module": [True, False],

        # target-level switches
        "with_duckdb": [True, False],
        "with_exprtk": [True, False],
        "with_pdfium": [True, False],
        "with_duckx": [True, False],
    }

    default_options = {
        "with_transformer_module": True,
        "with_server_module": True,
        "with_duckdb": True,
        "with_exprtk": True,
        "with_pdfium": True,
        "with_duckx": True
    }

    @property
    def _min_cppstd(self):
        return "20"

    def package_id(self):
        self.info.clear()

    def validate(self):
        check_min_cppstd(self, 20)

    def layout(self):
        cmake_layout(self)

    def requirements(self):
        self.requires("hash-library/8.0")
        self.requires("bshoshany-thread-pool/4.1.0")

        if self.options.with_duckdb:
            self.requires("duckdb/0.10.2", options={"with_httpfs": True})

        self.requires("uriparser/0.9.7")
        self.requires("crossguid/0.2.2")
        self.requires("protobuf/5.27.0")
        self.requires("reactiveplusplus/2.1.1")
        self.requires("icu/74.1", options={"with_extras": True, "data_packaging": "static"})
        self.requires("tsl-ordered-map/1.1.0")
        self.requires("fmt/10.2.1")
        self.requires("fmtlog/2.2.1")
        self.requires("nlohmann_json/3.11.3")
        self.requires("base64/0.5.2")
        self.requires("libcurl/8.6.0")
        self.requires("inja/3.4.0")
        self.requires("concurrentqueue/1.0.4")
        self.requires("cpptrace/0.5.4")

        if self.options.with_pdfium:
            self.requires("pdfium/95.0.4629")

        if self.options.with_duckx:
            self.requires("duckx/1.2.2")

        if self.options.with_exprtk:
            self.requires("exprtk/0.0.2")

        if self.options.with_transformer_module:
            self.requires("cpp-httplib/0.15.3")

        # deps of examples
        self.requires("cli11/2.4.1")
        # test deps
        self.test_requires("gtest/1.14.0")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        # cmake = CMake(self)
        # cmake.install()
        copy(self, "LICENSE", src=self.source_folder, dst=os.path.join(self.package_folder, "LICENSE"))
        copy(self, "*",
             src=os.path.join(self.package_folder, "modules/instinct"),
             dst=os.path.join(self.package_folder, "include", "instinct")
         )

    def package_info(self):
        self.cpp_info.libs = ["hello"]
