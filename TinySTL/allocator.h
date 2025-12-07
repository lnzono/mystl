#ifndef TINYSTL_ALLOCATOR_H_
#define TINTSTL_ALLOCATOR_H_

// 这个文件中包含 allocator 类的声明和实现，allocator 用于内存分配，释放和对象构造，析构

// 涉及到的知识点：（详见 Notes\Allocators\note1.md）
// 1.  ::operator new 和 ::operator delete 的具体操作
//2. 移动复制与std::move
// 3. 完美转发，通用引用和引用折叠  std::forward

#include <cstddef>		// ptrdiff_t,size_t
#include <new>			// ::operator new
#include "construct.h"
#include "util.h"

namespace tinystl {
	template <class T>
	class allocator {
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

		static void construct(T* ptr);	// 默认构造
		static void construct(T* ptr, const T& value);	// 拷贝构造
		static void construct(T* ptr, T&& value);	// 移动构造

		template <class... Args>
		static void construct(T* ptr, Args&&... args);	// 带参数的构造

		static void destroy(T* ptr);
		static void destroy(T* first, T* last);
	};

	template <class T>
	T* allocator<T>::allocate() {
		// 使用全局的 ::operator new 分配内存，分配一个T 大小的内存并进行类型转换
		return static_cast<T*>(::operator new(sizeof(T)));
	}

	template <class T>
	T* allocator<T>::allocate(size_type n) {
		if (n == 0)return nullpter;
		// 使用全局的 ::operator new 分配内存,分配 n 个T 大小的内存并进行类型转换
		return static_cast<T*>(::operator new(sizeof(T) * n);
	}

	template <class T>
	void allocator<T>::deallocate(T* ptr) {
		if (ptr == nullptr)return;
		// 使用全局的 ::operator delete 释放 ptr 指向的内存
		::operator delete(ptr);
	}

	template <class T>
	void allocator<T>::deallocate(T* ptr, size_type /* size */) {
		if (ptr == nullptr)return;
		// 使用全局的 ::operator delete 释放 ptr 指向的内存
		::operator delete(ptr);
	}
	template<class T>
	void allocator<T>::construct(T* ptr) {
		// 调用 construct.h 中的 construct 函数进行默认构造
		tinystl::construct(ptr); // 默认构造
	}

	template <class>
	void allocator<T>::construct(T* ptr, const T& value) {
		tinystl::construct(ptr, value); // 拷贝构造
	}

	template <class T>
	void allocator<T>::construct(T* ptr, T&& value) {
		// tinystl::move 将 value 转换为右值引用,详见 util.h
		tinystl::construct(ptr, tinystl::move(value)); // 移动构造
	}

	template <class T>
	template <class... Args>
	void allocator<T>::construct(T* ptr, Args&&... args) {
		// 完美转发参数 args 给 tinystl::construct 进行构造
		// tinystl::forward 用于完美转发,详见 util.h
		tinystl::construct(ptr, tinystl::forward<Args>(args)...);
	}

	template <class T>
	void allocator<T>::destroy(T* ptr) {
		// 调用 construct.h 中的 destroy 函数
		tinystl::destroy(ptr);
	}

	template <class T>
	void allocator<T>::destroy(T* first, T* last) {
		// 析构两个指针之间的所有对象
		tinystl::destroy(first, last);
	}
}  // namespace tinystl

#endif   //  TINYSTL_ALLOCATOR_H_