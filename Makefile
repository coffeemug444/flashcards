BACKENDS_DIR=backends
BACKENDS_ODIR=backends_obj
_BACKENDS_FILES = imgui_impl_sdl imgui_impl_opengl2
BACKENDS_FILES = $(patsubst %,$(BACKENDS_DIR)/%.cpp,$(_BACKENDS_FILES))
BACKENDS_OBJ = $(patsubst %,$(BACKENDS_ODIR)/%.o,$(_BACKENDS_FILES))

IMGUI_DIR=imgui
IMGUI_ODIR=imgui_obj
_IMGUI_FILES = imgui imgui_demo imgui_draw imgui_tables imgui_widgets
IMGUI_FILES = $(patsubst %,$(IMGUI_DIR)/%.cpp,$(_IMGUI_FILES))
IMGUI_OBJ = $(patsubst %,$(IMGUI_ODIR)/%.o,$(_IMGUI_FILES))

main: main.cpp $(IMGUI_OBJ) $(BACKENDS_OBJ)
	c++ `sdl2-config --cflags` -o $@ $^ `sdl2-config --libs` -lGL -I$(IMGUI_DIR) -I$(BACKENDS_DIR)

$(IMGUI_ODIR)/%.o: $(IMGUI_DIR)/%.cpp
	mkdir -p $(IMGUI_ODIR)
	c++ -c -o $@ $<

$(BACKENDS_ODIR)/%.o: $(BACKENDS_DIR)/%.cpp
	mkdir -p $(BACKENDS_ODIR)
	c++ -c -o $@ $< -I$(IMGUI_DIR)

.PHONY: clean
clean:
	rm -rf main $(IMGUI_ODIR) $(BACKENDS_ODIR)
