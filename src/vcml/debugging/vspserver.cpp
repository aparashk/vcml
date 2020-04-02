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

#include "vcml/debugging/vspserver.h"

namespace vcml { namespace debugging {

    static vspserver* session = nullptr;

    static void cleanup_session() {
        if (session != nullptr)
            session->cleanup();
    }

    string vspserver::handle_none(const char* command) {
        return "";
    }

    string vspserver::handle_step(const char* command) {
        vector<string> args = split(command, ',');
        sc_time duration, next;
        if (args.size() > 1) { // step duration passed as argument
            stringstream ss(args[1]);
            double t; ss >> t;
            duration = sc_time(t, SC_SEC);
        } else if (sc_get_curr_simcontext()->next_time(next)) { // next event
            duration = next - sc_time_stamp();
        } else { // step entire quantum
            duration = tlm::tlm_global_quantum::instance().get();
        }

        run_interruptible(duration);

        // Make sure we are still connected, since during an IO interrupt
        // we may have disconnected due to a protocol error.
        return is_connected() ? "OK" : "";
    }

    string vspserver::handle_cont(const char* command) {
        run_interruptible(SC_ZERO_TIME);

        // Make sure we are still connected, since during an IO interrupt
        // we may have disconnected due to a protocol error.
        return is_connected() ? "OK" : "";
    }

    string vspserver::handle_info(const char* command) {
        vector<string> args = split(command, ',');

        // If no arguments are given, return the top level objects as children
        // of a virtual "root" node.
        if (args.size() < 2) {
            stringstream info;
            info << "kind:root,";
            for (const sc_object* obj : sc_core::sc_get_top_level_objects())
                info << "child:" << obj->basename() << ",";
            return info.str();
        }

        // First argument holds the object name that was queried.
        string name = args[1];
        sc_object* obj = find_object(name);
        if (obj == nullptr)
            return mkstr("object '%s' not found", name.c_str());

        stringstream info;
        info << "kind:" << obj->kind() << ",";

        // List all child objects
        for (const sc_object* child : obj->get_child_objects())
            info << "child:" << child->basename() << ",";

        // List all attributes
        for (const sc_attr_base* attr : obj->attr_cltn())
            info << "attr:" << attr->name() << ",";
        sc_core::sc_attr_cltn::const_iterator it;

        // List all commands
        component* mod = dynamic_cast<component*>(obj);
        if (mod != nullptr) {
            for (const command_base* cmd : mod->get_commands()) {
                info << "cmd:" << escape(cmd->name(), ",")
                     << ":" << cmd->argc()
                     << ":" << escape(cmd->desc(), ",") << ",";
            }
        }

        return info.str();
    }

    string vspserver::handle_exec(const char* command) {
        vector<string> args = split(command, ',');

        if (args.size() < 3)
            return mkstr("insufficient arguments %d", args.size());

        string name = args[1];
        sc_object* obj = find_object(name);
        if (obj == nullptr)
            return mkstr("object '%s' not found", name.c_str());

        module* mod = dynamic_cast<module*>(obj);
        if (!mod)
            return mkstr("object '%s' does not support commands", name.c_str());

        try {
            stringstream ss;
            vector<string> cmdargs(args.begin() + 3, args.end());
            bool success = mod->execute(args[2], cmdargs, ss);
            string resp = escape(ss.str(), ",");
            return mkstr("%s,%s", success ? "OK" : "ERROR", resp.c_str());
        } catch (std::exception& e) {
            return mkstr("ERROR,%s", escape(e.what(), ",").c_str());
        }
    }

    string vspserver::handle_time(const char* command) {
        u64 delta = sc_delta_count();
        u64 nanos = time_to_ns(sc_time_stamp());
        return mkstr("%lu,%lu", nanos, delta);
    }

    string vspserver::handle_rdgq(const char* command) {
        sc_time quantum = tlm::tlm_global_quantum::instance().get();
        return mkstr("%4.12f", quantum.to_seconds());
    }

    string vspserver::handle_wrgq(const char* command) {
        vector<string> args = split(command, ',');
        if (args.size() < 2)
            return mkstr("insufficient arguments %d", args.size());

        stringstream ss(args[1]);
        double t;
        ss >> t;

        sc_time quantum(t, SC_SEC);
        tlm::tlm_global_quantum::instance().set(quantum);
        return "OK";
    }

    string vspserver::handle_geta(const char* command) {
        vector<string> args = split(command, ',');
        if (args.size() < 2)
            return mkstr("insufficient arguments %d", args.size());

        string name = args[1];
        sc_attr_base* attr = find_attribute(name);
        if (attr == nullptr)
            return mkstr("attribute '%s' not found", name.c_str());

        stringstream ss;
        ss << attr->name();

        property_base* prop = dynamic_cast<property_base*>(attr);
        if (prop) {
            ss << "," << escape(prop->str(), ",");
            ss << "," << prop->size();
            ss << "," << prop->num();
        }

        return ss.str();
    }

    string vspserver::handle_seta(const char* command) {
        vector<string> args = split(command, ',');
        if (args.size() < 3)
            return mkstr("insufficient arguments %d", args.size());

        string name = args[1];
        string value = args[2];

        sc_attr_base* attr = find_attribute(name);
        if (attr == nullptr)
            return mkstr("attribute '%s' not found", name.c_str());

        property_base* prop = dynamic_cast<property_base*>(attr);
        if (prop == nullptr)
            return mkstr("attribute '%s' not writable", name.c_str());

        prop->str(value);
        return "OK";
    }

    string vspserver::handle_quit(const char* command) {
        stop();
        sc_stop();
        return "OK";
    }

    string vspserver::handle_vers(const char* command) {
        stringstream ss;
        ss << SC_VERSION << ",";
        ss << VCML_VERSION_STRING;
#ifdef VCML_DEBUG
        ss << "-debug";
#endif
        return ss.str();
    }

    static void do_interrupt(int fd, int event) {
        VCML_ERROR_ON(session == nullptr, "interrupt on no session");
        session->interrupt();
    }

    void vspserver::run_interruptible(const sc_time& duration) {
        thctl_enter_critical();
        notify_resume(nullptr);
        aio_notify(get_connection_fd(), &do_interrupt, AIO_ONCE);
        if (duration != SC_ZERO_TIME)
            sc_start(duration);
        else
            sc_start();
        aio_cancel(get_connection_fd());
        notify_suspend(nullptr);
        thctl_exit_critical();

        switch (sc_core::sc_get_curr_simcontext()->sim_status()) {
        case sc_core::SC_SIM_ERROR:
            send_packet(ERR_INTERNAL);
            disconnect();
            stop();
            break;
        case sc_core::SC_SIM_USER_STOP:
            send_packet("STOP");
            disconnect();
            stop();
            break;
        case sc_core::SC_SIM_OK:
        default:
            break;
        }
    }

    void vspserver::notify_suspend(sc_object* obj) {
        const auto& children = obj ? obj->get_child_objects()
                                   : sc_core::sc_get_top_level_objects();
        for (auto child : children)
            notify_suspend(child);

        if (obj == nullptr)
            return;

        module* mod = dynamic_cast<module*>(obj);
        if (mod != nullptr)
            mod->session_suspend();
    }

    void vspserver::notify_resume(sc_object* obj) {
        const auto& children = obj ? obj->get_child_objects()
                                   : sc_core::sc_get_top_level_objects();
        for (auto child : children)
            notify_resume(child);

        if (obj == nullptr)
            return;

        module* mod = dynamic_cast<module*>(obj);
        if (mod != nullptr)
            mod->session_resume();
    }

    vspserver::vspserver(u16 port):
        rspserver(port),
        m_announce(tempdir() + mkstr("vcml_session_%d", (int)port)) {
        VCML_ERROR_ON(session != nullptr, "vspserver already created");
        session = this;
        atexit(&cleanup_session);

        using std::placeholders::_1;
        register_handler("n", std::bind(&vspserver::handle_none, this, _1));
        register_handler("s", std::bind(&vspserver::handle_step, this, _1));
        register_handler("c", std::bind(&vspserver::handle_cont, this, _1));
        register_handler("i", std::bind(&vspserver::handle_info, this, _1));
        register_handler("e", std::bind(&vspserver::handle_exec, this, _1));
        register_handler("t", std::bind(&vspserver::handle_time, this, _1));
        register_handler("q", std::bind(&vspserver::handle_rdgq, this, _1));
        register_handler("Q", std::bind(&vspserver::handle_wrgq, this, _1));
        register_handler("a", std::bind(&vspserver::handle_geta, this, _1));
        register_handler("A", std::bind(&vspserver::handle_seta, this, _1));
        register_handler("x", std::bind(&vspserver::handle_quit, this, _1));
        register_handler("v", std::bind(&vspserver::handle_vers, this, _1));
    }

    vspserver::~vspserver() {
        cleanup();
        session = nullptr;
    }

    void vspserver::start() {
        cleanup();

        // Create announce file
        ofstream of(m_announce.c_str());
        of << "localhost:" << std::dec << get_port() << ":" << username()
           << ":" << progname() << std::endl;

        // Finish elaboration first before processing commands
        sc_start(SC_ZERO_TIME);
        log_info("vspserver listening on port %d", (int)get_port());

        thctl_exit_critical();
        run();
    }

    void vspserver::interrupt() {
        try {
            int sig = recv_signal();
            switch (sig) {
            case 0x00: // terminate request
            case  'x':
                sc_stop();
                return;

            case 0x03: // interrupt request
            case  'a':
                sc_pause();
                return;

            case 0x42: // update request
            case  'u':
                send_packet(handle_time(nullptr));
                aio_notify(get_connection_fd(), &do_interrupt, AIO_ONCE);
                return;

            default:
                VCML_ERROR("invalid signal received: 0x%x", sig);
            }

        } catch (...) {
            sc_pause();
            disconnect();
        }
    }

    void vspserver::cleanup() {
        if (!file_exists(m_announce))
            return;

        if (remove(m_announce.c_str())) {
            log_warning("failed to remove file '%s': %s", m_announce.c_str(),
                        strerror(errno));
        }
    }

    void vspserver::handle_connect(const char* peer) {
        log_info("vspserver connected to %s", peer);
    }

    void vspserver::handle_disconnect() {
        log_info("vspserver listening on port %d", (int)get_port());
    }

}}
