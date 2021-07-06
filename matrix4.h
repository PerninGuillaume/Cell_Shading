#pragma once
#include <vector>
#include <array>
#include <iostream>

namespace mygl {

    class matrix4 {
    public:
        matrix4();
        static matrix4 identity();
        void operator*=(const matrix4 &rhs);


        std::array<std::array<float, 4>, 4> matrix_;
    };

}

std::ostream &operator<<(std::ostream &out, const mygl::matrix4 &m);
void frustrum(mygl::matrix4 &mat, const float &left, const float &right,
              const float &bottom, const float &top, const float &z_near, const float &z_far
);

void look_at(mygl::matrix4 &mat,
             const float &eyeX, const float &eyeY, const float &eyeZ,
             const float &centerX, const float &centerY, const float &centerZ,
             float upX, float upY, float upZ
);
