INCLUDES=backends
IMGUI_STUFF=imgui

main: main.cpp
	c++ `sdl2-config --cflags` -o $@ $^ -I$(INCLUDES) -I$(IMGUI_STUFF) $(INCLUDES)/imgui_impl_sdl.cpp $(INCLUDES)/imgui_impl_opengl2.cpp $(IMGUI_STUFF)/*.cpp `sdl2-config --libs` -lGL

.PHONY: clean
clean:
	rm -f main