INCLUDES=backends
IMGUI_DIR=imgui
IMGUI_ODIR=imgui_obj
_IMGUI_FILES = imgui imgui_demo imgui_draw imgui_tables imgui_widgets
IMGUI_FILES = $(patsubst %,$(IMGUI_DIR)/%.cpp,$(_IMGUI_FILES))
IMGUI_OBJ = $(patsubst %,$(IMGUI_ODIR)/%.o,$(_IMGUI_FILES))

main: main.cpp $(IMGUI_OBJ)
	mkdir -p $(IMGUI_ODIR)
	c++ `sdl2-config --cflags` -o $@ $^ -I$(INCLUDES) -I$(IMGUI_DIR) $(INCLUDES)/imgui_impl_sdl.cpp $(INCLUDES)/imgui_impl_opengl2.cpp `sdl2-config --libs` -lGL

$(IMGUI_ODIR)/%.o: $(IMGUI_DIR)/%.cpp
	c++ -c -o $@ $< -I$(INCLUDES) -I$(IMGUI_DIR)



.PHONY: clean
clean:
	rm -f main
