/******************************************************************************
 *                                                                            *
 * Copyright 2018 Jan Henrik Weinstock                                        *
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

#ifndef VCML_TYPES_H
#define VCML_TYPES_H

#include "vcml/common/includes.h"

namespace vcml {

    typedef uint8_t  u8;
    typedef uint16_t u16;
    typedef uint32_t u32;
    typedef uint64_t u64;

    typedef int8_t  i8;
    typedef int16_t i16;
    typedef int32_t i32;
    typedef int64_t i64;

    using std::clock_t;

    const clock_t kHz = 1000;
    const clock_t MHz = 1000 * kHz;
    const clock_t GHz = 1000 * MHz;
    const clock_t THz = 1000 * GHz;

    using std::size_t;

    const size_t KiB = 1024;
    const size_t MiB = 1024 * KiB;
    const size_t GiB = 1024 * MiB;
    const size_t TiB = 1024 * GiB;

    using std::min;
    using std::max;

    using std::list;
    using std::queue;
    using std::vector;

    template <typename V, typename T>
    inline void stl_remove_erase(V& v, const T& t) {
        v.erase(std::remove(v.begin(), v.end(), t), v.end());
    }

    template <typename V, typename T>
    inline bool stl_contains(const V& v, const T& t) {
        return std::find(v.begin(), v.end(), t) != v.end();
    }

    template <typename T1, typename T2>
    inline bool stl_contains(const std::map<T1,T2>& m, const T1& t) {
        return m.find(t) != m.end();
    }

    template <typename T1, typename T2>
    inline bool stl_contains(const std::unordered_map<T1,T2>& m, const T1& t) {
        return m.find(t) != m.end();
    }

    template <typename T>
    inline void stl_add_unique(vector<T>& v, const T& t) {
        if (!stl_contains(v, t))
            v.push_back(t);
    }

    using std::ostream;
    using std::fstream;
    using std::ifstream;
    using std::ofstream;

    using std::shared_ptr;
    using std::unique_ptr;

    using std::function;

    using sc_core::sc_object;
    using sc_core::sc_attr_base;

    using sc_core::sc_gen_unique_name;
    using sc_core::SC_HIERARCHY_CHAR;

    using sc_core::sc_delta_count;

    using sc_core::sc_time;
    using sc_core::sc_time_stamp;

    using sc_core::SC_ZERO_TIME;
    using sc_core::SC_SEC;
    using sc_core::SC_MS;
    using sc_core::SC_US;
    using sc_core::SC_NS;
    using sc_core::SC_PS;

    using sc_core::sc_start;
    using sc_core::sc_stop;

#if (SYSTEMC_VERSION < 20120701)
#   define sc_pause sc_stop
#else
    using sc_core::sc_pause;
#endif

    using sc_core::sc_simcontext;
    using sc_core::sc_get_curr_simcontext;

    using sc_core::sc_event;

    using sc_core::sc_process_b;
    using sc_core::sc_get_current_process_b;

    using sc_core::sc_actions;
    using sc_core::sc_report;

    using sc_core::sc_port;
    using sc_core::sc_export;
    using sc_core::sc_signal;
    using sc_core::sc_out;
    using sc_core::sc_in;

    using sc_core::sc_module_name;
    using sc_core::sc_module;

    using sc_core::sc_spawn;
    using sc_core::sc_spawn_options;

    using tlm::tlm_global_quantum;
    using tlm::tlm_initiator_socket;
    using tlm::tlm_target_socket;
    using tlm::tlm_generic_payload;
    using tlm::tlm_dmi;
    using tlm::tlm_extension;
    using tlm::tlm_extension_base;

    using tlm::tlm_command;
    using tlm::TLM_READ_COMMAND;
    using tlm::TLM_WRITE_COMMAND;
    using tlm::TLM_IGNORE_COMMAND;

    using tlm::tlm_response_status;
    using tlm::TLM_OK_RESPONSE;
    using tlm::TLM_INCOMPLETE_RESPONSE;
    using tlm::TLM_GENERIC_ERROR_RESPONSE;
    using tlm::TLM_ADDRESS_ERROR_RESPONSE;
    using tlm::TLM_COMMAND_ERROR_RESPONSE;
    using tlm::TLM_BURST_ERROR_RESPONSE;
    using tlm::TLM_BYTE_ENABLE_ERROR_RESPONSE;

    inline static bool success(tlm_response_status status) {
        return status == TLM_OK_RESPONSE;
    }

    inline static bool failed(tlm_response_status status) {
        return status != TLM_OK_RESPONSE;
    }

    using tlm_utils::simple_initiator_socket;
    using tlm_utils::simple_initiator_socket_tagged;
    using tlm_utils::simple_target_socket;
    using tlm_utils::simple_target_socket_tagged;

    static inline bool is_set(int flags, int set) {
        return (flags & set) == set;
    }

    enum vcml_access {
        VCML_ACCESS_NONE       = 0x0,
        VCML_ACCESS_READ       = 0x1,
        VCML_ACCESS_WRITE      = 0x2,
        VCML_ACCESS_READ_WRITE = VCML_ACCESS_READ | VCML_ACCESS_WRITE
    };

    static inline bool is_read_allowed(int a) {
        return is_set(a, VCML_ACCESS_READ);
    }

    static inline bool is_write_allowed(int a) {
        return is_set(a, VCML_ACCESS_WRITE);
    }

}

#endif
