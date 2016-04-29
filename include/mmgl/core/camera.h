/**
 * Header for the Camera class.
 */

#ifndef RAYTRACER_CAMERA_H
#define RAYTRACER_CAMERA_H


#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
#include <functional>
#include <random>
#include <utility>
#include <vector>
#include <ctime>
#include <cmath>
#include <cstdlib>
#include <future>
#include <functional>

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
#include "mmgl/util/thread_pool.h"

namespace mmgl {

/**
 * Camera class.
 *
 * The Camera class defines the viewpoint, image sizes, and so on. It is used in a scene object for rendering the objects in that scene.
 */
class Camera {
public:

    /**
     * Constructer that sets all camera configurations.
     *
     * The camera uses default parameters where most values are set to 0.
     *
     * @param eye Position of the camera or the viewpoint.
     * @param
     */
    Camera(const Point &eye = Point(), float d = 0,
           const Vector &u = Vector(), const Vector &v = Vector(), const Vector &w = Vector(),
           int nx = 0, int ny = 0, float l = 0, float r = 0, float t = 0, float b = 0);

    void config(float x, float y, float z, float d, float dx, float dy, float dz,
                int nx, int ny, float iw, float ih);

    Ray project_pixel(float i, float j);

    Camera &at(float x, float y, float z);

    Camera &at(const Point &point);

    Camera &focal_length(float d);

    Camera &facing(float dx, float dy, float dz);

    Camera &facing(const Vector &dir);

    Camera &view_range(float iw, float ih);

    Camera &image_size(int nx, int ny);

    void render(const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                const BVHNode *const parent, const SceneConfig &sceneConfig);

    void writeRgba(const std::string &) const;

    inline int width() const {
        return _nx;
    }

    inline int height() const {
        return _ny;
    }

    inline const Point &eye() const {
        return _eye;
    }

    inline const Image &image() const {
        return _image;
    }

    inline const RenderResult& handle() const {
        return _image.handle();
    }

    friend std::ostream &operator<<(std::ostream &os, const Camera &camera);

private:
    void render_partition(const size_t partition_id, const size_t partition_size,
                          const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                          const BVHNode *const parent, const SceneConfig &sceneConfig);

    // add this function for multithreading
    Vector render_pixel(int x, int y, const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                        const BVHNode *const parent, const SceneConfig &sceneConfig,
                        const std::function<float()> &rand_float);

    Vector L(Ray &ray, int recursive_limit, const Surface *const object_id,
             const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
             const BVHNode *const parent, const Render &flag, int s_sampling_nu,
             const std::function<float()> &rand_float);

    std::pair<bool, Vector> blinn_phong(const Ray &pri_ray, const Point &light_pt, const Vector &light_cl,
                                        const Intersection &intersection,
                                        const Material &material, const std::vector<Surface *> &objects,
                                        const BVHNode *const parent,
                                        const Render &flag);

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
