/**
 * @file	cyclic_buffer.h
 * @brief	A cyclic buffer is a data structure that uses a single, fixed-size buffer as if it were connected end-to-end.
 *
 * http://en.wikipedia.org/wiki/Circular_buffer
 *
 * @date Dec 8, 2013
 * @author Andrey Belomutskiy, Daniel Hill
 *
 * Daniel Hill - Modified to use C++ - Mar 2, 2014
*/

#ifndef CYCLIC_BUFFER_H
#define CYCLIC_BUFFER_H

#include <string.h>
#include "rusefi_true.h"

static const short CB_MAX_SIZE = 128;

#define BUFFER_MAX_VALUE 200123

template<typename T, size_t maxSize = CB_MAX_SIZE>
class cyclic_buffer
{
  public:
	cyclic_buffer();
    explicit cyclic_buffer(int size);

  public:
    void add(T value);
    T get(int index) const;
    T sum(int length) const;
    T maxValue(int length) const;
    T minValue(int length) const;
    void setSize(unsigned int size);
    bool contains(T value) const;
    int getSize() const;
    int getCount() const;
    void clear();
    volatile T elements[maxSize];
    volatile int currentIndex;

  private:
    void baseC(int size);
    /**
     * number of elements added into this buffer, would be eventually bigger then size
     */
    volatile int count;
    int size;
};

template<typename T, size_t maxSize>
cyclic_buffer<T, maxSize>::cyclic_buffer() {
  baseC(maxSize);
}

template<typename T, size_t maxSize>
cyclic_buffer<T, maxSize>::cyclic_buffer(int size) {
  baseC(size);
}

template<typename T, size_t maxSize>
void cyclic_buffer<T, maxSize>::baseC(int size) {
	currentIndex = 0;
	memset((void*)&elements, 0, sizeof(elements));
	setSize(size);
}

template<typename T, size_t maxSize>
void cyclic_buffer<T, maxSize>::add(T value) {
	elements[currentIndex] = value;

	++currentIndex;
	if (currentIndex == size) {
		currentIndex = 0;
	}

	++count;
}

template<typename T, size_t maxSize>
bool cyclic_buffer<T, maxSize>::contains(T value) const {
	for (int i = 0; i < currentIndex ; i++) {
		if (elements[i] == value) {
			return TRUE;
		}
	}
	return FALSE;
}

template<typename T, size_t maxSize>
void cyclic_buffer<T, maxSize>::setSize(unsigned int size) {
	clear();
	this->size = size < maxSize ? size : maxSize;
}

template<typename T, size_t maxSize>
int cyclic_buffer<T, maxSize>::getSize() const {
	return size;
}

template<typename T, size_t maxSize>
int cyclic_buffer<T, maxSize>::getCount() const {
	return count;
}

template<typename T, size_t maxSize>
T cyclic_buffer<T, maxSize>::get(int index) const {
	while (index < 0) {
		index += size;
	}
	while (index >= size) {
		index -= size;
	}
	return elements[index];
}

template<typename T, size_t maxSize>
T cyclic_buffer<T, maxSize>::maxValue(int length) const {
	if (length > count) {
		// not enough data in buffer
		length = count;
	}
	int ci = currentIndex; // local copy to increase thread-safety
	T result = -BUFFER_MAX_VALUE; // todo: better min value?
	for (int i = 0; i < length; ++i) {
		int index = ci - i - 1;
		while (index < 0) {
			index += size;
		}

		if (elements[index] > result) {
			result = elements[index];
		}
	}
	return result;
}

template<typename T, size_t maxSize>
T cyclic_buffer<T, maxSize>::minValue(int length) const {
	if (length > count) {
		length = count;
	}
	int ci = currentIndex; // local copy to increase thread-safety
	T result = +BUFFER_MAX_VALUE; // todo: better max value?
	for (int i = 0; i < length; ++i) {
		int index = ci - i - 1;
		while (index < 0) {
			index += size;
		}

		if (elements[index] < result) {
			result = elements[index];
		}
	}
	return result;
}

template<typename T, size_t maxSize>
T cyclic_buffer<T, maxSize>::sum(int length) const {
	if (length > count) {
		length = count;
	}

	int ci = currentIndex; // local copy to increase thread-safety
	T result = 0;

	for (int i = 0; i < length; ++i) {
		int index = ci - i - 1;
		while (index < 0) {
			index += size;
		}

		result += elements[index];
	}

	return result;
}

template<typename T, size_t maxSize>
void cyclic_buffer<T, maxSize>::clear() {
	memset((void*) elements, 0, sizeof(elements)); // I would usually use static_cast, but due to the elements being volatile we cannot.
	count = 0;
	currentIndex = 0;
}

#endif //CYCLIC_BUFFER_H
