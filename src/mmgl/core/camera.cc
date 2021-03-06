//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
//

#include "mmgl/core/camera.h"

namespace mmgl {

Ray Camera::project_pixel(float i, float j) {
    float u = _l + (_r - _l) * (i + 0.5f) / _nx;
    float v = _b + (_t - _b) * (_ny - j + 0.5f) / _ny;

    Vector dir(_u.x() * u + _v.x() * v - _w.x() * _d, _u.y() * u + _v.y() * v - _w.y() * _d,
               _u.z() * u + _v.z() * v - _w.z() * _d);
    dir.normalize();

    return std::move(Ray(_eye, dir));
}

Camera::Camera(const Point &eye, float d, const Vector &u, const Vector &v, const Vector &w, int nx, int ny,
               float l, float r, float t, float b) : _eye{eye}, _d{d}, _u{u}, _v{v}, _w{w},
                                                     _nx{nx}, _ny{ny}, _l{l}, _r{r}, _t{t}, _b{b} { }

void Camera::config(float x, float y, float z, float d, float dx, float dy, float dz,
                    int nx, int ny, float iw, float ih) {
    _eye.x(x), _eye.y(y), _eye.z(z);
    _d = d;

    Vector d_dir{dx, dy, dz};

    _u = d_dir.cross(Vector{0, 1, 0});
    _v = _u.cross(d_dir);
    _w = d_dir * -1;

    _u.normalize();
    _v.normalize();
    _w.normalize();

    _nx = nx;
    _ny = ny;

    _l = -iw / 2, _r = iw / 2, _t = ih / 2, _b = -ih / 2;

    _image.resize(nx, ny);
}

std::ostream &operator<<(std::ostream &os, const Camera &camera) {

    os << "Camera:\n";
    os << "\teye: " << camera._eye << "\n";
    os << "\td: " << camera._d << "\n";
    os << "\tu: " << camera._u << "\n";
    os << "\tv: " << camera._v << "\n";
    os << "\tw: " << camera._w << "\n";
    os << "\tnx: " << camera._nx << "\n";
    os << "\tny: " << camera._ny << "\n";
    os << "\tl: " << camera._l << "\n";
    os << "\tr: " << camera._r << "\n";
    os << "\tt: " << camera._t << "\n";
    os << "\tb: " << camera._b << std::flush;

    return os;
}

std::pair<bool, Vector> Camera::blinn_phong(const Ray &pri_ray, const Point &light_pt, const Vector &light_cl,
                                            const Intersection &intersection,
                                            const Material &material, const std::vector<Surface *> &objects,
                                            const BVHNode *const parent,
                                            const Render &flag) {
    std::pair<bool, Vector> ret(false, Vector());

    // create light ray from intersection point, shadow ray
    Vector lightRayDir = light_pt - intersection.point();
    float interMagnitude = lightRayDir.magnitude();
    lightRayDir /= interMagnitude;
    Ray shadowRay{intersection.point(), lightRayDir};

    int obj_size = static_cast<int>(objects.size());

    // render flag
    if (flag == Render::NORMAL || flag == Render::BBOX_ONLY) {
        for (int j = 0; j < obj_size; ++j) {
            if (objects[j] != intersection.id()) {
                objects[j]->intersect(shadowRay, flag);
            }
        }
    } else {
        BVHNode::intersect(shadowRay, parent, intersection.id(), flag);
    }

    if (!shadowRay.has_block(interMagnitude)) {
        ret.first = true;
        // diffuse
        float d_scalar = intersection.normal().dot(lightRayDir);
        Vector d_temp = material.kd() * light_cl;
        d_temp *= d_scalar > 0 ? d_scalar : 0;
        ret.second += d_temp;
        // specular
        float s_scalar = intersection.normal().dot(bisector(pri_ray.dir() * -1, lightRayDir));
        Vector s_temp = material.ks() * light_cl;
        s_temp *= s_scalar > 0 ? powf(s_scalar, material.r()) : 0;
        ret.second += s_temp;
    }

    return std::move(ret);
}

Vector Camera::L(Ray &ray, int recursive_limit, const Surface *const object_id,
                 const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                 const BVHNode *const parent, const Render &flag, int s_sampling_num,
                 const std::function<float()> &rand_float) {
    static float inv_s_sampling_num_pow2 = 1.0f / (s_sampling_num * s_sampling_num);
    if (recursive_limit == 0)
        return std::move(Vector{0.0f, 0.0f, 0.0f});

    // compute ray intersection with all objects
    int obj_size = static_cast<int>(objects.size());
    if (flag == Render::NORMAL || flag == Render::BBOX_ONLY) {
        // I have to write like this, more redundant code to avoid useless checks in for loop
        if (object_id) {
            for (int i = 0; i < obj_size; ++i) {
                if (objects[i] != object_id) {
                    objects[i]->intersect(ray, flag);
                }
            }
        } else {
            for (int i = 0; i < obj_size; ++i) {
                objects[i]->intersect(ray, flag);
            }
        }
    } else {
        BVHNode::intersect(ray, parent, object_id, flag);
    }

    // no intersection, return empty vector
    if (!ray.has_intersect()) {
        return std::move(Vector{0.0f, 0.0f, 0.0f});
    }
    // hold return value
    Vector rgb;
    // get intersection, object, material
    const Intersection &intersection = ray.intersection();
    const Surface &surface = *intersection.id();
    const Material &material = surface.material();
    // iterate over all lights, use iterator
    for (auto &light_ptr : lights) {
        if (PointLight *pointLight = dynamic_cast<PointLight *>(light_ptr)) {  // For point light
            // compute shading
            rgb += blinn_phong(ray, pointLight->orig(), pointLight->color(), intersection, material, objects, parent,
                               flag).second;
        } else if (AmbientLight *ambientLight = dynamic_cast<AmbientLight *>(light_ptr)) {  // for ambient light
            rgb += material.kd() * ambientLight->color();
        } else if (AreaLight *areaLight = dynamic_cast<AreaLight *>(light_ptr)) {    // for square area light
            Vector sub_rgb;
            if (s_sampling_num == 1) {
                // compute shading
                std::pair<bool, Vector> temp = blinn_phong(ray, areaLight->orig(), areaLight->color(), intersection,
                                                           material, objects, parent, flag);
                if (temp.first) {
                    // create light vector from intersection point
                    Vector lightRayDir = areaLight->orig() - intersection.point();
                    float interMagnitude = lightRayDir.magnitude();
                    lightRayDir /= interMagnitude;

                    // normalize
                    float a_scalar = areaLight->norm().dot(lightRayDir * -1) / powf(interMagnitude + 1.0f, 2);
                    sub_rgb += temp.second * (a_scalar > 0 ? a_scalar : 0);
                }
            } else {
                for (Point &sample_p : areaLight->sample(s_sampling_num, rand_float)) {
                    std::pair<bool, Vector> temp = blinn_phong(ray, sample_p, areaLight->color(), intersection,
                                                               material, objects, parent, flag);
                    if (temp.first) {
                        // create light vector from intersection point
                        Vector lightRayDir = sample_p - intersection.point();
                        float interMagnitude = lightRayDir.magnitude();
                        lightRayDir /= interMagnitude;

                        // normalize
                        float a_scalar = areaLight->norm().dot(lightRayDir * -1) / powf(interMagnitude + 1.0f, 2);
                        sub_rgb += temp.second * (a_scalar > 0 ? a_scalar : 0);
                    }
                }
                sub_rgb *= inv_s_sampling_num_pow2;
            }

            rgb += sub_rgb;
        }
    }
    // reflective material
    if (material.isReflective()) {
        // reduce intermediately generated object
        Vector refRayDir = intersection.normal() * (-2.0f * ray.dir().dot(intersection.normal()));
        refRayDir += ray.dir();
        refRayDir.normalize();
        Ray refRay{intersection.point(), refRayDir};
        // recursively compute it
        rgb += material.ki() *
               L(refRay, recursive_limit - 1, intersection.id(), objects, lights, parent, flag, s_sampling_num, rand_float);
        return std::move(rgb);
    } else {
        return std::move(rgb);
    }
}

void Camera::render(const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                    const BVHNode *const parent, const SceneConfig &sceneConfig) {
    const size_t partition_num {sceneConfig.partition_num()};
    const size_t partition_size {(_nx * _ny + partition_num - 1) / partition_num};
    thread_pool pool(sceneConfig.thread_num());

    // render each partition in parallel
    std::vector<std::future<void>> futures(partition_num);
    for (size_t i {0}; i < partition_num; ++i) {
        if (sceneConfig.parallel_method() == ParallelMethod::STD_ASYNC) {
            futures[i] = async(&Camera::render_partition, this, i, partition_size,
                               std::cref(objects), std::cref(lights), std::cref(parent), std::cref(sceneConfig));
        } else if (sceneConfig.parallel_method() == ParallelMethod::STD_ASYNC_FORCE) {
            futures[i] = async(std::launch::async, &Camera::render_partition, this, i, partition_size,
                               std::cref(objects), std::cref(lights), std::cref(parent), std::cref(sceneConfig));
        } else { /* ParallelMethod::THREAD_POOL */
            futures[i] = pool.submit(bind(&Camera::render_partition, this, i, partition_size,
                                          std::cref(objects), std::cref(lights), std::cref(parent), std::cref(sceneConfig)));
        }
    }
    for (auto &f : futures) {
        f.get();
    }
}

void Camera::render_partition(const size_t partition_id, const size_t partition_size,
                              const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                              const BVHNode *const parent, const SceneConfig &sceneConfig) {
    const int sampling_num_pow2 = std::pow(sceneConfig.pixel_sampling_num(), 2);
    long seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution<float> distribution(0.0, 1.0);
    std::function<float()> rand_float = bind(distribution, generator);

    size_t pixel_start = partition_id * partition_size;
    size_t pixel_end = std::min(pixel_start + partition_size, static_cast<size_t>(_nx) * _ny);
    for (size_t i {pixel_start}; i < pixel_end; ++i) {
        int x {static_cast<int>(i % _nx)};
        int y {static_cast<int>(i / _nx)};
        Vector rgb = render_pixel(x, y, objects, lights, parent, sceneConfig, rand_float);
        rgb /= sampling_num_pow2;
        _image.pixel(x, y, rgb);
    }
}

Vector Camera::render_pixel(int x, int y, const std::vector<Surface *> &objects, const std::vector<Light *> &lights,
                            const BVHNode *const parent, const SceneConfig &sceneConfig,
                            const std::function<float()> &rand_float) {
    Vector rgb;

    if (sceneConfig.pixel_sampling_num() == 1) {
        Ray ray = project_pixel(x, y);
        rgb += L(ray, sceneConfig.recursive_limit(), nullptr, objects, lights, parent, sceneConfig.render_flag(),
                 sceneConfig.shadow_sampling_num(), rand_float);
    } else {
        for (int p = 0; p < sceneConfig.pixel_sampling_num(); p++) {
            for (int q = 0; q < sceneConfig.pixel_sampling_num(); q++) {
                Ray sampling_ray = project_pixel(x + (p + rand_float()) / sceneConfig.pixel_sampling_num(),
                                                 y + (q + rand_float()) / sceneConfig.pixel_sampling_num());
                rgb += L(sampling_ray, sceneConfig.recursive_limit(), nullptr, objects, lights, parent,
                         sceneConfig.render_flag(), sceneConfig.shadow_sampling_num(), rand_float);
            }
        }
    }

    return std::move(rgb);
}

void Camera::writeRgba(const std::string &fileName) const {
    _image.save(fileName);
}

Camera &Camera::at(float x, float y, float z) {
    _eye.x(x), _eye.y(y), _eye.z(z);
    return *this;
}

Camera &Camera::focal_length(float d) {
    _d = d;
    return *this;
}

Camera &Camera::facing(float dx, float dy, float dz) {
    Vector d_dir{dx, dy, dz};

    _u = d_dir.cross(Vector{0, 1, 0});
    _v = _u.cross(d_dir);
    _w = d_dir * -1;

    _u.normalize();
    _v.normalize();
    _w.normalize();

    return *this;
}

Camera &Camera::view_range(float iw, float ih) {
    _l = -iw / 2, _r = iw / 2, _t = ih / 2, _b = -ih / 2;

    return *this;
}

Camera &Camera::image_size(int nx, int ny) {
    _nx = nx;
    _ny = ny;

    _image.resize(nx, ny);
    return *this;
}

Camera &Camera::at(const Point &point) {
    _eye.x(point.x()), _eye.y(point.y()), _eye.z(point.z());
    return *this;
}

Camera &Camera::facing(const Vector &d_dir) {
    _u = d_dir.cross(Vector{0, 1, 0});
    _v = _u.cross(d_dir);
    _w = d_dir * -1;

    _u.normalize();
    _v.normalize();
    _w.normalize();

    return *this;
}

}
