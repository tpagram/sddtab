OS = $(shell uname)
MACC = clang++
LINUXC = g++
CPPFLAGS = -std=c++0x -O2 -Wall -Iinclude
EXEC = sddtab
SRC = src/sddtab.cpp src/kf/KFormula.cpp src/kf/compiler.cpp src/options.cpp src/provers/proverFactory.cpp src/provers/kProver.cpp src/provers/s4Prover.cpp src/provers/prover.cpp
SRC_OBJ = $(SRC:.cpp=.o)

HEADERS = sddapi.h


ifeq ($(OS), Darwin)
LIBRARIES = -Llib -lsdd_mac
sddtab: src/sddtab.cpp
	$(MACC) -O2 -stdlib=libc++ $(CPPFLAGS) $(LIBRARIES) $(SRC) -o bin/$(EXEC)
endif
ifeq ($(OS), Linux)
LIBRARIES = -Llib -lsdd_linux -lm

sddtab: $(SRC_OBJ)
	$(LINUXC) $(CPPFLAGS) $(SRC_OBJ) $(LIBRARIES) -o bin/$(EXEC)

%.o: %.cpp $(HEADERS) 
	$(LINUXC) -c $< -o $@

clean:
	rm -f $(SRC_OBJ) $(EXEC)
endif
