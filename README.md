# haxxerz_chat
Simple decentralized socket chat with encryption

## Dependencies

1. g++/clang with c++17 support
1. boost-system 1.71
1. SDL2
1. GLEW
1. OpenGL 3.0
1. CMake 3.16 or something

Installing them from apt on ubuntu-based distros would look like that:
```bash
sudo apt install -y cmake libsdl2-dev libboost-system1.71-dev libglew-dev libgl-dev 
```

## Build

~~~bash
  git submodule update --init
  mkdir build && cd build
  cmake ..
  make -j4
~~~

## Usage

**Server:**

~~~bash
  server [port(optional, default=1337)]
~~~

**Client:**

~~~bash
  client [ip] [port] # use ip 0.0.0.0 for local testing
~~~
