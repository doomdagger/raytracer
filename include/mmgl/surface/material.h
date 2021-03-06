//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
//

#ifndef RAYTRACER_MATERIAL_H
#define RAYTRACER_MATERIAL_H

#include "mmgl/util/vector.h"

namespace mmgl {

/**
 * Class for materials used by surfaces.
 * Public member functions have obvious meanings.
 */
class Material {
public:

    Material(float dr = 0.7f, float dg = 0.7f, float db = 0.7f,
             float sr = 0, float sg = 0, float sb = 0,
             float ir = 0, float ig = 0, float ib = 0,
             float r = 0) : _kd{dr, dg, db}, _ks{sr, sg, sb}, _ki{ir, ig, ib},
                            _r(r) { }


    inline const Vector &kd() const {
        return _kd;
    }

    inline const Vector &ks() const {
        return _ks;
    }

    inline const Vector &ki() const {
        return _ki;
    }

    inline bool isReflective() const {
        return _ki.x() || _ki.y() || _ki.z();
    }

    inline float r() const {
        return _r;
    }

private:
    Vector _kd;
    Vector _ks;
    Vector _ki;
    float _r;
};

}

#endif //RAYTRACER_MATERIAL_H
