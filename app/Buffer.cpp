#include <app/Buffer.h>
#include <glb/opengl.h>

namespace app
{
	void Buffer::init(unsigned int maxSizeBytes)
	{
		_maxSizeBytes = maxSizeBytes;
		_sizeBytes = 0;
		_count = 0;
		glCreateBuffers(1, &_id);
		glNamedBufferData(_id, _maxSizeBytes, nullptr, GL_STATIC_DRAW);
	}

	void Buffer::write(unsigned int offsetBytes, unsigned int sizeBytes, const void* data)
	{
		glNamedBufferSubData(_id, offsetBytes, sizeBytes, data);
	}

	void Buffer::read(unsigned int offsetBytes, unsigned int sizeBytes, void* data)
	{
		glGetNamedBufferSubData(_id, offsetBytes, sizeBytes, data);
	}

	unsigned int Buffer::getCount()
	{
		return _count;
	}

	unsigned int Buffer::getCurrSizeBytes()
	{
		return _sizeBytes;
	}

	unsigned int Buffer::getID()
	{
		return _id;
	}

	void Buffer::_addData(unsigned int sizeBytes, const void* data)
	{
		glNamedBufferSubData(_id, _sizeBytes, sizeBytes, data);
		_sizeBytes += sizeBytes;
	}
}
