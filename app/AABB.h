#pragma once
#include <bl/bl.h>

namespace app
{
	struct AABB
	{
		vec3 min = vec3(math::limit_posf());
		vec3 max = vec3(math::limit_negf());

		void expand(const vec3& v)
		{
			min.x = math::min(min.x, v.x);
			min.y = math::min(min.y, v.y);
			min.z = math::min(min.z, v.z);

			max.x = math::max(max.x, v.x);
			max.y = math::max(max.y, v.y);
			max.z = math::max(max.z, v.z);
		}

		void expand(const AABB& o)
		{
			min.x = math::min(min.x, o.min.x);
			min.y = math::min(min.y, o.min.y);
			min.z = math::min(min.z, o.min.z);

			max.x = math::max(max.x, o.max.x);
			max.y = math::max(max.y, o.max.y);
			max.z = math::max(max.z, o.max.z);
		}

		bool isValid() const
		{
			return min.x <= max.x && min.y <= max.y && min.z <= max.z;
		}

		vec3 getCenter() const
		{
			return (min + max) * 0.5f;
		}

		vec3 getSize() const
		{
			return max - min;
		}
	};
}
