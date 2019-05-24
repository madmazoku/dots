#include "../bin/version.h"

#include <iostream>
#include <random>

#include <boost/program_options.hpp>
#include <spdlog/spdlog.h>

#include <SDL2/SDL.h>
// #include <SDL2/SDL_image.h>

#include <cmath>

#include "scene.h"

void run();

int main(int argc, char** argv)
{
    auto console = spdlog::stdout_logger_st("console");
    console->info("Wellcome!");

    boost::program_options::options_description desc("Allowed options");
    desc.add_options()
    ("help,h", "print usage message")
    ("version,v", "print version number");

    boost::program_options::variables_map vm;
    boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
    boost::program_options::notify(vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
    } else if (vm.count("version")) {
        SDL_version sdl_version_compiled;
        SDL_VERSION(&sdl_version_compiled);
        std::cout << "Build version: " << build_version() << std::endl;
        std::cout << "Boost version: " << (BOOST_VERSION / 100000) << '.' << (BOOST_VERSION / 100 % 1000) << '.' << (BOOST_VERSION % 100) << std::endl;
        std::cout << "SDL2 version: " << uint(sdl_version_compiled.major) << '.' << uint(sdl_version_compiled.minor) << '.' << uint(sdl_version_compiled.patch) << std::endl;
    } else {
        run();
    }

    console->info("Goodbye!");

    return 0;
}

void run() {
    auto console = spdlog::get("console");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        console->error("SDL_Init Error: {0}", SDL_GetError());
        throw std::runtime_error("SDL_Init");
    }

    SDL_DisplayMode display_mode;
    if (SDL_GetCurrentDisplayMode(0, &display_mode) != 0) {
        console->error("SDL_GetCurrentDisplayMode Error: {0}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL_GetCurrentDisplayMode");
    }

    console->info("Display:");
    console->info("\tformat: {0}", SDL_GetPixelFormatName(display_mode.format));
    console->info("\twidth:  {0}", display_mode.w);
    console->info("\theight: {0}", display_mode.h);

    int window_width= 512;
    int window_height = 512;

    console->info("Display:");
    console->info("\twidth:  {0}", window_width);
    console->info("\theight: {0}", window_height);

    SDL_Window *sdl_window = SDL_CreateWindow(
                          "Hellow World!",
                          SDL_WINDOWPOS_CENTERED,
                          SDL_WINDOWPOS_CENTERED,
                          window_width,
                          window_height,
                          SDL_WINDOW_SHOWN
                      );
    if (sdl_window == nullptr) {
        console->error("SDL_CreateWindow Error: {0}", SDL_GetError());
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow");
    }

    SDL_Renderer *sdl_renderer = SDL_CreateRenderer(sdl_window, -1, SDL_RENDERER_ACCELERATED);
    if (sdl_renderer == nullptr) {
        console->error("SDL_CreateRenderer Error: {0}", SDL_GetError());
        SDL_DestroyWindow(sdl_window);
        SDL_Quit();
        throw std::runtime_error("SDL_CreateRenderer");
    }

    Dots dots(window_width, window_height, 400);

    bool run = true;
    auto start_time = std::chrono::system_clock::now();
    auto last_time = start_time;
    uint64_t count = 0;
    uint64_t last_count = 0;
    double time_step = display_mode.refresh_rate < 10 ? 0.1 : 1.0 / display_mode.refresh_rate;
    while (run) {
        auto loop_start_time = std::chrono::system_clock::now();
        ++count;

        int16_t x = count % window_width;
        int16_t y = count % window_height;
        SDL_Rect rect = {x - 5, y - 5, 10, 10};

        SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        SDL_RenderClear(sdl_renderer);

        // SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
        // SDL_RenderDrawLine(sdl_renderer, x, 0, x, window_height);
        // SDL_RenderDrawLine(sdl_renderer, 0, y, window_width, y);
        // SDL_RenderFillRect(sdl_renderer, &rect);

        dots.step(time_step);
        dots.render(sdl_renderer);

        SDL_RenderPresent(sdl_renderer);

        SDL_Event sdl_event;
        while (SDL_PollEvent(&sdl_event)) {
            if (sdl_event.type == SDL_QUIT) {
                run = false;
            }
        }

        auto current_time = std::chrono::system_clock::now();
        std::chrono::duration<double> full_elapsed = current_time - start_time;
        std::chrono::duration<double> last_elapsed = current_time - last_time;
        std::chrono::duration<double> loop_elapsed = current_time - loop_start_time;
        time_step = (time_step * (count-1) + loop_elapsed.count()) / count;

        if (!run || last_elapsed.count() >= 1) {
            int frames = count - last_count;
            double fps = ((double)frames) / last_elapsed.count();

            SDL_SetWindowTitle(sdl_window, ("Hello World! FPS: " + std::to_string(fps)).c_str());

            console->info("[{0} / {1}] fps: {2}; time_step: {3}", full_elapsed.count(), count, fps, time_step);

            last_time = current_time;
            last_count = count;
        }

        // if(loop_elapsed.count() < 0.01) {
        //     SDL_Delay(100 - loop_elapsed.count() * 100);
        // }
    }

    SDL_DestroyRenderer(sdl_renderer);
    SDL_DestroyWindow(sdl_window);
    SDL_Quit();
}
