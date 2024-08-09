#include "core/allocator.h"
#include <utility>

namespace infini
{
    Allocator::Allocator(Runtime runtime) : runtime(runtime)
    {
        used = 0;
        peak = 0;
        ptr = nullptr;

        // 'alignment' defaults to sizeof(uint64_t), because it is the length of
        // the longest data type currently supported by the DataType field of
        // the tensor
        alignment = sizeof(uint64_t);
    }

    Allocator::~Allocator()
    {
        if (this->ptr != nullptr)
        {
            runtime->dealloc(this->ptr);
        }
    }

    size_t Allocator::alloc(size_t size)
    {
        IT_ASSERT(this->ptr == nullptr);
        // pad the size to the multiple of alignment
        size = this->getAlignedSize(size);

        // =================================== 作业 ===================================
        // TODO: 设计一个算法来分配内存，返回起始地址偏移量
        // =================================== 作业 ===================================

        bool flag = false;
        size_t ans_addr = -1;

        for(auto it = free_blocks.begin();it != free_blocks.end(); it++) {
            std::cout<<"target size is "<<size<<", free block size is "<<it->second<<std::endl;
            if (it->second >= size) {
                printf("use free blocks\n");
                size_t remain_size = it->second - size;
                ans_addr = it->first;
                if(remain_size == 0) {    
                    free_blocks.erase(it);
                } else if(remain_size > 0) {
                    free_blocks[ans_addr + size] = remain_size;
                    free_blocks.erase(it);
                }
                used += size;
                if(peak < used) {
                    peak = used;
                }

                flag = true;
                break;
            } 
        }
        if(!flag) { //没有合适的内存块

            
            getPtr();
            ans_addr = static_cast<size_t>(reinterpret_cast<uintptr_t>(this->ptr));
            this->ptr = nullptr;
            if(peak > size) {
                free_blocks[ans_addr + size] = peak - size;
            }
            used += size;
            if(peak < used) {
                peak = used;
            }
        }


        return ans_addr;
    }

    void Allocator::free(size_t addr, size_t size)
    {
        IT_ASSERT(this->ptr == nullptr);
        size = getAlignedSize(size);

        // =================================== 作业 ===================================
        // TODO: 设计一个算法来回收内存
        // =================================== 作业 ===================================

        free_blocks[addr] = size;
        std::cout<<"free ["<<addr<<"] "<<size<<std::endl;
        used -= size;

        map<size_t,size_t>::iterator it = free_blocks.find(addr);
        if(it != free_blocks.end()){
            //test if the front one can be merged
            if(it != free_blocks.begin()){
                auto prev = std::prev(it);
                if(prev != free_blocks.begin() && prev->first + prev->second >= it->first) {
                    prev->second += it->second;
                    free_blocks.erase(it);
                    it = prev;
                }
            }

            auto next = std::next(it);
            if(next != free_blocks.end() && it->first + it->second == next->first){
                it->second += next->second;
                free_blocks.erase(next);
            }
        }
         
    }

    void *Allocator::getPtr()
    {
        if (this->ptr == nullptr)
        {
            this->ptr = runtime->alloc(this->peak);
            printf("Allocator really alloc: %p %lu bytes\n", this->ptr, peak);
        }
        return this->ptr;
    }

    size_t Allocator::getAlignedSize(size_t size)
    {
        return ((size - 1) / this->alignment + 1) * this->alignment;
    }

    void Allocator::info()
    {
        std::cout << "Used memory: " << this->used
                  << ", peak memory: " << this->peak << std::endl;
    }
}
