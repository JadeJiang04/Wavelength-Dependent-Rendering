cmake .. -G "MinGW Makefiles" `
         -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake `
         -DBUILD_CUSTOM=ON

cmake --build . --config Release -j 4

./pathtracer -t 8 -e ../exr/grace.exr -s 4 -l 64 -f bunny_microfacet_cu_importance.png ../dae/sky/bunny_microfacet_cu_unlit.dae

./pathtracer -t 8 -e ../exr/grace.exr -s 4 -l 64 -f bubble_test.png ../dae/sky/bubble_test.dae
./pathtracer -t 8 -s 4 -l 64 -f CBspheres.png ../dae/sky/CBspheres.dae

./pathtracer.exe -f output_thinfilm_room.png -t 8 -m 5 -l 4 -s 64 ../dae/sky/CBspheres_thinfilm_room.dae

./pathtracer.exe -f output_thinfilm_standard.png -t 8 -m 5 -l 64 -s 128 ../dae/sky/CBspheres_thinfilm_room.dae

./pathtracer -t 8 -m 100 -s 64 -l 128 -f quality_test.png ../dae/sky/CBspheres_refract.dae
