/******************************************************************************
 *                                                                            *
 * Copyright 2020 Jan Henrik Weinstock                                        *
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

#ifndef VCML_SETUP_H
#define VCML_SETUP_H

#include "vcml/common/includes.h"
#include "vcml/common/types.h"
#include "vcml/common/utils.h"
#include "vcml/common/report.h"

#include "vcml/logging/logger.h"
#include "vcml/logging/log_term.h"
#include "vcml/logging/log_file.h"
#include "vcml/logging/log_stream.h"

#include "vcml/properties/property.h"
#include "vcml/properties/property_provider.h"
#include "vcml/properties/property_provider_arg.h"
#include "vcml/properties/property_provider_env.h"
#include "vcml/properties/property_provider_file.h"

namespace vcml {

    class setup
    {
    private:
        bool m_no_log;
        bool m_log_debug;

        vector<string>  m_args;
        vector<string>  m_log_files;
        vector<string>  m_trace_files;
        vector<string>  m_config_files;

        vector<logger*> m_loggers;
        vector<property_provider*> m_providers;

        bool parse_command_line(int argc, char** argv);

        static setup* s_instance;

    public:
        setup() = delete;
        setup(int argc, char** argv);
        virtual ~setup();

        bool no_log() const { return m_no_log; }
        bool log_debug() const { return m_log_debug; }

        const vector<string>& log_files() const { return m_log_files; }
        const vector<string>& trace_files() const { return m_trace_files; }
        const vector<string>& config_files() const { return m_config_files; }

        unsigned int argc() const { return m_args.size(); }
        const vector<string>& argv() const { return m_args; }

        static setup& instance();
    };

    int main(int argc, char** argv);

}

#endif
