#include <future>
#include "Log.hpp"
#include "Service.hpp"
#include "sources/MP3.hpp"

// Path to log file
#define LOG_FILE "/switch/TriPlayer/sysmodule.log"

// Heap size:
// DB: ~0.5MB
// Queue: ~0.2MB
// MP3: ~0.5MB
// Sockets: ~0.5MB
#define INNER_HEAP_SIZE (size_t)(2 * 1024 * 1024)

// It hangs if I don't use C... I wish I knew why!
extern "C" {
    u32 __nx_applet_type = AppletType_None;
    u32 __nx_fs_num_sessions = 1;

    size_t nx_inner_heap_size = INNER_HEAP_SIZE;
    char   nx_inner_heap[INNER_HEAP_SIZE];

    void __libnx_initheap(void);
    void __appInit(void);
    void __appExit(void);
}

void __libnx_initheap(void) {
    void*  addr = nx_inner_heap;
    size_t size = nx_inner_heap_size;

    // Newlib
    extern char* fake_heap_start;
    extern char* fake_heap_end;

    fake_heap_start = (char*)addr;
    fake_heap_end   = (char*)addr + size;
}

// Init services on start
void __appInit(void) {
    if (R_FAILED(smInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_SM));
    }

    // FS + Log
    if (R_FAILED(fsInitialize())) {
        fatalThrow(MAKERESULT(Module_Libnx, LibnxError_InitFail_FS));
    }
    fsdevMountSdmc();

    // Open the log file, defaulting to Warning level
    Log::openFile(LOG_FILE, Log::Level::Warning);

    // Sockets use small buffers
    constexpr SocketInitConfig sockCfg = {
        .bsdsockets_version = 1,

        .tcp_tx_buf_size = 0x1000,
        .tcp_rx_buf_size = 0x1000,
        .tcp_tx_buf_max_size = 0x3000,
        .tcp_rx_buf_max_size = 0x3000,

        .udp_tx_buf_size = 0x0,
        .udp_rx_buf_size = 0x0,

        .sb_efficiency = 1,
    };
    if (R_FAILED(socketInitialize(&sockCfg))) {
        Log::writeError("[SOCKET] Failed to initialize sockets!");
    }

    // Audio
    audrenInitialize(&audrenCfg);
    Audio::getInstance();
    audrenStartAudioRenderer();
    MP3::initLib();
}

// Close services on quit
void __appExit(void) {
    // In reverse order

    // Audio
    audrenStopAudioRenderer();
    delete Audio::getInstance();
    audrenExit();

    // Socket
    socketExit();

    // FS
    fsdevUnmountAll();
    fsExit();
    smExit();
}

int main(int argc, char * argv[]) {
    // Create Service
    MainService * s = new MainService();

    // Start audio thread
    std::future<void> audioThread = std::async(std::launch::async, &Audio::process, Audio::getInstance());
    // Start decoding thread
    std::future<void> playbackThread = std::async(std::launch::async, &MainService::playbackThread, s);

    // This thread is responsible for handling communication
    s->socketThread();

    // Join threads (only run after service has exit signal)
    Audio::getInstance()->exit();
    audioThread.get();
    playbackThread.get();

    // Now that it's done we can delete!
    delete s;

    return 0;
}