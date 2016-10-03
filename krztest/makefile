CFLAGS = -std=c++14 -I /usr/local/include 
LFLAGS = -L/usr/local/lib -laudiofile -lportaudio -lstdc++ -lm -lglfw3 -ldrawtext -framework OpenGL 
BUILDDIR = obj
CXX = clang++

OBJS := $(patsubst src/%.cpp,$(BUILDDIR)/%.o,$(wildcard src/*.cpp))

$(BUILDDIR)/krztest:	$(OBJS)
	$(CXX) $(OBJS) $(CFLAGS) $(LFLAGS) -g -o $(BUILDDIR)/krztest 

HDRS := src/krztest.h src/krzdata.h src/objectdb.h src/krztypes.h src/alg.h src/layer.h src/filters.h src/controller.h

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
