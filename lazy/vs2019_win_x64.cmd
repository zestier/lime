pushd ..
conan config install https://github.com/zstiers/conan-config.git
mkdir .build
pushd .build
mkdir win64
pushd win64
	conan install ../.. -g cmake_multi -s build_type=Debug -e CONAN_IMPORT_PATH=Debug --build=outdated --profile windows-msvc16-amd64
	conan install ../.. -g cmake_multi -s build_type=Release -e CONAN_IMPORT_PATH=Release --build=outdated --profile windows-msvc16-amd64

	conan install ../.. -g cmake_multi -s build_type=MinSizeRel -e CONAN_IMPORT_PATH=MinSizeRel --build=outdated --profile windows-msvc16-amd64
	conan install ../.. -g cmake_multi -s build_type=RelWithDebInfo -e CONAN_IMPORT_PATH=RelWithDebInfo --build=outdated --profile windows-msvc16-amd64
	cmake ../.. -G "Visual Studio 16 2019" -A x64
popd
popd
popd