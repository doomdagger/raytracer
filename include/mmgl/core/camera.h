//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
//

#ifndef RAYTRACER_CAMERA_H
#define RAYTRACER_CAMERA_H


#include <iostream>
#include <vector>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <memory>

#include "mmgl/util/point.h"
#include "mmgl/util/vector.h"
#include "mmgl/surface/ray.h"
#include "mmgl/surface/surface.h"
#include "mmgl/light/light.h"
#include "mmgl/light/pointlight.h"
#include "mmgl/light/ambientlight.h"
#include "mmgl/surface/triangle.h"
#include "mmgl/util/common.h"
#include "mmgl/surface/bvh_node.h"
#include "mmgl/light/arealight.h"
#include "mmgl/util/scene_config.h"
#include "mmgl/util/image.h"

namespace mmgl {

class Camera {
public:
    friend class Scene;

    void config(float x, float y, float z, float d, float dx, float dy, float dz,
                int nx, int ny, float iw, float ih);

    Ray project_pixel(float i, float j);

    void render(const std::vector<std::unique_ptr<Surface>> &objects, const std::vector<std::unique_ptr<Light>> &lights,
                const std::unique_ptr<BVHNode> &parent, const SceneConfig &sceneConfig);

    void render_partition(const size_t partition_id, const size_t partition_size,
                          const std::vector<std::unique_ptr<Surface>> &objects, const std::vector<std::unique_ptr<Light>> &lights,
                          const std::unique_ptr<BVHNode> &parent, const SceneConfig &sceneConfig);

    // add this function for multithreading
    Vector render_pixel(int x, int y, const std::vector<std::unique_ptr<Surface>> &objects, const std::vector<std::unique_ptr<Light>> &lights,
                        const std::unique_ptr<BVHNode> &parent, const SceneConfig &sceneConfig,
                        const std::function<float()> &rand_float);

    Vector L(Ray &ray, int recursive_limit, const uintptr_t object_id,
             const std::vector<std::unique_ptr<Surface>> &objects, const std::vector<std::unique_ptr<Light>> &lights,
             const std::unique_ptr<BVHNode> &parent, const Render &flag, int s_sampling_nu,
             const std::function<float()> &rand_float);

    std::pair<bool, Vector> blinn_phong(const Ray &pri_ray, const Point &light_pt, const Vector &light_cl,
                                        const Intersection &intersection,
                                        const Material &material, const std::vector<std::unique_ptr<Surface>> &objects,
                                        const std::unique_ptr<BVHNode> &parent,
                                        const Render &flag);

    void writeRgba(const std::string &) const;

    int width() const;

    int height() const;

    inline const Image &image() const {
        return _image;
    }

    friend std::ostream &operator<<(std::ostream &os, const Camera &camera);

private:
    Camera(const Point &eye = Point(), float d = 0,
           const Vector &u = Vector(), const Vector &v = Vector(), const Vector &w = Vector(),
           int nx = 0, int ny = 0, float l = 0, float r = 0, float t = 0, float b = 0);

    Point _eye;
    float _d;
    Vector _u, _v, _w;  // both normalized
    int _nx;
    int _ny;
    float _l, _r, _t, _b;
    Image _image;
};

}

#endif //RAYTRACER_CAMERA_H
