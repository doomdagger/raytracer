//
// Final Project for COMS 4998: C++ Library Design
// Author: He Li(hl2918), Haoxiang Xu(hx2185), Wangda Zhang(zwd)
// Code Credit to deusmacabre (http://stackoverflow.com/users/318726/deusmacabre):
// A post from stackoverflow about how to write bmp image in pure c/c++, see the link below
// http://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries
//

#ifndef MMGL_IMAGE_H
#define MMGL_IMAGE_H

#include <vector>
#include <cstring>

#include "mmgl/util/common.h"

namespace mmgl {

/**
 * This class maintains the rendering results in an RGB matrix.
 */
class Image {
public:
    Image(): _image{} {}

    Image(int width, int height);

    inline int width() const {
        return _width;
    }

    inline int height() const {
        return _height;
    }

    /**
     * This member function returns a handle to the RenderResult, i.e. RGB matrix.
     * @return a const reference to the underlying _image matrix.
     */
    inline const RenderResult& handle() const {
        return _image;
    }

    void resize(int width, int height);

    void clear();

    inline std::vector<Vector> &operator[](int height) {
        return _image[height];
    }

    inline const std::vector<Vector> &operator[](int height) const {
        return _image[height];
    }

    void pixel(int width, int height, const Vector &rgb);

    const Vector &pixel(int width, int height) const;

    void save(const std::string &file_path) const;

private:
    int _width;
    int _height;
    std::vector<std::vector<Vector>> _image;
};

}


#endif //MMGL_IMAGE_H
