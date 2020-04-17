/* Copyright 2020 the SumatraPDF project authors (see AUTHORS file).
   License: Simplified BSD (see COPYING.BSD) */

// note: include BaseUtil.h instead of including directly

namespace geomutil {

template <typename T>
struct PointT {
    T x = 0;
    T y = 0;

    PointT() : x(0), y(0) {
    }
    PointT(T x, T y) : x(x), y(y) {
    }

    template <typename S>
    PointT<S> Convert() const {
        return PointT<S>((S)x, (S)y);
    }

    PointT<int> ToInt() const {
        return PointT<int>((int)floor(x + 0.5), (int)floor(y + 0.5));
    }

    // TODO: rename to IsEmpty()
    bool empty() const {
        return x == 0 && y == 0;
    }

    bool operator==(const PointT<T>& other) const {
        return this->x == other.x && this->y == other.y;
    }
    bool operator!=(const PointT<T>& other) const {
        return !this->operator==(other);
    }
};

template <typename T>
struct SizeT {
    T dx = 0;
    T dy = 0;

    SizeT() : dx(0), dy(0) {
    }
    SizeT(T dx, T dy) : dx(dx), dy(dy) {
    }

    template <typename S>
    SizeT<S> Convert() const {
        return SizeT<S>((S)dx, (S)dy);
    }

    SizeT<int> ToInt() const {
        return SizeT<int>((int)floor(dx + 0.5), (int)floor(dy + 0.5));
    }

    bool IsEmpty() const {
        return dx == 0 || dy == 0;
    }

    // TODO: temporary
    bool empty() const {
        return dx == 0 || dy == 0;
    }

    bool operator==(const SizeT<T>& other) const {
        return this->dx == other.dx && this->dy == other.dy;
    }
    bool operator!=(const SizeT<T>& other) const {
        return !this->operator==(other);
    }
};

template <typename T>
struct RectT {
    PointT<T> min{};
    PointT<T> max{};

    RectT() = default;

    RectT(T x, T y, T dx, T dy) {
        min.x = x;
        min.y = y;
        max.x = x + dx;
        max.y = y + dy;
    }

    RectT(PointT<T> pt, SizeT<T> size) : x(pt.x), y(pt.y), dx(size.dx), dy(size.dy) {
    }

    RectT(const RectT<T>& r) {
        min.x = r.min.x;
        min.y = r.min.y;
        max.x = r.max.x;
        max.y = r.max.y;
    }

    T X() const {
        return min.x;
    }

    T Y() const {
        return min.y;
    }

    T Width() const {
        return max.x - min.x;
    }

    T Height() const {
        return max.y - min.y;
    }

    T Dx() const {
        return max.x - min.x;
    }

    T Dy() const {
        return max.y - min.y;
    }

    void SetDx(T dx) {
        max.x = min.x + dx;
    }

    void SetDy(T dy) {
        max.y = min.y + dy;
    }

    static RectT FromXY(T xs, T ys, T xe, T ye) {
        T dx = xe - xs;
        if (dx < 0) {
            xs = xe;
            dx = -dx;
        }
        T dy = ye - ys;
        if (dy < 0) {
            ys = ye;
            dy = -dy;
        }
        return RectT(xs, ys, dx, dy);
    }
    static RectT FromXY(PointT<T> TL, PointT<T> BR) {
        return FromXY(TL.x, TL.y, BR.x, BR.y);
    }

    template <typename S>
    RectT<S> Convert() const {
        return RectT<S>((S)x, (S)y, (S)dx, (S)dy);
    }

    RectT<int> ToInt() const {
        return RectT<int>((int)floor(X() + 0.5), (int)floor(Y() + 0.5), (int)floor(Dx() + 0.5), (int)floor(Dy() + 0.5));
    }
    // cf. fz_roundrect in mupdf/fitz/base_geometry.c
#ifndef FLT_EPSILON
#define FLT_EPSILON 1.192092896e-07f
#endif
    RectT<int> Round() const {
        return RectT<int>::FromXY((int)floor(X() + FLT_EPSILON), (int)floor(Y() + FLT_EPSILON),
                                  (int)ceil(X() + Dx() - FLT_EPSILON), (int)ceil(Y() + Dy() - FLT_EPSILON));
    }

    bool IsEmpty() const {
        return Dx() == 0 || Dy() == 0;
    }
    bool empty() const {
        return Dx() == 0 || Dy() == 0;
    }

    bool Contains(PointT<T> pt) const {
        T x = pt.x; T y = pt.y;
        if (x < this->X())
            return false;
        if (x > this->X() + this->Dx())
            return false;
        if (y < this->Y())
            return false;
        if (y > this->Y() + this->Dy())
            return false;
        return true;
    }

    /* Returns an empty rectangle if there's no intersection (see IsEmpty). */
    RectT Intersect(RectT other) const {
        /* The intersection starts with the larger of the start coordinates
           and ends with the smaller of the end coordinates */
        T _x = std::max(this->X(), other.X());
        T _y = std::max(this->Y(), other.Y());
        T _dx = std::min(this->X() + this->Dx(), other.X() + other.Dx()) - _x;
        T _dy = std::min(this->Y() + this->Dy(), other.Y() + other.Dy()) - _y;

        /* return an empty rectangle if the dimensions aren't positive */
        if (_dx <= 0 || _dy <= 0)
            return RectT();
        return RectT(_x, _y, _dx, _dy);
    }

    RectT Union(RectT other) const {
        if (this->Dx() <= 0 && this->Dy() <= 0)
            return other;
        if (other.Dx() <= 0 && other.Dy() <= 0)
            return *this;

        /* The union starts with the smaller of the start coordinates
           and ends with the larger of the end coordinates */
        T _x = std::min(this->X(), other.x());
        T _y = std::min(this->Y(), other.Y());
        T _dx = std::max(this->X() + this->Dx(), other.x() + other.Dx()) - _x;
        T _dy = std::max(this->Y() + this->Dy(), other.Y() + other.Dy()) - _y;

        return RectT(_x, _y, _dx, _dy);
    }

    void Offset(T _x, T _y) {
        min.x += _x;
        min.y += _y;
    }

    void Inflate(T _x, T _y) {
        min.x -= _x;
        max.x += _x;
        min.y -= _y;
        max.y += _y;
    }

    PointT<T> TL() const {
        return PointT<T>(x, y);
    }
    PointT<T> BR() const {
        return PointT<T>(x + dx, y + dy);
    }
    SizeT<T> Size() const {
        return SizeT<T>(dx, dy);
    }

#ifdef _WIN32
    RECT ToRECT() const {
        RectT<int> rectI(this->ToInt());
        RECT result = {rectI.X(), rectI.Y(), rectI.X() + rectI.Dx(), rectI.Y() + rectI.Dy()};
        return result;
    }
    static RectT FromRECT(const RECT& rect) {
        return FromXY(rect.left, rect.top, rect.right, rect.bottom);
    }

#if 1 // def GDIPVER, note: GDIPVER not defined in mingw?
    Gdiplus::Rect ToGdipRect() const {
        RectT<int> rect(this->ToInt());
        return Gdiplus::Rect(rect.X(), rect.Y(), rect.Dx(), rect.Dy());
    }
    Gdiplus::RectF ToGdipRectF() const {
        RectT<float> rectF(this->Convert<float>());
        return Gdiplus::RectF(rectF.X(), rectF.Y(), rectF.Dx(), rectF.Dy());
    }
#endif
#endif

    bool operator==(const RectT<T>& other) const {
        return this->X() == other.X() && this->Y() == other.Y() && this->Dx() == other.Dx() && this->Dy() == other.Dy();
    }
    bool operator!=(const RectT<T>& other) const {
        return !this->operator==(other);
    }
};

} // namespace geomutil

typedef geomutil::SizeT<int> SizeI;
typedef geomutil::SizeT<double> SizeD;

typedef geomutil::PointT<int> PointI;
typedef geomutil::PointT<double> PointD;

typedef geomutil::RectT<int> RectI;
typedef geomutil::RectT<double> RectD;

inline SIZE ToSIZE(SizeI s) {
    return {s.dx, s.dy};
}
#ifdef _WIN32

inline RectI ClientRect(HWND hwnd) {
    RECT rc{};
    GetClientRect(hwnd, &rc);
    return RectI::FromRECT(rc);
}

inline RectI WindowRect(HWND hwnd) {
    RECT rc{};
    GetWindowRect(hwnd, &rc);
    return RectI::FromRECT(rc);
}

inline RectI MapRectToWindow(RectI rect, HWND hwndFrom, HWND hwndTo) {
    RECT rc = rect.ToRECT();
    MapWindowPoints(hwndFrom, hwndTo, (LPPOINT)&rc, 2);
    return RectI::FromRECT(rc);
}

#endif
