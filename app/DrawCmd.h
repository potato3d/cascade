#pragma once

namespace app
{
	struct DrawCmd
	{
		unsigned int elementCount = 0;
		unsigned int instanceCount = 0;
		unsigned int firstElement = 0;
		unsigned int baseVertex = 0;
		unsigned int baseInstance = 0;
	};
} // namespace app
