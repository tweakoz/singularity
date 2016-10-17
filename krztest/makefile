CFLAGS = -std=c++14 -I /usr/local/include 
LFLAGS = -L/usr/local/lib -laudiofile -lportaudio -lstdc++ -lm -lglfw3 -ldrawtext -framework OpenGL 
BUILDDIR = obj
CXX = clang++

OBJS := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(wildcard src/*.cpp))

$(BUILDDIR)/krztest:	$(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) $(LFLAGS) -g -o $(BUILDDIR)/krztest 

H1 := src/krztest.h src/krzdata.h src/objectdb.h src/krztypes.h
H2 := src/layer.h src/controller.h
H3 := src/alg.h src/alg_eq.h src/filters.h src/shelveeq.h src/para.h

HDRS := $(H1) $(H2) $(H3)

$(OBJS): $(BUILDDIR)/%.o: src/%.cpp $(HDRS)
	$(CXX) $(CFLAGS) -g -c $< -o $@

test:	obj/krztest
	./obj/krztest 

prep:
	rm -rf obj
	mkdir obj
	brew install glfw3 libdrawtext portmidi portaudio

clean:
	rm -rf obj
	mkdir obj
