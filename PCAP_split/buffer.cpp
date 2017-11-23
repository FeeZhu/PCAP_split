
#include "stdafx.h"
#include <memory>
#include <fstream>
#include "buffer.h"

buffer::buffer()
{
	_capacity = 512;
	_buf = new byte[_capacity];
	_pos = 0;
	_count = 0;
}
buffer::buffer(size_t s)
{
	_capacity = s;
	_buf = new byte[_capacity];
	_pos = 0;
	_count = 0;
}
buffer::buffer(const buffer& other)
{
	_capacity = other._capacity;
	_pos = other._pos;
	_count = other._count;
	_buf = new byte[_capacity];
	memcpy(_buf, other._buf, _count);
}
buffer::~buffer()
{
	_count = 0;
	delete[] _buf;
	_buf = NULL;
}

buffer& buffer::operator=(const buffer& other)
{
	if(this != &other){
		_capacity = other._capacity;
		_pos = other._pos;
		_count = other._count;
		delete[] _buf;
		_buf = new byte[_capacity];
		memcpy(_buf, other._buf, _count);
	}
	return (*this);
}

void buffer::clear()
{
	_count = 0;
	_pos = 0;
}
void buffer::recapacity(size_t s)
{
	if (s <= 1) {
		return;
	}

	size_t cnt = s>_count ? _count : s;
	byte* newBuf = new byte[s];
	memcpy((void*)newBuf, (void*)(&_buf[_pos]), cnt);
	delete[] _buf;
	_buf = newBuf;
	_pos = 0;
	_count = cnt;
	_capacity = s;
}
void buffer::reserve(size_t cnt)
{
	size_t front = _pos;
	size_t behind = _capacity - _pos - _count;
	if (behind < cnt) {
		if (front + behind >= cnt) {
			for (size_t i = 0; i<_count; ++i) {
				_buf[i] = _buf[_pos + i];
			}
			_pos = 0;
		}
		else {
			recapacity(_capacity * 2);
		}
	}
}
void buffer::resize(size_t s)
{
	if (_count < s) {
		size_t cnt = s - _count;
		reserve(cnt);
	}
	_count = s;
}
void buffer::swap(buffer& other)
{
	if(this != &other){
		auto tmpCapacity = _capacity;
		auto tmpPos = _pos;
		auto tmpCount = _count;
		auto tmpBuf = _buf;

		_capacity = other._capacity;
		_pos = other._pos;
		_count = other._count;
		_buf = other._buf;

		other._capacity = tmpCapacity;
		other._pos = tmpPos;
		other._count = tmpCount;
		other._buf = tmpBuf;
	}
}

size_t buffer::capacity() const
{
	return _capacity;
}
size_t buffer::size() const
{
	return _count;
}
byte buffer::operator[](size_t idx)
{
	return (_count>0 && idx<_count) ? _buf[_pos + idx] : 0;
}
byte* buffer::buf()
{
	return _count != 0 ? &(_buf[_pos]) : NULL;
}
byte* buffer::buf() const
{
	return _count != 0 ? &(_buf[_pos]) : NULL;
}

byte buffer::pop_front()
{
	byte b = 0;
	if (_count > 0) {
		b = _buf[_pos];
		++_pos;
		--_count;
	}
	return b;
}
byte buffer::pop_back()
{
	byte b = 0;
	if (_count > 0) {
		b = _buf[_pos + _count - 1];
		--_count;
	}
	return b;
}
void buffer::pop_front(size_t n)
{
	for (size_t i = 0; i<n; ++i) {
		pop_front();
	}
}
void buffer::pop_back(size_t n)
{
	for (size_t i = 0; i<n; ++i) {
		pop_back();
	}
}
void buffer::push_back(const byte b)
{
	push_back(&b, 1);
}
void buffer::push_back(const byte* bs, size_t count)
{
	reserve(count);
	memcpy((void*)(&_buf[_pos + _count]), (void*)bs, count);
	_count += count;
}

bool read(FILE* fd, buffer* buf, size_t destCnt)
{
	while(buf->size() < destCnt){
		size_t len = buf->size();
		buf->resize(destCnt);		// 扩充buffer以保证有足够的空间可供is.read()使用
		byte* start = buf->buf() + len;
		size_t n = fread(start, 1, destCnt-len, fd);
		if(n <= 0){
			return false;
		}
		buf->resize(len+n);			// 根据实际读取的字节数调整buffer
	}
	return true;
}
bool read(std::istream& is, buffer* buf, size_t destCnt)
{
	while(buf->size() < destCnt){
		size_t len = buf->size();
		buf->resize(destCnt);		// 扩充buffer以保证有足够的空间可供is.read()使用
		byte* start = buf->buf() + len;
		is.read(start, destCnt-len);
		std::streamsize n = is.gcount();
		if(n <= 0){
			return false;
		}
		buf->resize(len+n);			// 根据实际读取的字节数调整buffer
	}
	return true;
}
