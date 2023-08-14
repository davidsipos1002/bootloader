./clean.sh
echo 'Building using CMake...'
cmake -S . -B cmake_build
cmake --build cmake_build
cmake --install cmake_build
echo 'Generating image...'
./gen.sh