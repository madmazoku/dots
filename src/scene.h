#pragma once

#include <cmath>

#include <vector>

#include <utility>

inline double sgn(double d, double s = 1.0) {
    return d < 0 ? -s : d > 0 ? s : 0.0;
}

inline double pown(double d, size_t n) {
    if(n == 0)
        return 1.0;
    else if(n == 1)
        return d;
    else if(n == 2)
        return d*d;
    else if(n&0x1)
        return d * pown(d, n-1);
    else {
        double p = pown(d, n >> 1);
        return p * p;
    }
}

class Vector {
public:
    double _x;
    double _y;

    Vector() : _x(0), _y(0) {};
    Vector(double x, double y) : _x(x), _y(y) {}
    Vector(const Vector& v) {
        _x = v._x;
        _y = v._y;
    }

    Vector abs() const {
        return Vector(std::fabs(_x), std::fabs(_y));
    }

    double len() const {
        return std::hypot(_x, _y);
    }

    Vector normal() const {
        double l = len();
        return l > 0 ? Vector(_x / l, _y / l) : Vector();
    }

    Vector operator-() const {
        return Vector(- _x, - _y);
    }

    Vector operator*(double d) const {
        return Vector(_x * d, _y * d);
    }

    Vector& operator*=(double d) {
        _x *= d;
        _y *= d;
        return *this;
    }

    Vector operator*(const Vector& v) const {
        return Vector(_x * v._x, _y * v._y);
    }

    Vector& operator*=(const Vector& v) {
        _x *= v._x;
        _y *= v._y;
        return *this;
    }

    Vector operator/(double d) const {
        return Vector(_x / d, _y / d);
    }

    Vector operator+(const Vector& v) const {
        return Vector(_x + v._x, _y + v._y);
    }

    Vector& operator+=(const Vector& v) {
        _x += v._x;
        _y += v._y;
        return *this;
    }

    Vector operator-(const Vector& v) const {
        return Vector(_x - v._x, _y - v._y);
    }

    Vector& operator-=(const Vector& v) {
        _x -= v._x;
        _y -= v._y;
        return *this;
    }

};


class Ceil {
private:
    static inline double tor_cap(double x, double min, double max, double sz) {
        // return x;
        while(x < min)
            return x + sz;
        while(x > max)
            return x - sz;
        // return std::fmod(x - min, sz) + min;
    }

public:
    Vector _min;
    Vector _max;
    Vector _sz;

    Ceil(const Vector& min, const Vector& max) : _min(min), _max(max), _sz(_max - _min) { }

    Vector tor_cap(const Vector& v) {
        return Vector(
            tor_cap(v._x, _min._x, _max._x, _sz._x),
            tor_cap(v._y, _min._y, _max._y, _sz._y)
        );
    }

    double dist(const Vector& v1, const Vector& v2) {
        Vector v(v2 - v1);
        return std::hypot(v._x, v._y);
        // double x1 = std::fabs(v2._x - v1._x);
        // double y1 = std::fabs(v2._y - v1._y);
        // double x2 = std::fabs(x2 - _sz._x);
        // double y2 = std::fabs(y2 - _sz._y);
        // return std::hypot(std::min(x1, x2), std::min(y1, y2));
    }
};

class Dot {
public:
    Vector _p;
    Vector _v;
    Vector _a;

    Dot() = default;
    Dot(const Vector& p, const Vector& v) : _p(p), _v(v) { }
};

class Dots {
public:
    typedef std::vector<Dot> DotsCollection;

    Ceil _ceil;
    DotsCollection _dots;
    DotsCollection _dots_prev;

    uint16_t _width;
    uint16_t _height;

    Dots(uint16_t width, uint16_t height, uint16_t n = 0) : _width(width), _height(height), _ceil(Vector(0, 0), Vector(1.0, 1.0)) {
        if(n > 0)
            fill_random(n);
    }

    void fill_random(uint16_t n) {
        _dots.clear();
        _dots_prev.clear();

        _dots.resize(n);
        _dots_prev.resize(n);

        std::random_device rd;
        std::default_random_engine re(rd());
        std::uniform_real_distribution<double> rnd(0.0, 1.0);

        for(size_t i = 0; i < n; ++i) {
            double x = 0.8 / 20.0 * (i / 20) + 0.1;
            double y = 0.8 / 20.0 * (i % 20) + 0.1;
            _dots[i] = Dot(Vector(x, y), Vector());
        }

        // for(size_t i = 0; i < n; ++i) {
        //     Vector p(
        //         rnd(re) * _ceil._sz._x + _ceil._min._x + _ceil._sz._x,
        //         rnd(re) * _ceil._sz._y + _ceil._min._y + _ceil._sz._y
        //     );
        //     // Vector v(rnd(re) * 0.01, rnd(re) * 0.01);
        //     Vector v;
        //     _dots[i] = Dot(p, v);
        // }
    }

    double lennard_jones(double r, double D, double a) {
        if( r == 0.0 )
            return 0.0;
        double d = a / r;
        return - D * (pown(d, 5) - pown(d, 3));
    }

    double gravity(double r) {
        if( r == 0.0 )
            return 0.0;
        if(r < 0.01)
            return 0.0;
        return 1e-5 / pown(r, 2);
    }

    void step(double time_step) {
        std::swap(_dots, _dots_prev);

        for(size_t i = 0; i < _dots.size(); ++i) {
            Dot& dp = _dots_prev[i];
            Dot& d = _dots[i];
            d._p = _ceil.tor_cap(dp._p + dp._v * time_step);
            d._v = dp._v + dp._a * time_step;
            double v_len = dp._v.len();
            d._a = Vector();
            if(v_len > 0.0)
                d._a = -dp._v * 0.1;
        }
        for(size_t i = 0; i < _dots.size()-1; ++i) {
            Dot& dpi = _dots_prev[i];
            Dot& di = _dots[i];
            for(size_t j = i + 1; j < _dots.size(); ++j) {
                Dot& dpj = _dots_prev[j];
                Dot& dj = _dots[j];
                // for(int k = -1; k <= 1; ++k)
                //     for(int l = -1; l <= 1; ++l) {
                        int k = 0;
                        int l = 0;

                        Vector pj(
                            dpj._p._x + k * _ceil._sz._x,
                            dpj._p._y + l * _ceil._sz._y
                        );

                        double r = _ceil.dist(pj, dpi._p);
                        // double f = gravity(r);
                        double f = lennard_jones(r, 1, 0.01);
                        Vector a = r > 0 ? (pj - dpi._p) / r * f : Vector();
                        di._a += a;
                        dj._a -= a;
                    // }
            }
            double a_len = di._a.len();
            if(a_len > 0.01)
                di._a *= 0.01 / a_len;
        }
    }

    void render(SDL_Renderer* sdl_renderer) {
        int sx = 0.01 * _width;
        int sy = 0.01 * _height;
        // for(size_t i = 0; i < _dots.size(); ++i) {
        //     Dot& d = _dots[i];
        //     int x1 = d._p._x * _width;
        //     int y1 = d._p._y * _height;
        //     int x2 = (d._p._x + d._v._x) * _width;
        //     int y2 = (d._p._y + d._v._y) * _height;
        //     int x3 = (d._p._x + d._v._x + d._a._x) * _width;
        //     int y3 = (d._p._y + d._v._y + d._a._y) * _height;
        //     SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0x00, 0x00, SDL_ALPHA_OPAQUE);
        //     SDL_RenderDrawLine(sdl_renderer, x1, y1, x2, y2);
        //     SDL_SetRenderDrawColor(sdl_renderer, 0x00, 0xff, 0x00, SDL_ALPHA_OPAQUE);
        //     SDL_RenderDrawLine(sdl_renderer, x1, y1, x3, y3);
        // }
        for(size_t i = 0; i < _dots.size(); ++i) {
            Dot& d = _dots[i];
            int x1 = d._p._x * _width;
            int y1 = d._p._y * _height;
            SDL_Rect rc = {x1 - (sx >> 1), y1 - (sy >> 1), sx, sy };
            SDL_SetRenderDrawColor(sdl_renderer, 0xff, 0xff, 0xff, SDL_ALPHA_OPAQUE);
            SDL_RenderDrawPoint(sdl_renderer, x1, y1);
            SDL_RenderDrawRect(sdl_renderer, &rc);
        }
    }
};