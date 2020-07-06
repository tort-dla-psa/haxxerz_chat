# haxxerz_chat
Simple decentralized socket chat with encryption

## Dependencies

1. g++/clang with c++17 support
1. boost 1.65

## Build

~~~bash
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
