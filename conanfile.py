from conans import ConanFile, CMake
import os

class LimeConan(ConanFile):
    name = "lime"
    version = "0.1"
    license = "BSD 3-clause"
    author = "Zachary Stiers"
    url = "https://github.com/zstiers/lime"
    description = 'Relational component framework'
    settings = "os", "compiler", "build_type", "arch"
    options = { "shared": [True, False] }
    default_options = "shared=True"
    generators = "cmake"
    exports_sources = ("CMakeLists.txt", "inc/*", "src/*", "test/*", "LICENSE")
    requires = (
        "boost/1.73.0",
        "fmt/5.3.0@bincrafters/stable",
        "spdlog/1.3.1@bincrafters/stable",
        "ctti/0.0.2@Manu343726/stable",
        "gsl_microsoft/2.0.0@bincrafters/stable",
        "concurrentqueue/1.0.1",
    )

    def build(self):
        pass
        #cmake = CMake(self)
        #cmake.configure()
        #cmake.build()

    def package(self):
        self.copy("*.h"  , dst="include", src="inc", keep_path=True)
        self.copy("*.a"  , dst="lib", src="", keep_path=False)
        self.copy("*.so" , dst="lib", src="", keep_path=False)
        self.copy("*.lib", dst="lib", src="", keep_path=False)
        self.copy("*.pdb", dst="lib", src="", keep_path=False)
        self.copy("*.dll", dst="bin", src="", keep_path=False)
        self.copy("license*", dst="licenses",  ignore_case=True, keep_path=False)
        
    def package_info(self):
        pass
        #self.cpp_info.libs = ["lime"]
