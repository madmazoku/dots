#pragma once

#include <cmath>

#include <deque>

class Dot {
public:
    double _x;
    double _y;

    Dot(double x, double y) : _x(x), _y(y) { }
    Dot(Dot&& dot) { 
        _x = dot._x;
        _y = dot._y;
    }
};

class Dots {
public:
    typedef std::deque<Dot> DotsCollection;

    DotsCollection _dots;

    uint16_t _width;
    uint16_t _height;

    Dots(uint16_t width, uint16_t height, uint16_t n = 0) : _width(width), _height(height) {
        if(n > 0)
            fill_random(n);
    }

    void fill_random(uint16_t n) {
        _dots.clear();

        std::random_device rd;
        std::default_random_engine re(rd());
        std::uniform_real_distribution<double> uniform_dist(0.0, 1.0);

        for(size_t i = 0; i < n; ++i)
            _dots.push_back(Dot(uniform_dist(re), uniform_dist(re)));
    }

    void render(SDL_Renderer* sdl_renderer) {
        SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
        for(auto &d : _dots)
            SDL_RenderDrawPoint(sdl_renderer, d._x, d._y);
    }
};