OBJS = 	build/sddtab.o \
	build/options.o \
	build/kf/compiler.o \
	build/kf/Kformula.o \
	build/provers/prover.o \
	build/provers/kProver.o \
	build/provers/s4Prover.o \
	build/provers/proverFactory.o

OS = $(shell uname)
ifeq ($(OS), Darwin)
	CC = clang++
	LIBS = -Llib -lsdd_mac
	INCLUDES = -Iinclude
	CFLAGS = -Wall -O2 -c -g -std=c++11 -stdlib=libc++ $(INCLUDES)
	LFLAGS = -Wall -O2 -g -stdlib=libc++ $(LIBS)
endif
ifeq ($(UNAME), Linux)
	CC = g++
	LIBS = -Llib -lsdd_linux -lm
	INCLUDES = -Iinclude
	CFLAGS = -Wall -O2 -c -g -std=c++11 $(INCLUDES)
	LFLAGS = -Wall -O2 -g $(LIBS)
endif

bin/sddtab : $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(LFLAGS) $(OBJS) -o bin/sddtab

build/sddtab.o : 	src/sddtab.cpp \
			include/options.h \
			include/kf/compiler.h \
			include/provers/proverFactory.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/sddtab.cpp -o build/sddtab.o

build/options.o : 	src/options.cpp \
			include/options.h 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/options.cpp -o build/options.o

build/kf/compiler.o : 	src/kf/compiler.cpp \
			include/kf/compiler.h \
			include/kf/Kformula.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/kf/compiler.cpp -o build/kf/compiler.o

build/kf/Kformula.o : 	src/kf/Kformula.cpp \
			include/kf/kformula.h 
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/kf/kformula.cpp -o build/kf/kformula.o

build/provers/prover.o : src/provers/prover.cpp \
			include/provers/prover.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/provers/prover.cpp -o build/provers/prover.o

build/provers/kProver.o : src/provers/kProver.cpp \
			include/provers/kProver.h \
			include/provers/prover.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/provers/kProver.cpp -o build/provers/kProver.o

build/provers/s4Prover.o : src/provers/s4Prover.cpp \
			include/provers/s4Prover.h \
			include/provers/prover.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/provers/s4Prover.cpp -o build/provers/s4Prover.o

build/provers/proverFactory.o : src/provers/proverFactory.cpp \
			include/provers/proverFactory.h \
			include/provers/prover.h \
			include/provers/s4Prover.h \
			include/provers/kProver.h
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) src/provers/proverFactory.cpp -o build/provers/proverFactory.o

clean:
	$(RM) -r build bin