// Dear ImGui: standalone example application for SDL2 + OpenGL
// (SDL is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

// **DO NOT USE THIS CODE IF YOUR CODE/ENGINE IS USING MODERN OPENGL (SHADERS, VBO, VAO, etc.)**
// **Prefer using the code in the example_sdl_opengl3/ folder**
// See imgui_impl_sdl.cpp for details.

#include "imgui.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_opengl2.h"
#include <stdio.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#include <string>
#include <vector>

typedef enum Page {
    LESSON_SELECTION,
    FLASHCARD_SELECTION,
    SHOW_FLASHCARD,
    REVEAL_FLASHCARD,
    SHOW_RESULTS
} Page;

typedef enum FlashcardField {
    ENGLISH = 1,
    CHINESE = 2,
    PINYIN = 4
} FlashcardField;

typedef struct Flashcard {
    std::string english;
    std::string chinese;
    std::string pinyin;
    bool correct = false;
} Flashcard;

Flashcard testcard1 = {
    "english 1",
    "chinese 1",
    "pinyin 1"
};
Flashcard testcard2 = {
    "english 2",
    "chinese 2",
    "pinyin 2"
};

SDL_Window* window;
SDL_GLContext gl_context;
ImGuiIO* io;

Page currentPage = LESSON_SELECTION;
int fields = 0;
std::vector<Flashcard> flashcards = {testcard1, testcard2};
int currentCard = 0;

int setup() {
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }

    // Setup window
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);

    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);
    window = SDL_CreateWindow("Chinese Flashcards", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 500, 350, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL2_Init();

    return 0;
}

void cleanup() {
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

void showLessonSelection() {
    static bool selectableLessons[18];
    if (ImGui::BeginTable("split", 3))
    {
        for (int i = 1; i <= 18; i++) {
            char lessonText[10];
            sprintf(lessonText, "Lesson %d", i);
            ImGui::TableNextColumn(); ImGui::Checkbox(lessonText, selectableLessons + (i-1));
        }
        ImGui::EndTable();
    }
    if(ImGui::Button("Next")) {
        bool anythingSelected = false;
        for (int i = 1; i <= 18; i++) {
            anythingSelected |= selectableLessons[i-1];
        }
        if (anythingSelected) {
            currentPage = FLASHCARD_SELECTION;
        }
    }
}

void showFlashcardSelection() {
    static bool en = false;
    static bool cn = false;
    static bool py = false;
    if(ImGui::Button("Return to menu")) {
        currentPage = LESSON_SELECTION;
    }
    ImGui::Checkbox("English", &en);
    ImGui::Checkbox("Chinese", &cn);
    ImGui::Checkbox("Pinyin", &py);
    if(ImGui::Button("Next")) {
        fields  = 0;
        if (en || cn || py) {
            // user must select something to advance

            if (en) fields |= ENGLISH;
            if (cn) fields |= CHINESE;
            if (py) fields |= PINYIN;
            currentPage = SHOW_FLASHCARD;
            currentCard = 0;
        }
    }
}

void showFlashcard(){
    if(ImGui::Button("Return to menu")) {
        currentPage = LESSON_SELECTION;
    }
    Flashcard &card = flashcards[currentCard];
    if (fields & ENGLISH) ImGui::Text("%s", card.english.c_str());
    if (fields & CHINESE) ImGui::Text("%s", card.chinese.c_str());
    if (fields & PINYIN) ImGui::Text("%s", card.pinyin.c_str());
    if (ImGui::BeginTable("split", 3)) {
        ImGui::TableNextColumn(); if (ImGui::Button("Previous")) {
            if (currentCard > 0) {
                currentCard--;
            }
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Flip")) {
            currentPage = REVEAL_FLASHCARD;
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Next")) {
            card.correct = true;
            if (currentCard == flashcards.size() - 1) {
                currentPage = SHOW_RESULTS;
            } else {
                currentCard++;
            }
        }
        ImGui::EndTable();
    }
}

void revealFlashcard() {
    if(ImGui::Button("Return to menu")) {
        currentPage = LESSON_SELECTION;
    }
    Flashcard &card = flashcards[currentCard];
    ImGui::Text("%s", card.english.c_str());
    ImGui::Text("%s", card.chinese.c_str());
    ImGui::Text("%s", card.pinyin.c_str());
    if (ImGui::BeginTable("split", 2)) {
        ImGui::TableNextColumn(); if(ImGui::Button("Incorrect")){
            card.correct = true;
            if (currentCard == flashcards.size() - 1) {
                currentPage = SHOW_RESULTS;
            } else {
                currentPage = SHOW_FLASHCARD;
                currentCard++;
            }
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Correct")){
            card.correct = true;
            if (currentCard == flashcards.size() - 1) {
                currentPage = SHOW_RESULTS;
            } else {
                currentPage = SHOW_FLASHCARD;
                currentCard++;
            }
        }
        ImGui::EndTable();
    }
}

void showResults() {
    int numCorrect = 0;
    for (auto& card : flashcards) {
        numCorrect += card.correct;
    }
    ImGui::Text("%d/%lu correct", numCorrect, flashcards.size());
    if (ImGui::BeginTable("split", 2)) {
        ImGui::TableNextColumn(); if(ImGui::Button("Restart lesson")) {
            currentCard = 0;
            currentPage = SHOW_FLASHCARD;
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Back to menu")) {
            currentPage = LESSON_SELECTION;
        };
        ImGui::EndTable();
    }
}

// Main code
int main(int, char**)
{
    if (setup() != 0) {
        return -1;
    }


    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool open = true;
    while (open)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                open = false;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                open = false;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGuiWindowFlags window_flags =
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoNav;

            ImGui::Begin("Hello, world!", nullptr, window_flags);     // Create a window called "Hello, world!" and append into it.

            switch (currentPage) {
            case LESSON_SELECTION:
                showLessonSelection();
                break;
            case FLASHCARD_SELECTION:
                showFlashcardSelection();
                break;
            case SHOW_FLASHCARD:
                showFlashcard();
                break;
            case REVEAL_FLASHCARD:
                revealFlashcard();
                break;
            case SHOW_RESULTS:
                showResults();
                break;
            }

            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, io->DisplaySize.x, io->DisplaySize.y);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    cleanup();

    return 0;
}