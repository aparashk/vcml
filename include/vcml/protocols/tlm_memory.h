/******************************************************************************
 *                                                                            *
 * Copyright 2021 Jan Henrik Weinstock                                        *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the "License");            *
 * you may not use this file except in compliance with the License.           *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 *     http://www.apache.org/licenses/LICENSE-2.0                             *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an "AS IS" BASIS,          *
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 *                                                                            *
 ******************************************************************************/

#ifndef VCML_PROTOCOLS_TLM_MEMORY_H
#define VCML_PROTOCOLS_TLM_MEMORY_H

#include "vcml/common/types.h"
#include "vcml/common/report.h"
#include "vcml/common/range.h"
#include "vcml/common/systemc.h"

#include "vcml/protocols/tlm_sbi.h"

namespace vcml {

    class tlm_memory : public tlm_dmi
    {
    private:
        void*  m_base;
        u8*    m_data;
        size_t m_size;
        size_t m_real_size;
        bool   m_discard_writes;

    public:
        u8*    data() const { return m_data; }
        size_t size() const { return m_size; }

        void allow_read_only()  { allow_read(); }
        void allow_write_only() { allow_write(); }

        void discard_writes(bool set = true) { m_discard_writes = set; }

        tlm_memory();
        tlm_memory(size_t size, alignment al = VCML_ALIGN_NONE);
        tlm_memory(const tlm_memory&) = delete;
        tlm_memory(tlm_memory&& other);
        virtual ~tlm_memory();

        void init(size_t size, alignment al);
        void free();

        tlm_response_status
        read(const range& addr, void* dest, bool debug = false);

        tlm_response_status
        write(const range& addr, const void* dest, bool debug = false);

        void transport(tlm_generic_payload& tx, const tlm_sbi& sbi);

        u8  operator [] (size_t offset) const;
        u8& operator [] (size_t offset);
    };

    inline u8 tlm_memory::operator [] (size_t offset) const {
        VCML_ERROR_ON(m_data == nullptr, "memory not initialized");
        VCML_ERROR_ON(offset >= m_size, "offset out of bounds: %zu", offset);
        return m_data[offset];
    }

    inline u8& tlm_memory::operator [] (size_t offset) {
        VCML_ERROR_ON(m_data == nullptr, "memory not initialized");
        VCML_ERROR_ON(offset >= m_size, "offset out of bounds: %zu", offset);
        return m_data[offset];
    }

}

#endif
