./clean.sh
echo 'Building using CMake...'
cmake -Dbasic_logging=ON -Dextensive_logging=OFF -Dprint_config=ON -DCMAKE_BUILD_TYPE=Debug -S . -B build/cmake_build
cmake --build build/cmake_build
cmake --install build/cmake_build
echo 'Generating image...'
./gen.sh