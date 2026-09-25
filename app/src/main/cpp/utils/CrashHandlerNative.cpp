#include "CrashHandlerNative.h"
#include <signal.h>
#include <ucontext.h>
#include <unwind.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <android/log.h>
#include <sstream>
#include <iomanip>
#include <vector>
#include <ctime>
#include <cstring>
#include <cstdlib>

#define LOG_TAG "TextDraw_CrashNative"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

static std::string s_package_name = "com.textdraw.editor";
static std::string s_files_dir = "";

struct BacktraceState {
    void** current;
    void** end;
};

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context* context, void* arg) {
    BacktraceState* state = static_cast<BacktraceState*>(arg);
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc) {
        if (state->current == state->end) {
            return _URC_END_OF_STACK;
        }
        *state->current++ = reinterpret_cast<void*>(pc);
    }
    return _URC_NO_REASON;
}

static size_t capture_backtrace(void** buffer, size_t max) {
    BacktraceState state = {buffer, buffer + max};
    _Unwind_Backtrace(unwind_callback, &state);
    return state.current - buffer;
}

static void native_signal_handler(int sig, siginfo_t* info, void* context) {
    const char* sig_name = "UNKNOWN_SIGNAL";
    switch (sig) {
        case SIGSEGV: sig_name = "SIGSEGV (Segmentation Fault - Invalid Memory Reference)"; break;
        case SIGABRT: sig_name = "SIGABRT (Abort Signal)"; break;
        case SIGBUS:  sig_name = "SIGBUS (Bus Error - Alignment or I/O)"; break;
        case SIGFPE:  sig_name = "SIGFPE (Arithmetic Exception)"; break;
        case SIGILL:  sig_name = "SIGILL (Illegal Instruction)"; break;
    }

    std::ostringstream ss;
    ss << "==================================================\n";
    ss << "   SA-MP TEXTDRAW EDITOR NATIVE C++ CRASH REPORT  \n";
    ss << "==================================================\n";

    time_t now = time(nullptr);
    char time_str[64];
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&now));
    ss << "Waktu Crash   : " << time_str << "\n";
    ss << "Native Signal : " << sig_name << " (" << sig << ")\n";
    ss << "Signal Code   : " << (info ? info->si_code : 0) << "\n";
    ss << "Fault Address : 0x" << std::hex << (info ? (uintptr_t)info->si_addr : 0) << std::dec << "\n";
    ss << "PID / TID     : " << getpid() << " / " << gettid() << "\n";
    ss << "--------------------------------------------------\n";
    ss << "NATIVE CALLSTACK:\n";

    const size_t max_frames = 30;
    void* buffer[max_frames];
    size_t count = capture_backtrace(buffer, max_frames);

    for (size_t i = 0; i < count; ++i) {
        void* addr = buffer[i];
        Dl_info dlinfo;
        if (dladdr(addr, &dlinfo) && dlinfo.dli_fname) {
            const char* sname = dlinfo.dli_sname ? dlinfo.dli_sname : "(unknown)";
            uintptr_t offset = (uintptr_t)addr - (uintptr_t)dlinfo.dli_fbase;
            ss << "#" << std::setw(2) << std::setfill('0') << i << " pc 0x"
               << std::hex << offset << "  " << dlinfo.dli_fname << " (" << sname << ")\n";
        } else {
            ss << "#" << std::setw(2) << std::setfill('0') << i << " pc " << addr << "\n";
        }
    }
    ss << "==================================================\n";

    std::string report = ss.str();
    LOGE("%s", report.c_str());

    // Save report to disk
    if (!s_files_dir.empty()) {
        std::string crash_dir = s_files_dir + "/crash_logs";
        mkdir(crash_dir.c_str(), 0777);
        std::string crash_file = crash_dir + "/last_crash.txt";
        
        int fd = open(crash_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0666);
        if (fd >= 0) {
            write(fd, report.c_str(), report.size());
            close(fd);
        }

        // Fork child process to start CrashActivity via Android Activity Manager
        pid_t pid = fork();
        if (pid == 0) {
            std::string comp = s_package_name + "/com.textdraw.editor.CrashActivity";
            execl("/system/bin/am", "am", "start", "-n", comp.c_str(), "--es", "extra_log_path", crash_file.c_str(), nullptr);
            _exit(0);
        } else if (pid > 0) {
            usleep(800000); // 800ms to allow CrashActivity to display
        }
    }

    _exit(sig);
}

namespace CrashHandlerNative {

void init(const std::string& package_name, const std::string& files_dir) {
    s_package_name = package_name;
    s_files_dir = files_dir;

    // Setup alternate stack for signal handler (handles stack overflow crashes)
    stack_t ss;
    ss.ss_sp = malloc(SIGSTKSZ);
    ss.ss_size = SIGSTKSZ;
    ss.ss_flags = 0;
    sigaltstack(&ss, nullptr);

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_sigaction = native_signal_handler;
    sa.sa_flags = SA_SIGINFO | SA_ONSTACK;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
    sigaction(SIGILL,  &sa, nullptr);

    LOGE("Native crash signal handlers installed successfully.");
}

} // namespace CrashHandlerNative
