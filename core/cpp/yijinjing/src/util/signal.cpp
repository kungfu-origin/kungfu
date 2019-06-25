//
// Created by Keren Dong on 2019-06-10.
//

#include <signal.h>
#include <spdlog/spdlog.h>

#include <kungfu/yijinjing/util/os.h>
#include <kungfu/practice/hero.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace os
        {
            static practice::hero * hero_instance;

            void stop_hero()
            {
                if (hero_instance != nullptr)
                {
                    hero_instance->signal_stop();
                }
            }

            void kf_os_signal_handler(int signum)
            {
                switch (signum)
                {
#ifdef _WINDOWS
                    case SIGINT:          // interrupt
                    case SIGBREAK:        // Ctrl-Break sequence
                        SPDLOG_INFO("kungfu app interrupted");
                        exit(signum);
                        break;
                    case SIGTERM:         // Software termination signal from kill
                        SPDLOG_INFO("kungfu app killed");
                        exit(signum);
                        break;
                    case SIGILL:          // illegal instruction - invalid function image
                    case SIGFPE:          // floating point exception
                    case SIGSEGV:         // segment violation
                    case SIGABRT:         // abnormal termination triggered by abort call
                    case SIGABRT_COMPAT:  // SIGABRT compatible with other platforms, same as SIGABRT
                        SPDLOG_CRITICAL("kungfu app stopped by signal {}", signum);
                        exit(signum);
                        break;
#else
                    case SIGURG:       // discard signal       urgent condition present on socket
                    case SIGCONT:      // discard signal       continue after stop
                    case SIGCHLD:      // discard signal       child status has changed
                    case SIGIO:        // discard signal       I/O is possible on a descriptor (see fcntl(2))
                    case SIGWINCH:     // discard signal       Window size change
                    case SIGINFO:      // discard signal       status request from keyboard
                        SPDLOG_INFO("kungfu app discard signal {}", signum);
                        break;
                    case SIGSTOP:      // stop process         stop (cannot be caught or ignored)
                    case SIGTSTP:      // stop process         stop signal generated from keyboard
                    case SIGTTIN:      // stop process         background read attempted from control terminal
                    case SIGTTOU:      // stop process         background write attempted to control terminal
                        SPDLOG_CRITICAL("kungfu app stopped by signal {}", signum);
                        exit(signum);
                    case SIGINT:       // terminate process    interrupt program
                        SPDLOG_INFO("kungfu app interrupted");
                        stop_hero();
                        break;
                    case SIGTERM:      // terminate process    software termination signal
                        SPDLOG_INFO("kungfu app interrupted");
                        stop_hero();
                        break;
                    case SIGKILL:      // terminate process    kill program
                        SPDLOG_INFO("kungfu app killed");
                        exit(signum);
                    case SIGHUP:       // terminate process    terminal line hangup
                    case SIGPIPE:      // terminate process    write on a pipe with no reader
                    case SIGALRM:      // terminate process    real-time timer expired
                    case SIGXCPU:      // terminate process    cpu time limit exceeded (see setrlimit(2))
                    case SIGXFSZ:      // terminate process    file size limit exceeded (see setrlimit(2))
                    case SIGVTALRM:    // terminate process    virtual time alarm (see setitimer(2))
                    case SIGPROF:      // terminate process    profiling timer alarm (see setitimer(2))
                        SPDLOG_CRITICAL("kungfu app terminated by signal {}", signum);
                        exit(signum);
                    case SIGUSR1:      // terminate process    User defined signal 1
                    case SIGUSR2:      // terminate process    User defined signal 2
                        SPDLOG_CRITICAL("kungfu app caught user defined signal {}", signum);
                        exit(signum);
                    case SIGQUIT:      // create core image    quit program
                    case SIGILL:       // create core image    illegal instruction
                    case SIGTRAP:      // create core image    trace trap
                    case SIGABRT:      // create core image    abort program (formerly SIGIOT)
                    case SIGEMT:       // create core image    emulate instruction executed
                    case SIGFPE:       // create core image    floating-point exception
                    case SIGBUS:       // create core image    bus error
                        SPDLOG_CRITICAL("bus error");
                        exit(signum);
                    case SIGSEGV:      // create core image    segmentation violation
                        SPDLOG_CRITICAL("segmentation violation");
                        exit(signum);
                    case SIGSYS:       // create core image    non-existent system call invoked
                        SPDLOG_CRITICAL("kungfu app caught unexpected signal {}", signum);
                        exit(signum);
#endif // _WINDOWS
                    default:
                        SPDLOG_INFO("kungfu app caught unknown signal {}, signal ignored", signum);
                }
            }

            void handle_os_signals(void *hero)
            {
                if (hero_instance != nullptr)
                {
                    throw yijinjing_error("kungfu can only have one hero instance per process");
                }

                hero_instance = static_cast<practice::hero *>(hero);

                for (int s = 1; s < NSIG; s++)
                {
                    signal(s, kf_os_signal_handler);
                }
            }
        }
    }
}

