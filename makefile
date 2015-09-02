OS = $(shell uname)
MACC = clang++
LINUXC = clang++
CPPFLAGS = -std=c++0x
LIBRARIES = -Iinclude -Llib -lsdd
EXEC = sddtab
SRC = src/sddtab.cpp src/kf/KFormula.cpp src/kf/compiler.cpp src/options.cpp src/provers/proverFactory.cpp src/provers/kProver.cpp src/provers/s4Prover.cpp src/provers/prover.cpp
all:compile

ifeq ($(OS), Darwin)
compile: src/sddtab.cpp
	$(shell cp ./lib/libsdd_mac.a ./lib/libsdd.a)
	$(MACC) -O2 -stdlib=libc++ $(CPPFLAGS) $(LIBRARIES) $(SRC) -o bin/$(EXEC)
endif
ifeq ($(OS), Linux)
compile: src/sddtab.cpp
	$(LINUXC) -O2 $(CPPFLAGS) $(LIBRARIES) $(SRC) -o bin/$(EXEC)
endif
