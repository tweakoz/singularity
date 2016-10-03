CFLAGS = -std=c++14 -I /usr/local/include
LFLAGS = -L /usr/local/lib/libgig -L/usr/local/lib -lgig -lsndfile -laudiofile -lstdc++ -lm
BUILDDIR = obj
CXX = clang++

OBJS := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(wildcard src/*.cpp))

$(BUILDDIR)/krzparse:	$(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) $(LFLAGS) -g -o $(BUILDDIR)/krzparse 

$(OBJS): $(BUILDDIR)/%.o: src/%.cpp src/krzio.h
	$(CXX) $(CFLAGS) -g -c $< -o $@

test:	obj/krzparse
	./obj/krzparse krzfiles/k2v3setuprom.rom

dbg:
	lldb ./obj/krzparse -- krzfiles/k2v3setuprom.bin

prep:
	rm -rf obj
	mkdir obj

clean:
	rm -rf obj
	mkdir obj

install_deps:
	brew install tweakoz/homebrew-csound/csoundx --with-acpython --with-aclua --with-stk
	brew install tweakoz/vrxtap/libgigx
