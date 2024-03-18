# beaglebone-node
## Build
### Release ###
```
mkdir ./build
cd ./build
cmake ..
make all
```
### Debug ###
```
mkdir ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make all
```
## Build tests
### Build ###
```
mkdir ./build
cd ./build
cmake -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON ..
make tests
```
### Run ###
```
make test
```