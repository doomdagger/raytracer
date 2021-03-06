//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
//

#ifndef RAYTRACER_BVH_NODE_H
#define RAYTRACER_BVH_NODE_H


#include <vector>
#include <algorithm>
#include <limits>

#include "mmgl/surface/surface.h"

namespace mmgl {

/**
 * Class for nodes in the BVH tree.
 * Derived from Surface base class.
 */
class BVHNode : public Surface {
public:
    friend class Scene;

    friend class Camera;

private:
    /**
     * Create bvh tree with a lot of pointers, scene is in charge of deleting all of these pointers.
     */
    static Surface *create_bvh_tree(const std::vector<Surface *>::iterator &begin,
                                    const std::vector<Surface *>::iterator &end, const BVH &bvh_mode);

    /**
     * Helper function for computing volume.
     */
    static float compute_volume(const std::vector<Surface *>::const_iterator &begin,
                                const std::vector<Surface *>::const_iterator &end);

    /*
     * Helper function for determining cut position.
     */
    static const std::vector<Surface *>::iterator determine_cut(const std::vector<Surface *>::iterator &begin,
                                                                const std::vector<Surface *>::iterator &end);

    /**
     * Intersect function, from parent node to all its leaves.
     */
    static void intersect(Ray &ray, const BVHNode *const parent, const Surface *const surface, const Render &flag);

    BVHNode(const std::vector<Surface *>::const_iterator &begin,
            const std::vector<Surface *>::const_iterator &end);

    bool intersect(Ray &, const Render &) const;

    std::string to_string() const;

    Surface *_left;
    Surface *_right;
};

/**
 * Function object for comparing bounding boxes.
 */
struct BBoxComparable {
    enum class CompareFlag {
        X, Y, Z
    };
    CompareFlag _flag;

    BBoxComparable(const CompareFlag &f = CompareFlag::X) : _flag{f} { }

    bool operator()(const Surface *l, const Surface *r) {
        if (_flag == CompareFlag::X) {
            return l->box().min().x() < r->box().min().x();
        } else if (_flag == CompareFlag::Y) {
            return l->box().min().y() < r->box().min().y();
        } else {
            return l->box().min().z() < r->box().min().z();
        }
    }
};

}

#endif //RAYTRACER_BVH_NODE_H
