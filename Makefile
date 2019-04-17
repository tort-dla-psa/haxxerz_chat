incdir := include
libdir := lib
srcdir := src
bindir := bin
blddir := build
submodsdir := submodules

cxx := g++
cxxflags := -ffast-math
extra =

client_src:=cli.cpp
client_trg:=cli

server_src:=server.cpp
server_header:=server.h
server_main:=serv.cpp
server_trg:=serv

#IO submodule
IO := $(blddir)/IO.flag
$(IO):
	@$(MAKE) -C $(submodsdir)/UnixIO-cpp/ extra="$(extra)" all
	@cp $(submodsdir)/UnixIO-cpp/lib/* $(libdir)/libunixiocpp.so
	@touch $(@)

objects := $(patsubst $(srcdir)/%.cpp, $(blddir)/%.o, $(shell find $(srcdir) -iname "*.cpp"))

cli_obj := $(blddir)/cli.o $(blddir)/main-cli.o
cli_trg := $(bindir)/cli
cli_dep := $(IO)

srv_obj := $(blddir)/srv.o $(blddir)/main-srv.o
srv_trg := $(bindir)/srv
srv_dep := $(IO)

prepare: 
	@mkdir -p $(blddir) $(libdir) $(bindir)

$(blddir)/%.o: $(srcdir)/%.cpp
	@echo "> compile $< $@"
	@$(cxx) $(extra) $< -fPIC -I$(incdir) -I$(submodsdir)/UnixIO-cpp/include -c -o $@ -lncurses

$(cli_trg): $(cli_obj)
	@echo "> compile $< $@"
	@$(cxx) $^ -o $(cli_trg) $(extra) -I$(incdir) -I$(submodsdir)/UnixIO-cpp/include \
		-Llib -lunixiocpp -lpthread -lncurses

$(srv_trg): $(srv_obj)
	@echo "> compile $< $@"
	@$(cxx) $^ -o $(srv_trg) $(extra) -I$(incdir) -I$(submodsdir)/UnixIO-cpp/include \
		-Llib -lunixiocpp -lpthread -lncurses

all: prepare $(IO) $(cli_trg) $(srv_trg)

clean:
	@echo "> cleaning"
	@rm -rf $(bindir) $(libdir) $(blddir)
	@$(MAKE) -C $(submodsdir)/UnixIO-cpp/ clean
