#ifndef TINYSTL_CONSTRUCT_H_
#define TINYSTL_CONSTRUCT_H_

// 这个头文件包含了 construct ，用于对象的构造与析构


#include <new>        // placement new
#include "type_traits.h"
#include "iterator.h"
namespace tinystl
{
	// construct ：负责对象构造
	template <class Ty>
	void construct(Ty* ptr)
	{
		// 将 ptr 强转为 void* 类型，然后在 ptr 上使用默认构造函数
		::new((void*)ptr) Ty();
	}
	template <class Ty1, class Ty2>
	void construct(Ty1* ptr, const Ty2& value)
	{
		// 将 ptr 强转为 void* 类型，然后在 ptr 上使用拷贝构造函数
		::new((void*)p) Ty1(value);
	}

	template <class Ty, class... Args>
	void construct(Ty* ptr, Args&&... args)
	{
		// 带参数构造和移动构造
		::new((void*)ptr) Ty(tinystl::forward<Args>(args)...);
	}

	// destroy ：负责对象析构

	template <class Ty>
	// 平凡析构不做任何操作
	void destory_one(Ty*, std::true_type) {}

	template <class Ty>
	// 非平凡析构调用析构函数
	void destory_one(Ty* ptr, std::false_type)
	{
		if (ptr != nullptr) {
			ptr->~Ty();
		}
	}


	template <class ForwardIterator>
	// 平凡析构，不做任何处理
	void destroy_cat(ForwardIterator, ForwardIterator, std::true_type) {}

	template <class ForwardIterator>
	// 非平凡析构，逐个调用析构函数
	void destroy_cat(ForwardIterator first, ForwardIterator last, std::false_type)
	{
		for (; first != last; ++first) {
			destory(&*first);
		}
	}

	template <class Ty>
	void destory(Ty* ptr)
	{
		// 根据 Ty 是否为平凡析构类型，选择不同的析构方式
		destory_one(ptr, std::is_trivially_destructible<Ty>{});
	}

	template <class ForwardIterator>
	void destory(ForwardIterator first, ForwardIterator last)
	{
		// 根据迭代器所指对象类型是否为平凡析构类型，选择不同的析构方式
		destroy_cat(first, last, std::is_trivially_destructible<
			typename iterator_traits<ForwardIterator>::value_tye>{});
	}
} // namespace tinystl

#endif // TINYSTL_CONSTRUCT_H_
