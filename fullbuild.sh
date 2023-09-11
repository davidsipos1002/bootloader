./clean.sh
echo 'Building using CMake...'
cmake -Dbasic_logging=ON -Dextensive_logging=ON -Dprint_config=ON -DCMAKE_BUILD_TYPE=Debug -S . -B cmake_build
cmake --build cmake_build
cmake --install cmake_build
echo 'Generating image...'
./gen.sh