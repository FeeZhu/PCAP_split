
#pragma once

typedef char byte;
typedef unsigned char ubyte;

class buffer
{
public:
	buffer();
	buffer(size_t s);
	buffer(const buffer& other);
	~buffer();

	buffer& operator=(const buffer& other);

	void clear();
	void recapacity(size_t s);
	void reserve(size_t cnt);
	void resize(size_t s);
	void swap(buffer& other);

	size_t capacity() const;
	size_t size() const;
	byte operator[](size_t idx);
	byte* buf();
	byte* buf() const;

	byte pop_front();
	byte pop_back();
	void pop_front(size_t n);
	void pop_back(size_t n);

	void push_back(const byte b);
	void push_back(const byte* bs, size_t count);

private:
	byte*   _buf;
	size_t  _capacity;
	size_t  _pos;
	size_t  _count;
};


bool read(FILE* fd, buffer* buf, size_t destCnt);
bool read(std::istream& is, buffer* buf, size_t destCnt);
