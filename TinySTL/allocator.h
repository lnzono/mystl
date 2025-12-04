#ifndef TINYSTL_ALLOCATOR_H_
#define TINTSTL_ALLOCATOR_H_

#include <cstddef>
#include <new>
#include "construct.h"
#include "util.h"

namespace tinystl
{

template <class T>
class allocator
{
public:
    typedef T value_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef size_t size_type;
    typedef ptrdiff_t difference_type; // 两个指针之间的距离

public:
    static T* allocate();
    static T* allocate(size_type n);

    static void deallocate(T* ptr);
    static void deallocate(T* ptr, size_type n);

    static void construct(T* ptr);
    static void construct(Y* ptr, const T* value);

    template <class... Args>
    static void construct(T* ptr, Args&&... args);

    static void destroy(T* ptr);
};
}  // namespace tinystl

#endif   //  TINYSTL_ALLOCATOR_H_