#ifndef TINYSTL_TYPE_TRAITS_H_
#define TINYSTL_TYPE_TRAITS_H_

// 这个头文件用于提取类型信息

//  use standard header for type traits
#include <type_traits>

// SGI-STL 中，所有的基本类型都i是设置为 _true_type 类别，
// 而所有的对象都会被特华为 _false_type 类别，这是为了保守而不得这样做。
// 也因为 _true_type 类别让我们在执行普通类别时能够以最大效率对其的 copy ，析构等操作。

namespace tinystl
{
	struct _true_type {};
	struct _false_type {};

} // namespace tinystl
#endif // !TINYSTL_TYPE_TRAITS_H_
