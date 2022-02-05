#pragma once


#include <forward_list>
#include <memory>



namespace nonstd {


    template<typename T>
    class memory_pool {
    public:
        using size_type = std::size_t;
        static constexpr auto default_page_size = 512u;

    private:

        union block {
            alignas(T) char space[sizeof(T)];
            block* next;
        };

        block* head_{nullptr};
        size_type page_size_{default_page_size};
        std::forward_list<std::unique_ptr<block[]>> pages_;

    public:

        memory_pool(size_type page_size = default_page_size) noexcept: page_size_{page_size} { }
        memory_pool(memory_pool const&) = delete;
        memory_pool& operator = (memory_pool const&) = delete;

        memory_pool(memory_pool&& other) noexcept:
            head_{other.head_}, page_size_{other.page_size_}, pages_{std::move(other.pages_)} {
            other.head_ = nullptr;
        }

        memory_pool& operator = (memory_pool&& other) noexcept {
            head_ = other.head_; other.head_ = nullptr;
            page_size_ = other.page_size_;
            pages_ = std::move(other.pages_);
            return *this;
        }

        ~memory_pool() {
            auto* each_block = head_;
            // construct all free blocks in current page
            while(each_block != nullptr) {
                auto* next_block = each_block->next;
                new(each_block->space) T;
                each_block = next_block;
            }
            // destruct all pages
            for(auto& each_page: pages_) {
                auto* begin = each_page.get();
                auto* end = each_page.get() + page_size_;
                for(auto* each_block = begin; each_block != end; ++each_block)
                    reinterpret_cast<T*>(each_block->space)->~T();
            }
        }

        template<typename... Types>
        T* create(Types&&... arguments) {
            return new(allocate()) T(std::forward<Types>(arguments)...);
        }

        void destroy(T* object) noexcept {
            object->~T();
            free(reinterpret_cast<block*>(object));
        }

    private:

        char* allocate() {
            if(!head_)
                add_page();
            char* block = head_->space;
            head_ = head_->next;
            return block;
        }

        void free(block* block) noexcept {
            block->next = head_;
            head_ = block;
        }

        void add_page() {
            auto new_page = std::make_unique<block[]>(page_size_);
            auto* begin = new_page.get();
            auto* end = new_page.get() + page_size_ - 1;
            for(auto* each_block = begin; each_block != end; ++each_block) {
                each_block->next = (each_block + 1);
            }
            end->next = nullptr;
            head_ = begin;
            pages_.emplace_front(std::move(new_page));
        }

    }; // memory_pool

} // namespace nonstd
