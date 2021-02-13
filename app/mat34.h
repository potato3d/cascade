#pragma once
#include <bl/bl.h>

namespace app
{
    struct mat34
    {
        float data[12];

        mat34(){}

        explicit mat34(const mat4& m)
        {
            int i = 0;
            data[i++] = m.at(0,0);	data[i++] = m.at(0,1);	data[i++] = m.at(0,2);	data[i++] = m.at(0,3);
            data[i++] = m.at(1,0);	data[i++] = m.at(1,1);	data[i++] = m.at(1,2);	data[i++] = m.at(1,3);
            data[i++] = m.at(2,0);	data[i++] = m.at(2,1);	data[i++] = m.at(2,2);	data[i++] = m.at(2,3);
        }

        mat4 as_mat4() const
        {
            return mat4(data[0], data[1], data[2], data[3],
                    data[4], data[5], data[6], data[7],
                    data[8], data[9], data[10], data[11],
                    0,0,0,1);
        }
    };
} // namespace app
