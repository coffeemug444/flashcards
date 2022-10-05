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
#include <filesystem>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <random>

namespace fs = std::filesystem;

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

typedef enum CardStatus {
    CORRECT,
    INCORRECT,
    UNDECIDED
} CardStatus;

typedef struct Flashcard {
    std::string english;
    std::string chinese;
    std::string pinyin;
    CardStatus status = UNDECIDED;
} Flashcard;

SDL_Window* window;
SDL_GLContext gl_context;
ImGuiIO* io;

Page currentPage = LESSON_SELECTION;
std::vector<std::vector<Flashcard>> lessons;
int fields = 0;
std::vector<Flashcard> active_set;
std::vector<Flashcard> inactive_set;
int currentCard = 0;
auto rng = std::default_random_engine{std::random_device()()};

ImFont* en_large;
ImFont* cn_large;
float large_font_size = 48.0f;

int setup() {
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to the latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
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
    window = SDL_CreateWindow("Chinese Flashcards", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 550, 230, window_flags);
    gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);
    
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = &ImGui::GetIO();
    io->Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 18.0f);
    cn_large = io->Fonts->AddFontFromFileTTF("fonts/NotoSansSC-Thin.otf", large_font_size, NULL, io->Fonts->GetGlyphRangesChineseFull());
    io->Fonts->Build();
    ImGui::StyleColorsDark();

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

void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetWindowSize().x;
    auto textWidth   = ImGui::CalcTextSize(text.c_str()).x;

    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::Text("%s", text.c_str());
}

void skipInvisibleFlashcardFields() {
    int invisibleFields = 0;
    invisibleFields += !(fields & ENGLISH);
    invisibleFields += !(fields & PINYIN);
    invisibleFields += !(fields & CHINESE);
    if (invisibleFields == 0) {
        return;
    }
    float ypos = ImGui::GetCursorScreenPos().y;
    ImGui::SetCursorPosY(ypos + invisibleFields*large_font_size);
}

void showLessonSelection() {
    static bool selectableLessons[18];
    if (ImGui::BeginTable("split", 3))
    {
        for (int i = 1; i <= lessons.size(); i++) {
            char lessonText[10];
            sprintf(lessonText, "Lesson %d", i);
            ImGui::TableNextColumn(); ImGui::Checkbox(lessonText, selectableLessons + (i-1));
        }
        ImGui::EndTable();
    }
    if(ImGui::Button("Next")) {
        active_set.clear();
        inactive_set.clear();
        for (int i = 0; i <= lessons.size(); i++) {
            if (selectableLessons[i]) {
                active_set.insert(end(active_set), begin(lessons[i]), end(lessons[i]));
            }
        }
        if (active_set.size() != 0) {
            std::shuffle(begin(active_set), end(active_set), rng);
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
    Flashcard card = active_set.front();
    ImGui::PushFont(cn_large);
    if (fields & ENGLISH) TextCentered(card.english);
    if (fields & PINYIN) TextCentered(card.pinyin);
    if (fields & CHINESE) TextCentered(card.chinese);
    ImGui::PopFont();
    skipInvisibleFlashcardFields();
    if (ImGui::BeginTable("split", 3)) {
        ImGui::TableNextColumn(); if (ImGui::Button("Previous")) {
            if (inactive_set.size() != 0) {
                Flashcard prevCard = inactive_set.back();
                if (prevCard.status = CORRECT) {
                    prevCard.status = UNDECIDED;
                }
                active_set.insert(begin(active_set), prevCard);
                inactive_set.pop_back();
            }
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Flip")) {
            currentPage = REVEAL_FLASHCARD;
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Next")) {
            if (card.status != INCORRECT) {
                card.status = CORRECT;
            }
            inactive_set.push_back(card);
            active_set.erase(begin(active_set));
            if (active_set.size() == 0) {
                currentPage = SHOW_RESULTS;
            }
        }
        ImGui::EndTable();
    }
}

void revealFlashcard() {
    if(ImGui::Button("Return to menu")) {
        currentPage = LESSON_SELECTION;
    }
    Flashcard card = active_set.front();
    ImGui::PushFont(cn_large);
    TextCentered(card.english);
    TextCentered(card.pinyin);
    TextCentered(card.chinese);
    ImGui::PopFont();
    if (ImGui::BeginTable("split", 2)) {
        ImGui::TableNextColumn(); if(ImGui::Button("Incorrect")){
            card.status = INCORRECT;
            active_set.erase(begin(active_set));
            int num_left = active_set.size();
            if (num_left <= 3) {
                active_set.push_back(card);
            } else {
                std::uniform_int_distribution<> distr(3, num_left-1);
                active_set.insert(begin(active_set) + distr(rng), card);
            }


            if (active_set.size() == 0) {
                currentPage = SHOW_RESULTS;
            } else {
                currentPage = SHOW_FLASHCARD;
            }
        }
        ImGui::TableNextColumn(); if(ImGui::Button("Correct")){
            if (card.status != INCORRECT) {
                card.status = CORRECT;
            }
            inactive_set.push_back(card);
            active_set.erase(begin(active_set));
            if (active_set.size() == 0) {
                currentPage = SHOW_RESULTS;
            } else {
                currentPage = SHOW_FLASHCARD;
            }
        }
        ImGui::EndTable();
    }
}

void showResults() {
    int numCorrect = 0;
    for (auto& card : inactive_set) {
        numCorrect += card.status == CORRECT;
    }
    ImGui::Text("%d/%lu correct", numCorrect, inactive_set.size());
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

void push_lesson(int lessonNumber) {
    std::vector<Flashcard> lesson;
    std::stringstream pathstream;
    pathstream << "lessons/lesson" << lessonNumber << ".csv";
    std::ifstream file(pathstream.str());
    std::string line;
    file.ignore(3); // seems to not like the first 3 bytes
    while (std::getline(file, line)) {
        Flashcard card;
        std::stringstream lineStream(line);
        std::string cell;
        
        std::getline(lineStream, cell, ',');
        card.english = cell;
        std::getline(lineStream, cell, ',');
        card.pinyin = cell;
        std::getline(lineStream, cell, ',');
        card.chinese = cell;
        lesson.push_back(card);
    }

    lessons.push_back(lesson);
}

// Main code
int main(int, char**)
{
    if (setup() != 0) {
        return -1;
    }

    std::string path = "lessons";
    int lessonNumber = 0;
    for (const auto & entry : fs::directory_iterator(path)) {
        push_lesson(++lessonNumber);
    }
    

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool open = true;
    while (open)
    {
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
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }

    cleanup();

    return 0;
}
