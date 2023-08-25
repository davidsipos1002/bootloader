./clean.sh
echo 'Building using CMake...'
cmake -S . -B cmake_build -G "MinGW Makefiles"
cmake --build cmake_build
cmake --install cmake_build