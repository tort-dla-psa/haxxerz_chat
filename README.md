# haxxerz_chat
Simple decentralized socket chat with encryprion

##Dependencies

1. g++
1. ncurses
1. cpypto++

##Build

~~~bash
  make all extra="-O2 -s"
~~~

##Run

**Server:**

~~~bash
  ./bin/srv [port]
~~~

**Client:**

~~~bash
  ./bin/cli [ip] [port] # use ip 0.0.0.0 for local testing
~~~
