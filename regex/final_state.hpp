#pragma once

#include <algorithm>
#include <any>
#include <cstdint>
#include <optional>
#include <set>

namespace regex {

    // NFA与DFA的公共终态
    class final_state {
       public:
        using id_t       = std::uint32_t;
        using closure    = std::set<id_t>;
        using metadata_t = std::any;

        explicit final_state(id_t id): id(id)
        {
        }

        explicit final_state(id_t id, metadata_t metadata): id(id), metadata(metadata)
        {
        }

        operator id_t(void) const
        {
            return this->id;
        }

        // 用于std::set比较的运算符
        bool operator<(const final_state& other) const
        {
            return this->id < other.id;
        }

        void set_metadata(const metadata_t& metadata)
        {
            this->metadata = metadata;
        }

        void set_metadata(metadata_t&& metadata)
        {
            this->metadata = std::move(metadata);
        }

        metadata_t get_metadata(void) const
        {
            return this->metadata;
        }

       private:
        id_t id;
        metadata_t metadata;
    };

} // namespace regex
