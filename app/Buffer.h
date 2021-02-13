#pragma once

namespace app
{
	class Buffer
	{
	public:
		void init(unsigned int maxSizeBytes);

		template<typename T>
		bool addData(unsigned int count, const T* data)
		{
			unsigned int sizeBytes = count*sizeof(T);
			if(_sizeBytes + sizeBytes > _maxSizeBytes)
			{
				return false;
			}
			_addData(sizeBytes, data);
			_count += count;
			return true;
		}

		template<typename T>
		bool setData(unsigned int count, const T* data)
		{
			_sizeBytes = 0;
			_count = 0;
			return addData(count, data);
		}

		void write(unsigned int offsetBytes, unsigned int sizeBytes, const void* data);
		void read(unsigned int offsetBytes, unsigned int sizeBytes, void* data);

		unsigned int getCount();
		unsigned int getCurrSizeBytes();
		unsigned int getID();

	private:
		void _addData(unsigned int sizeBytes, const void* data);

		unsigned int _id = 0;
		unsigned int _maxSizeBytes = 0;
		unsigned int _sizeBytes = 0;
		unsigned int _count = 0;
	};
}
