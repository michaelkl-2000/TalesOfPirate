#include "stdafx.h"
#include "MPDataStream.h"

#include "MPStringUtil.h"

//=============================================================================
// MPDataStream
//=============================================================================
template <typename T>
MPDataStream& MPDataStream::operator>>(T& val) {
	read(static_cast<void*>(&val), sizeof(T));
}

//-----------------------------------------------------------------------------
String MPDataStream::getLine(bool trimAfter) {
	MPStringUtil::StrStreamType str;
	size_t c = MP_STREAM_TEMP_SIZE - 1;
	while (c == MP_STREAM_TEMP_SIZE - 1) {
		c = readLine(_TmpArea, MP_STREAM_TEMP_SIZE - 1);
		str << _TmpArea;
	}

	String retString(str.str());
	if (trimAfter) {
		MPStringUtil::trim(retString);
	}

	return retString;
}

//-----------------------------------------------------------------------------
String MPDataStream::getAsString(void) {
	char* pBuf = new char[_Size + 1];
	read(pBuf, _Size);
	pBuf[_Size] = '\0';
	String str;
	str.insert(0, pBuf, _Size);
	delete [] pBuf;
	return str;
}

//=============================================================================
// MPMemoryDataStream
//=============================================================================
MPMemoryDataStream::MPMemoryDataStream(void* pMem, size_t size, bool freeOnClose)
	: MPDataStream() {
	_Data = _Pos = static_cast<uchar*>(pMem);
	_Size = size;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(const String& name, void* pMem, size_t size,
									   bool freeOnClose)
	: MPDataStream(name) {
	_Data = _Pos = static_cast<uchar*>(pMem);
	_Size = size;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(MPDataStream& sourceStream, bool freeOnClose)
	: MPDataStream() {
	_Size = sourceStream.size();
	_Data = new uchar[_Size];
	sourceStream.read(_Data, _Size);
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(MPDataStreamPtr& sourceStream, bool freeOnClose)
	: MPDataStream() {
	_Size = sourceStream->size();
	_Data = new uchar[_Size];
	sourceStream->read(_Data, _Size);
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(const String& name, MPDataStream& sourceStream, bool freeOnClose)
	: MPDataStream(name) {
	_Size = sourceStream.size();
	_Data = new uchar[_Size];
	sourceStream.read(_Data, _Size);
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(const String& name, const MPDataStreamPtr& sourceStream, bool freeOnClose)
	: MPDataStream(name) {
	_Size = sourceStream->size();
	_Data = new uchar[_Size];
	sourceStream->read(_Data, _Size);
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(size_t size, bool freeOnClose)
	: MPDataStream() {
	_Size = size;
	_Data = new uchar[_Size];
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::MPMemoryDataStream(const String& name, size_t size, bool freeOnClose)
	: MPDataStream(name) {
	_Size = size;
	_Data = new uchar[_Size];
	_Pos = _Data;
	_End = _Data + _Size;
	_FreeOnClose = freeOnClose;
}

//-----------------------------------------------------------------------------
MPMemoryDataStream::~MPMemoryDataStream() {
	close();
}

//-----------------------------------------------------------------------------
size_t MPMemoryDataStream::read(void* buf, size_t count) {
	size_t cnt = count;

	if (_Pos + cnt > _End)
		cnt = _End - _Pos;
	if (cnt == 0)
		return 0;

	memcpy(buf, _Pos, cnt);
	_Pos += cnt;
	return cnt;
}

//-----------------------------------------------------------------------------
size_t MPMemoryDataStream::readLine(char* buf, size_t maxCount, const String& delim) {
	// UnixWindows DOS/Windows 
	//  CR LF UNIX 
	bool trimCR = false;
	if (delim.find_first_of('\n') != String::npos) {
		trimCR = true;
	}

	// ("\n")
	size_t pos = strcspn((const char*)_Pos, delim.c_str());
	if (pos > maxCount)
		pos = maxCount;

	// poseof
	if (_Pos + pos > _End) {
		pos = _End - _Pos;
	}

	if (pos > 0) {
		memcpy(buf, (const void*)_Pos, pos);
	}

	_Pos += pos + 1;

	// CR LF
	if (trimCR && buf[pos - 1] == '\r') {
		// '\r'
		--pos;
	}
	buf[pos] = '\0';

	return pos;
}

//-----------------------------------------------------------------------------
size_t MPMemoryDataStream::skipLine(const String& delim) {
	// ("\n")
	size_t pos = strcspn((const char*)_Pos, delim.c_str());

	// poseof
	if (_Pos + pos > _End) {
		pos = _End - _Pos;
	}

	_Pos += pos + 1;

	return pos;
}

//-----------------------------------------------------------------------------
void MPMemoryDataStream::skip(long count) {
	size_t newpos = (size_t)((_Pos - _Data) + count);
	assert(_Data + newpos <= _End);

	_Pos = _Data + newpos;
}

//-----------------------------------------------------------------------------
void MPMemoryDataStream::seek(size_t pos) {
	assert(_Data + pos <= _End);
	_Pos = _Data + pos;
}

//-----------------------------------------------------------------------------
size_t MPMemoryDataStream::tell(void) const {
	return _Pos - _Data;
}

//-----------------------------------------------------------------------------
bool MPMemoryDataStream::eof(void) const {
	return _Pos >= _End;
}

//-----------------------------------------------------------------------------
void MPMemoryDataStream::close(void) {
	if (_FreeOnClose && _Data) {
		delete [] _Data;
		_Data = 0;
	}
}

//=============================================================================
// MPFileStreamDataStream
//=============================================================================
MPFileStreamDataStream::MPFileStreamDataStream(std::ifstream* s, bool freeOnClose)
	: MPDataStream() {
	_pStream->seekg(0, std::ios_base::end);
	_Size = _pStream->tellg();
	_pStream->seekg(0, std::ios_base::beg);
}

//-----------------------------------------------------------------------------
MPFileStreamDataStream::MPFileStreamDataStream(const String& name, std::ifstream* s, bool freeOnClose)
	: MPDataStream(name) {
	_pStream->seekg(0, std::ios_base::end);
	_Size = _pStream->tellg();
	_pStream->seekg(0, std::ios_base::beg);
}

//-----------------------------------------------------------------------------
MPFileStreamDataStream::MPFileStreamDataStream(const String& name, std::ifstream* s, size_t size, bool freeOnClose)
	: MPDataStream(name) {
	_Size = size;
}

//-----------------------------------------------------------------------------
MPFileStreamDataStream::~MPFileStreamDataStream() {
	close();
}

//-----------------------------------------------------------------------------
size_t MPFileStreamDataStream::read(void* buf, size_t count) {
	_pStream->read(static_cast<char*>(buf), count);
	return _pStream->gcount();
}

//-----------------------------------------------------------------------------
size_t MPFileStreamDataStream::readLine(char* buf, size_t maxCount, const String& delim) {
	if (delim.empty()) {
		ToLogService("common", "FileStreamDataStream::readLine");
		assert(0);
	}
	if (delim.size() > 1) {
		ToLogService("common", "using only first delimeter");
	}
	// UnixWindows
	bool trimCR = false;
	if (delim.at(0) == '\n') {
		trimCR = true;
	}
	// maxCount + 1
	_pStream->getline(buf, maxCount + 1, delim.at(0));
	size_t ret = _pStream->gcount();
	// 3
	// 1) eof
	// 2) 
	// 3)  - 
	//    ret-1,ret-2
	// null

	if (_pStream->eof()) {
	}
	else if (_pStream->fail()) {
		// Did we fail because of maxCount hit? No - no terminating character
		// in included in the count in this case
		if (ret == maxCount) {
			// clear failbit for next time 
			_pStream->clear();
		}
		else {
			ToLogService("common", "FileStreamDataStream::readLine");
			assert(0);
		}
	}
	else {
		--ret;
	}

	// CR LF
	if (trimCR && buf[ret - 1] == '\r') {
		--ret;
		buf[ret] = '\0';
	}
	return ret;
}

//-----------------------------------------------------------------------------
size_t MPFileStreamDataStream::skipLine(const String& delim) {
	size_t c = MP_STREAM_TEMP_SIZE - 1;
	size_t total = 0;
	while (c == MP_STREAM_TEMP_SIZE - 1) {
		c = readLine(_TmpArea, MP_STREAM_TEMP_SIZE - 1);
		total += c;
	}
	return total;
}

//-----------------------------------------------------------------------------
void MPFileStreamDataStream::skip(long count) {
	_pStream->clear(); //eof
	_pStream->seekg(static_cast<std::ifstream::pos_type>(count), std::ios::cur);
}

//-----------------------------------------------------------------------------
void MPFileStreamDataStream::seek(size_t pos) {
	_pStream->clear(); //eof
	_pStream->seekg(static_cast<std::ifstream::pos_type>(pos), std::ios::beg);
}

//-----------------------------------------------------------------------------
size_t MPFileStreamDataStream::tell(void) const {
	_pStream->clear(); //eof
	return _pStream->tellg();
}

//-----------------------------------------------------------------------------
bool MPFileStreamDataStream::eof(void) const {
	return _pStream->eof();
}

//-----------------------------------------------------------------------------
void MPFileStreamDataStream::close(void) {
	if (_pStream) {
		_pStream->close();
		if (_FreeOnClose) {
			delete _pStream;
			_pStream = 0;
		}
	}
}

//=============================================================================
// MPFileHandleDataStream
//=============================================================================
MPFileHandleDataStream::MPFileHandleDataStream(FILE* handle)
	: MPDataStream(), _FileHandle(handle) {
	fseek(_FileHandle, 0, SEEK_END);
	_Size = ftell(_FileHandle);
	fseek(_FileHandle, 0, SEEK_SET);
}

//-----------------------------------------------------------------------------
MPFileHandleDataStream::MPFileHandleDataStream(const String& name, FILE* handle)
	: MPDataStream(name), _FileHandle(handle) {
	fseek(_FileHandle, 0, SEEK_END);
	_Size = ftell(_FileHandle);
	fseek(_FileHandle, 0, SEEK_SET);
}

//-----------------------------------------------------------------------------
MPFileHandleDataStream::~MPFileHandleDataStream() {
	close();
}

//-----------------------------------------------------------------------------
size_t MPFileHandleDataStream::read(void* buf, size_t count) {
	return fread(buf, count, 1, _FileHandle);
}

//-----------------------------------------------------------------------------
size_t MPFileHandleDataStream::readLine(char* buf, size_t maxCount, const String& delim) {
	// UnixWindows
	bool trimCR = false;
	if (delim.find_first_of('\n') != String::npos) {
		trimCR = true;
	}

	size_t chunkSize = std::min(maxCount, static_cast<size_t>(MP_STREAM_TEMP_SIZE - 1));
	size_t totalCount = 0;
	size_t readCount;
	while (chunkSize && (readCount = fread(_TmpArea, chunkSize, 1, _FileHandle))) {
		_TmpArea[readCount] = '\0';
		size_t pos = strcspn(_TmpArea, delim.c_str());

		if (pos < readCount) {
			fseek(_FileHandle, pos - readCount + 1, SEEK_CUR);
		}

		if (pos > 0) {
			// CR
			if (trimCR && _TmpArea[pos - 1] == '\r') {
				--pos;
			}

			if (buf) {
				memcpy(buf, (const void*)_TmpArea, pos);
				buf[pos] = '\0';
			}
			totalCount += pos;
		}

		if (pos < readCount) {
			break;
		}
		chunkSize = std::min(maxCount - totalCount, static_cast<size_t>(MP_STREAM_TEMP_SIZE - 1));
	}
	return totalCount;
}

//-----------------------------------------------------------------------------
size_t MPFileHandleDataStream::skipLine(const String& delim) {
	// readLine(), 
	char* nullBuf = 0;
	return readLine(nullBuf, 1024, delim);
}

//-----------------------------------------------------------------------------
void MPFileHandleDataStream::skip(long count) {
	fseek(_FileHandle, count, SEEK_CUR);
}

//-----------------------------------------------------------------------------
void MPFileHandleDataStream::seek(size_t pos) {
	fseek(_FileHandle, pos, SEEK_SET);
}

//-----------------------------------------------------------------------------
size_t MPFileHandleDataStream::tell(void) const {
	return ftell(_FileHandle);
}

//-----------------------------------------------------------------------------
bool MPFileHandleDataStream::eof(void) const {
	return feof(_FileHandle) != 0;
}

//-----------------------------------------------------------------------------
void MPFileHandleDataStream::close(void) {
	fclose(_FileHandle);
	_FileHandle = 0;
}
