#pragma once

#include <cstdint>

#pragma pack(push, 4)

namespace Corsairs::Util {

struct Point {
    void Move(std::int16_t dir, std::int32_t distance);

    std::int32_t X{};
    std::int32_t Y{};
};

struct Square {
    Point Centre;
    std::int32_t Radius{};
};

struct Circle {
    Point Centre;
    std::int32_t Radius{};
};

struct Rect {
    Point LeftTop;
    Point RightBottom;

    std::int32_t Width() const {
        return RightBottom.X - LeftTop.X;
    }

    std::int32_t Height() const {
        return RightBottom.Y - LeftTop.Y;
    }
};

void Eddy(std::int32_t x, std::int32_t y, std::int16_t dir, std::int32_t radius, std::int32_t& retx, std::int32_t& rety);
std::int16_t Arctan(const Point& src, const Point& dest);

bool Intersects(const Square& s1, const Square& s2);
bool Intersects(const Circle& c1, const Circle& c2);
bool Intersects(const Rect& r1, const Rect& r2);

inline bool operator==(const Point& p1, const Point& p2) {
    return (p1.X == p2.X) && (p1.Y == p2.Y);
}

inline bool operator==(const Square& s1, const Square& s2) {
    return (s1.Centre == s2.Centre) && (s1.Radius == s2.Radius);
}

inline bool operator==(const Circle& c1, const Circle& c2) {
    return (c1.Centre == c2.Centre) && (c1.Radius == c2.Radius);
}

inline bool operator==(const Rect& r1, const Rect& r2) {
    return (r1.LeftTop == r2.LeftTop) && (r1.RightBottom == r2.RightBottom);
}

} // namespace Corsairs::Util

#pragma pack(pop)
