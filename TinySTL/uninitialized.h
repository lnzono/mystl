#ifndef TINYSTL_UNINITIALIZED_H_
#define TINYSTL_UNINITIALIZED_H_

//这个头文件用于初始化未初始化空间构造元素

//涉及到的知识点：（详见 Notes\Allocators\note3.md）
// 1.typename
// 2.std::is_trivially_copy_assignable

#include<new>			// std::true_type,std::false_type
#include "algobase.h"	// copy.copy_n,copy_backward

namespace tinystl
{
	// TODO：对 char* 与 wchar_t* 做特化处理

	/** ********************************************/
	// uninitialized_copy
	// 将 [first,last) 范围内的元素拷贝构造到以 result 为起始位置的未初始化空间
	/** ********************************************/

	/**
	 * @brief 把[first,last)范围内的元素拷贝到以result为起始位置的未初始化空间，返回复制结束的位置
	 * @param InputIterator 输入迭代器
	 * @param ForwardIterator 前向迭代器
	 * @param first 输入迭代器的起始位置
	 * @param last 输入迭代器的结束位置
	 * @param reault 输出迭代器的目标位置
	 * @param std::true_type 用于区分是否为POD类型
	 * @return 返回复制结束的位置
	 *
	 */
	template <class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator reault, std::true_type) {
		return tinystl::copy(first, last, reault); //algobase.h
	}


	template <class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator reault, std::false_type) {
		auto cur = reault;
		try {
			for (; first != last; ++first, ++cur) {
				// 移动构造
				tinystl::construct(&*cur, *first);
			}
		}
		// commit or rollback ,要么产出所有的元素，要么一个都没有
		catch (...) {
			for (; reault != cur; ++reault) {
				// 析构
				tinystl::destroy(&*reault);
			}
		}
		return cur;
	}

	template <class InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result) {
		// 判断类型是否为 POD 类型（平凡拷贝赋值类型），这里使用了 std::is_trivially_copy_assignable
		return tinystl::_uninitialized_copy(first, last, result,
			std::is_trivially_copy_assignable<typename std::iterator_traits<InputIterator>::value_type>{});
	}


	/** ********************************************/
	// uninitialized_copy_n
	// 将 [first,first + n) 范围内的元素拷贝到以 result 为起始位置的未初始化空间，返回复制结束的位置
	/** ********************************************/

	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator _uninitialized_copy_n(InputIterator first, Size n, ForwardIterator result, std::true_type) {
		return tinystl::copy_n(first, n, result);
	}

	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator _uninitialized_copy_n(InputIterator first, Size n, ForwardIterator result, std::false_type) {
		auto cur = result;
		try {
			for (; n > 0; --n, ++first, ++cur) {
				// 移动构造
				tinystl::construct(&*cur, *first);
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				// 析构
				tinystl::destroy(&*result);
			}
		}
		return cur;
	}

	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator uninitialized_copy_n(InputIterator first, Size n, ForwardIterator result) {
		// 判断类型是否为 POD 类型（平凡拷贝赋值类型），
		return tinystl::_uninitialized_copy_n(first, n, result,
			std::is_trivially_copy_assignable<typename std::iterator_traits<InputIterator>::value_type>{});
	}


	/** ********************************************/
	// uninitialized_fill
	// 把[first,last)范围内的元素填充为 value
	/** ********************************************/
	template <class ForwardIterator, class T>
	void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value, std::true_type) {
		tinystl::fill(first, last, value);
	}

	template <class ForwardIterator, class T>
	void _uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value, std::false_type) {
		auto cur = first;
		try {
			for (; cur != last; ++cur) {
				// 带参数构造，也是移动构造
				tinystl::construct(&*cur, value);
			}
		}
		catch (...) {
			for (; first != cur; ++first) {
				// 析构
				tinystl::destroy(&*first);
			}
		}
	}

	template <class ForwardIterator, class T>
	void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& value) {
		tinystl::_uninitialized_fill(first, last, value,
			std::is_trivially_copy_assignable<typename std::iterator_traits<ForwardIterator>::value_type>{});
	}



	/** ********************************************/
	// uninitialized_fill_n
	// 把[first,first + n)范围内的元素填充为 value
	/** ********************************************/

	template <class ForwardIterator, class Size, class T>
	ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T& value, std::true_type) {
		return tinystl::fill_n(first, n, value);
	}

	template <class ForwardIterator, class Size, class T>
	ForwardIterator _uninitialized_fill_n(ForwardIterator first, Size n, const T& value, std::false_type) {
		auto cur = first;
		try {
			for (; n > 0; --n, ++cur) {
				// 带参数构造，也是移动构造
				tinystl::construct(&*cur, value);
			}
		}
		catch (...) {
			for (; first != cur; first++) {
				tinystl::destroy(&*first);
			}
		}
		return cur;
	}

	template <class ForwardIterator, class Size, class T>
	ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& value) {
		return tinystl::_uninitialized_fill_n(first, n, value,
			std::is_trivially_copy_assignable<typename std::iterator_traits<ForwardIterator>::value_type>{});
	}


	/** *******************************/
	// uninitialized_move
	// 将 [first,last) 范围内的元素移动构造到以 result
	/** *******************************/

	template <class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_move(InputIterator first, InputIterator last, ForwardIterator result, std::true_type) {
		return tinystl::move(first, last, result); //algobase.h
	}

	template <class InputIterator, class ForwardIterator>
	ForwardIterator _uninitialized_move(InputIterator first, InputIterator last, ForwardIterator result, std::false_type) {
		auto cur = result;
		try {
			for (; first != last; ++first, ++cur) {
				// 移动构造
				tinystl::construct(&*cur, tinystl::move(*first));
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				// 析构
				tinystl::destroy(&*result);
			}
		}
		return cur;
	}

	template <class InputIterator, class ForwardIterator>
	ForwardIterator uninitialized_move(InputIterator first, InputIterator last, ForwardIterator result) {
		return tinystl::_uninitialized_move(first, last, result,
			std::is_trivially_move_assignable<typename std::iterator_traits<InputIterator>::value_type>{});
	}


	/** *******************************/
	// uninitialized_move_n
	// 将 [first, first + n) 范围内的元素移动构造
	/** *******************************/
	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator _uninitialized_move_n(InputIterator first, Size n, ForwardIterator result, std::true_type) {
		return tinystl::move_n(first, first + n, result); //algobase.h
	}

	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator _uninitialized_move_n(InputIterator first, Size n, ForwardIterator result, std::false_type) {
		auto cur = result;
		try {
			for (; n > 0; --n, ++first, ++cur) {
				// 移动构造
				tinystl::construct(&*cur, tinystl::move(*first));
			}
		}
		catch (...) {
			for (; result != cur; ++result) {
				// 析构
				tinystl::destroy(&*result);
			}
		}
		return cur;
	}

	template <class InputIterator, class Size, class ForwardIterator>
	ForwardIterator uninitialized_move_n(InputIterator first, Size n, ForwardIterator result) {
		return tinystl::_uninitialized_move_n(first, n, result,
			std::is_trivially_move_assignable<typename std::iterator_traits<InputIterator>::value_type>{});
	}

} // namespace tinystl
#endif // TINYSTL_UNINITIALIZED_H_
