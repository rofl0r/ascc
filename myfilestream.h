#ifndef MYFILESTREAM_H
#define MYFILESTREAM_H

#include <stdio.h>

// this implements the bare minimum to get cc_compiler.Write()
// working. the upstream filestream class pulls in a dozen of additional
// files, thanks to "great OOP" inheritance paradigms.

class FileWriteStream
{
public:
	//FileStream(const char* file_name);
	//~FileStream() override;

	inline int Open(const char *file_name) {
		_file = fopen(file_name, "wb");
		return !!_file;
	}

	inline void Close() {
		fclose(_file);
	}

	inline size_t  Write(const void *buffer, size_t size) {
		return fwrite(buffer, 1, size, _file);
	}
	inline size_t WriteArray(const void *buffer, size_t elem_size, size_t count)
	{
		return Write(buffer, elem_size * count) / elem_size;
	}
	inline size_t WriteArrayOfInt32(const int32_t *buffer, size_t count)
	{
		return
#if 0
			MustSwapBytes() ?
				WriteAndConvertArrayOfInt32(buffer, count) :
#endif
#warning "this code is currently not working on big-endian"
			WriteArray(buffer, sizeof(int32_t), count);
	}
	inline size_t  WriteInt32(int32_t val) {
		return Write(&val, 4);
	}

private:
    FILE                *_file;
};


#endif
