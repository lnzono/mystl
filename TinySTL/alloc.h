#ifndef TINYSTL_ALLOC_H_
#define TINYSTL_ALLOC_H_

// 这个头文件包含了 alloc ，用于分配和回收内存，一内存池的方式实现

#include <new>			// placement new
#include <cstddef>      // ptrdiff_t,size_t
#include <cstdlib>		// malloc,free
#include <cstdio>		// fprintf

namespace tinystl
{
	//二级空间配置器的实现内容
	enum { _ALIGN = 8 };//小型区块的上调边界
	enum { _MAX_BYTES = 128 };//小型区块的上限,(当前大于128字节的区块由 malloc 分配)
	enum { _NFREELISTS = _MAX_BYTES / _ALIGN };//free-list 个数

	// @brief 共用体 FreeLiset 采用链表的方式管理内存，分配与回收小内存区块
	// 这里未使用 volatile ,因为不涉及到多线程的情况，详见 Notes\Allocators\note2.md
	union FreeList {
		union FreeList* next; // 指向下一个区块
		char data[1]; // 本区块的起始位置
	};

	class alloc {
	private:
		static char* start_free;
		static char* end_free;
		static size_t heap_size;

		static FreeList* free_list[_NFREELISTS]; //	16 条 free-list 链表,申请内存时，会被分配到最近的对齐块中

	public:
		static void* allocate(size_t n);
		static void8 deallocate(void* p, size_t n);
		static void* reallocate(void* p, size_t old_sz, size_t new_sz);

	private:
		static size_t ROUND_UP(size_t bytes); // 上调边界至8的倍数
		static size_t FREELIST_INDEX(size_t bytes); // 根据区块大小计算 free-list 的下标
		static void* refill(size_t n); // 重新填充 free-list
		static char* chunk_alloc(size_t size, size_t& nobjs); // 从内存池中取空间给 free-list 使用

	};

	// 静态成员变量的初始化
	char* alloc::start_free = nullptr; // 内存池起始位置
	char* alloc::end_free = nullptr; //	内存池结束位置
	char* alloc::heap_size = 0; //	申请 heap 空间附加值的大小

	FreeList* alloc::free_list[__NFREELISTS] = {
	   nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,
	   nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr,nullptr
	};

	/**
	 * @brief 分配 n 字节大小的内存
	 * @param n 分配空间的大小
	 * @return 分配空间的首地址
	 *
	 */
	void* alloc::allocate(size_t n)
	{
		FreeList* my_free_list; // 指向对应 free-list 的指针
		FreeList* result; // 返回空间的首地址

		// 大于128字节的区块直接调用 malloc 分配(一级配置器)
		if (n > static_cast<size_t>(_MAX_BYTES)) {
			// TODO ：一级配置器
			return std::malloc(n);
		}

		my_free_list = free_list + FREELIST_INDEX(n);
		result = *my_free_list;
		// 如果对应 free-list 为空，则需要重新填充
		if (result == nullptr) {
			void* r = refill(ROUND_UP(n));
			return r;
		}
		*my_free_list = result->next; // 更新 free-list 首地址，指向下一个区块
		return result;
	}

	/**
	 * @brief 回收 p 指向的 n 字节大小的内存
	 * @param p 待回收空间的首地址
	 * @param n 待回收空间的大小
	 */
	void alloc::deallocate(void* p, size_t n) {
		// 大于128字节的区块直接调用 free 回收(一级配置器)
		if (n > static_cast<size_t>(_MAX_BYTES)) {
			// TODO ：一级配置器
			std::free(p);
			return;
		}

		// 将 p 转换为 FreeList* 类型，关于 reinterpret_cast 可参考 Notes\Cpp\note2.md
		FreeList* q = reinterpret_cast<FreeList*> (p);
		FreeList** my_free_list = free_list + FREELIST_INDEX(n);
		q->next = *my_free_list; // 将回收的区块插入到 free-list 的首部
		*my_free_list = q; // 更新 free-list 的首地址,后续就能正常使用该链表
	}

	/**
	 * @brief 重新分配 p 指向的内存区块
	 * @param p 指向空间的首地址
	 * @param old_sz 旧空间的大小
	 * @param new_sz 新空间的大小
	 * return 新空间的首地址
	 * .
	 */
	void* alloc::reallocate(void* p, size_t old_sz, size_t new_sz) {
		deallocate(p, old_sz);
		p = allocate(new_sz);
		return p;
	}

	/**
	 * @brief 将 bytes 上调至对应区间大小
	 * @param bytes 申请区块大小
	 * @return 上调后的空间大小
	 */
	size_t alloc::ROUND_UP(size_t bytes) {
		// 逻辑等价于 （bytes + (_ALIGN - 1)) / _ALIGN * _ALIGN 位运算快，因此使用
		return (bytes + _ALIGN - 1) & ~(_ALIGN - 1);
	}

	/**
	 * @brief 根据区块大小计算 free-list 的下标
	 * @param bytes 区块大小
	 * @return free-list 的下标
	 */
	size_t alloc::FREELIST_INDEX(size_t bytes)
	{
		// 上调对齐：保证能被 _ALIGN 整除（如 8,16,24, ...）
		// 然后除以 _ALIGN 得到“第几个 8 字节单位”
		// 再减 1 使结果变成从 0 开始的数组下标。
		return ((bytes)+_ALIGN - 1) / _ALIGN - 1;
	}

	/**
	 * @brief 重新填充 free-list
	 * @param n 申请的区块大小
	 * @return 申请到的区块首地址
	 */
	void* alloc::refill(size_t n) {
		size_t nblock = 20; // 一次性申请的区块个数
		char* chunk = chunk_alloc(n, nblock);
		FreeList** my_free_list;
		FreeList* result, * cur, * next;

		if (nblock == 1) {
			return chunk;
		}

		my_free_list = free_list + FREELIST_INDEX(n);
		result = (FreeList*)chunk; // 返回空间的首地址
		// 由于将第一个区块作为返回值，因此需要将 free-list 的首地址便宜 n 个字节，指向下一个区块
		*my_free_list = next = (FreeList*)(chunk + n);
		// 将剩余的区块 插入到 free-list 中

		for (size_t i = 1; ; i++) {
			cur = next;
			next = (FreeList*)((char*)next + n); // 指向下一个区块
			if (nblock - 1 == i) {
				cur->next = nullptr;
				break;
			}
			else {
				cur->next = next;
			}
		}
		return result;
	}

	/**
	 * @brief 从内存池中取空间给 free-list 使用
	 * @param size 申请区块大小
	 * @param nobjs 申请区块个数
	 * @return 申请到的区块首地址
	 */
	char* alloc::chunk_alloc(size_t size, size_t& nobjs) {
		char* result;
		size_t total_bytes = size * nobjs; // 需要申请的总字节数
		size_t bytes_left = end_free - start_free; // 内存池剩余空间大小

		// 如果内存池剩余空间足够，直接返回内存池的首地址
		if (bytes_left >= total_bytes) {
			result = start_free;
			start_free += total_bytes;
			return result;
		}
		// 如果内存池剩余空间不够分配，但足够分配一个区块，直接返回内存池的首地址
		else if (bytes_left >= size) {
			nobjs = bytes_left / size; // 计算能分配多少个区块
			total_bytes = size * nobjs;
			result = start_free;
			start_free += total_bytes;
			return result;
		}
		// 内存池剩余空间不够一个区块
		else {
			if (bytes_left > 0) {
				// 将内存池剩余空间加入到对应的 free-list 中
				FreeList** my_free_list = free_list + FREELIST_INDEX(bytes_left);
				((FreeList*)start_free)->next = *my_free_list;
				*my_free_list = (FreeList*)start_free;
			}

			// malloc 申请 heap 中两倍+额外大小的内存
			size_t bytes_to_get = (total_bytes << 1) + ROUND_UP(heap_size >> 4);
			start_free = (char*)std::malloc(bytes_to_get); // TODO 一级配置器

			// heap 空间不足，malloc 失败
			if (start_free == nullptr) {
				FreeList** my_free_list, * p;
				// 在 free-list 中寻找是否有尚未使用且足够大的区块
				for (size_t i = size; i < _MAX_BYTES; i += _ALIGN) {
					my_free_list = free_list + FREELIST_INDEX(i);
					p = *my_free_list;

					// 尚有未用的区块
					if (p != nullptr) {
						*my_free_list = p->next;
						*start_free = (char*)p;
						*end_free = start_free + i;
						return charn_alloc(size, nobjs);
					}
				}

				// 如果一点内存都没有，只能调用一级配置器申请内存，并且用户没有设置处理例程就抛异常
				std::printf("out of memory\n");
				end_free = nullptr;
				throw std::bad_alloc(); // TODO 一级配置器 及其 handle

			}

			// 分配成功，更新内存池的结束位置,重新调用 chunk_alloc 分配内存
			heap_size += bytes_to_get;
			end_free = start_free + bytes_to_get;
			return chunk_alloc(size, nobjs);

		}
	}

} // namespace tinystl
#endif // TINYSTL_ALLOC_H_