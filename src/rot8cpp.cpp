#include <fstream>
#include <iostream>
#include <iomanip>
#include <cstdint>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <cmath>
#include <csignal>
#include <cassert>
#include <cstring>
#include <mutex>

#include <fcntl.h>
#include <semaphore.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
const char* semaphoreName = "/rot8cpp_semaphore";
const char* sharedMemoryName = "/rot8cpp_shared_memory";
// Debug

#ifdef DEBUG
template<typename ...Args>
void debug_log(Args && ...args)
{
    (std::cout << ... << args);
}
#define DEBUG_LOG(...) debug_log(__VA_ARGS__)
// #define DEBUG_LOG(...) std::cout << __VA_ARGS__ << std::endl;
#else
#define DEBUG_LOG(...)
#endif


// Signal related

#include <unistd.h>
bool isPiped (FILE* fd){
  return isatty(fileno(fd)) == 1;
}

struct Global {
  bool running;
  uint8_t index;
  bool istty;
  bool debug;
} Global;
struct Global init_g = { true, 0, false, false };

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
void sig_rtmin_handler(int signum, siginfo_t *siginfo, void *context)
{
  int shm_fd = shm_open(sharedMemoryName, O_RDWR, 0644);
  struct Global* g = (struct Global*)mmap(NULL, sizeof(struct Global), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (g == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return;
  }

  if(siginfo->si_value.sival_int == 0){
    g->running = !g->running;
    DEBUG_LOG("RTMIN Recieved\n");
  }

  munmap(g, sizeof(struct Global));
  close(shm_fd);
  /* switch (siginfo -> si_value.sival_int) {
    case:
    default:
  } */
};
#pragma GCC diagnostic pop



void signalHandler(int signum) {
  int shm_fd = shm_open(sharedMemoryName, O_RDWR, 0644);
  struct Global* g = (struct Global*)mmap(NULL, sizeof(struct Global), PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (g == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return;
  }

  if(signum == SIGUSR1){
    g->running = false;
    DEBUG_LOG("Stop Signal\n");
  }
  else if (signum == SIGUSR2) {
    g->running = true;
    DEBUG_LOG("Start Signal\n");
  }
  else {

  }
  munmap(g, sizeof(struct Global));
  close(shm_fd);
}


sem_t* semaphore;
std::mutex sharedMemoryMutex;
int shm_fd;
void cleanUp(int signum) {
  if(signum == SIGINT){
    sem_close(semaphore);
    sem_unlink(semaphoreName);

    // munmap(g, sizeof(struct Global));
    close(shm_fd);
    shm_unlink(sharedMemoryName);
    // std::cerr << "Cleanup complete. Exiting." << std::endl;
    // DEBUG_LOG("SIGINT\n");
    exit(EXIT_FAILURE);
  }
  else {

  }
}

// Application

#define U8_MAX ((unsigned int)((uint8_t)-1))

#define YELLOW "\033[33m"
#define GREEN "\033[32m"
#define RED "\033[31m"
#define NONE "\033[0m"

#define POLL_TIME 500
#define POS_POLL_TIME POLL_TIME
#define ROT_POLL_TIME POLL_TIME


#define DEVICES  "/sys/bus/iio/devices"
#define DEVTYPE  "iio:device"
#define ACCELREG1 "in_accel"
#define ACCELREG2 "_raw"

enum Rotation {
  NORMAL_0,
  NORMAL_90,
  NORMAL_270,
  NORMAL_180,
  FLIPPED_0,
  FLIPPED_90,
  FLIPPED_270,
  FLIPPED_180
};

struct Position {
  float x;
  float y;
  float z;
};

struct Orientation {
  Rotation r;
  Position& p;
};

/* Rotation positionToRotation(const Position& p1){

} */

struct Device {
  std::string name;
  std::vector<std::string> devs;
};


/*
          if(e2.path().string().find("x") != std::string::npos){
            s >> p.x;
          }
          else if(e2.path().string().find("y") != std::string::npos){
            s >> p.y;
          }
          else if(e2.path().string().find("z") != std::string::npos){
            s >> p.z;
          } */
Device getRotDevice(const std::filesystem::path& p){
  Device d;
  for (const auto& files : std::filesystem::directory_iterator(p)) {
    if(
        files.path().string().find(ACCELREG1) != std::string::npos &&
        files.path().string().find(ACCELREG2) != std::string::npos
      ){
      if (d.name == "") {
        std::ifstream s(p / "name");
        d.name = p.filename();
        s >> d.name;
      }
      d.devs.emplace_back(files.path().string());
    }
  }
  // Sort Devices Alphabetically
  std::sort(d.devs.begin(), d.devs.end(), [](std::string a, std::string b){
    return a<b;
  });
  return d;
}

void getRotDevices(std::vector<Device>& dev){
  // uint8_t i = 0;
  for (const auto& devices : std::filesystem::directory_iterator(DEVICES)) {
    if(devices.path().string().find(DEVTYPE) != std::string::npos){
      Device d = getRotDevice(devices.path());
      if (d.name != "") dev.emplace_back(d);
    }
  }
  std::sort(dev.begin(), dev.end(), [](Device a, Device b){
    return a.name<b.name;
  });
}

void getPosition(Position& p, std::vector<Device> dev, struct Global *g) {
  uint8_t i = 0;
  float *pos = reinterpret_cast<float*>(&p);
  for (const auto& d : dev[g->index].devs) {
    // std::cout << "Getting Position of: " << dev[g.index].name << std::endl;
    std::ifstream s(d);
    s >> pos[i];
    pos[i] = pos[i]/1'000'000; // 1e6
    i++;
    if (i > 2)
      break;
  }
}

void getPosT (Position& p){

  int shm_fd = shm_open(sharedMemoryName, O_RDONLY, 0644);
  struct Global* g = (struct Global*)mmap(NULL, sizeof(struct Global), PROT_READ, MAP_SHARED, shm_fd, 0);
  if (g == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return;
  }

  std::vector<Device> dev;

  getRotDevices(dev);

  while(true){
    // Only Run if allowed
    {
      std::lock_guard<std::mutex> lock(sharedMemoryMutex);
      if(g->running) {
        getPosition(p, dev, g);
      }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(POS_POLL_TIME));
  }
}

void printPosT (Position& p){

  int shm_fd = shm_open(sharedMemoryName, O_RDONLY, 0644);
  struct Global* g = (struct Global*)mmap(NULL, sizeof(struct Global), PROT_READ, MAP_SHARED, shm_fd, 0);
  if (g == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return;
  }

  Orientation current = { NORMAL_0, p };

  int rotatetdir = 0;
  int oldrotatetdir = 0;

#ifdef DEBUG
  const int8_t invalidx = 0;
  const int8_t invalidy = 1;
#endif // DEBUG

  while(true){
    // Only Run if allowed
    {
      std::lock_guard<std::mutex> lock(sharedMemoryMutex);
    if(g->running) {
      float pseudox = std::round(current.p.x);
      float pseudoy = std::round(current.p.y);
      // if (pseudox != invalidx || pseudoy != invalidy) rotatetdir = 90 * pseudox + (current.p.y > 0.8 ? 180 : 0);
      if (pseudox+pseudoy != 0 || pseudoy+pseudox != 2) rotatetdir = static_cast<int>(90 * pseudox + (current.p.y > 0.8 ? 180 : 0));
      // if (pseudox+pseudoy != 0 || pseudoy+pseudox != 2) rotatetdir = static_cast<int>(90 * pseudox + (current.p.y > 0.8 ? 180 : 0));
#ifdef DEBUG
      if (g->debug){
        if (g->istty) DEBUG_LOG(pseudox == invalidx ? RED : NONE);
        DEBUG_LOG(current.p.x);
        if (g->istty) DEBUG_LOG(NONE);
        DEBUG_LOG(", ");
        if (g->istty) DEBUG_LOG(pseudoy == invalidy ? RED : NONE);
        DEBUG_LOG(current.p.y);
        if (g->istty) DEBUG_LOG(NONE);
        DEBUG_LOG(", ", current.p.z, "\n");
      }
#endif // DEBUG
    }

      if (rotatetdir != oldrotatetdir) std::cout << rotatetdir << std::endl;
      oldrotatetdir = rotatetdir;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(ROT_POLL_TIME));
  }
}


bool positionMatch(const Position& p1, const Position& p2){
  return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

void help(){
  int w = 15;
  std::cout << std::left << std::setw(w) << "usage: rot8cpp" << "[-h | --help] [--listDevIndex]" << std::endl;
  std::cout << std::left << std::setw(w) << " "              << "[--devIndex <index>] [--paused]" << std::endl;
#ifdef DEBUG
  std::cout << std::left << std::setw(w) << " "              << "[--debug]" << std::endl;
#endif // DEBUG

  std::cout << std::endl << "HELP Options"    << std::endl;
  std::cout << "  " << std::left << std::setw(w) << "-h | --help"    << "View this page" << std::endl;
  std::cout << "  " << std::left << std::setw(w) << "--listDevIndex" << "List Devices with index, that are available to use for this daemon" << std::endl;

  std::cout << std::endl  << "RUN Options"    << std::endl;
  std::cout << "  " << std::left << std::setw(w) << "--devIndex"     << "Select Index of the Device you want to use for the daemon" << std::endl;
  std::cout << "  " << std::left << std::setw(w) << std::left << std::setw(w) << "--paused"       << "Starts in a paused state" << std::endl;
#ifdef DEBUG
  std::cout << "  " << std::left << std::setw(w) << std::left << std::setw(w) << "--debug"       << "Starts in a debug state" << std::endl;
#endif // DEBUG
}

int main (int argc, char* argv[]) {

  // Semaphore
  semaphore = sem_open(semaphoreName, O_CREAT | O_EXCL, 0644, 1);
  const bool realProg = (semaphore != SEM_FAILED);

  // Shared Memory
  if(realProg){
    shm_fd = shm_open(sharedMemoryName, O_CREAT | O_RDWR, 0644);
    ftruncate(shm_fd, sizeof(struct Global));
  } else {
    shm_fd = shm_open(sharedMemoryName, O_RDWR, 0644);
  }
  struct Global* g = (struct Global*)mmap(NULL, sizeof(struct Global), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (g == MAP_FAILED) {
    perror("mmap");
    close(shm_fd);
    return EXIT_FAILURE;
  }

  if(realProg){
    *g = init_g;
    g->istty = isPiped(stdout);
  }

  for(int i = 1; i < argc; i++){
    if(realProg){
      if(strcmp(argv[i], "--devIndex") == 0){
        assert(strlen(argv[i]) > 3);
        unsigned int x = static_cast<unsigned int>(atoi(argv[i]));
        assert(x < U8_MAX+1);
        i++;
        g->index = static_cast<uint8_t>(x);
      }
      else if(strcmp(argv[i], "--listDevIndex") == 0){
        std::vector<Device> dev;
        getRotDevices(dev);
        int k = 0;
        for(auto & d : dev){
          std::cout << k++ << ": " << d.name << std::endl;
        }
        return 0;
      }
#ifdef DEBUG
      else if(strcmp(argv[i], "--debug") == 0){
        g->debug = true;
      }
#endif // DEBUG
      else if(strcmp(argv[i], "--paused") == 0){
        g->running = false;
      }
      else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0){
        help();
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        shm_unlink(sharedMemoryName);
        sem_unlink(semaphoreName);
        return 0;
      }
      else{
        help();
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        shm_unlink(sharedMemoryName);
        sem_unlink(semaphoreName);
        return 1;
      }
    }
    else {
      if(strcmp(argv[i], "--cleanup") == 0){
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        shm_unlink(sharedMemoryName);
        sem_unlink(semaphoreName);
        std::cerr << "Unlinked Semaphore" << std::endl;
        return 0;
      }
      else if(strcmp(argv[i], "--set") == 0 && argc == i+2){
        if(strcmp(argv[i+1], "toggle") == 0){
          g->running = !g->running;
        } else {
          g->running = strcmp(argv[i+1], "lock") != 0;
        }
        // std::cout << std::boolalpha << realProg;
        if(isPiped(stdout)) {
          std::cout << "State:";
        }
        std::cout << g->running << std::endl;
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        return 0;
      }
      else if(strcmp(argv[i], "--watch") == 0){
        bool piped = isPiped(stdout);
        bool state = g->running;
        if(argc == i+2 && strcmp(argv[i+1], "-1") == 0){
            if(piped) {
              std::cout << "State:";
            }
            std::cout << state << std::endl;
        }
        while(true){
          if(state != g->running){
            if(piped) {
              std::cout << "State:";
            }
            std::cout << g->running << std::endl;
            state = g->running;
          }
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        // shm_unlink(sharedMemoryName);
        // sem_unlink(semaphoreName);
        return 0;
      }
      else if(strcmp(argv[i], "--state") == 0){
        if(isPiped(stdout)) {
          std::cout << "State:";
        }
        std::cout << g->running << std::endl;
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        // shm_unlink(sharedMemoryName);
        // sem_unlink(semaphoreName);
        return 0;
      }
      else{
        help();
        munmap(g, sizeof(struct Global));
        close(shm_fd);
        shm_unlink(sharedMemoryName);
        sem_unlink(semaphoreName);
        return 1;
      }
    }
  }

  munmap(g, sizeof(struct Global));
  close(shm_fd);

  if (realProg) {
    signal(SIGINT, cleanUp);
    signal(SIGUSR1, signalHandler);
    signal(SIGUSR2, signalHandler);
    struct sigaction act;
    memset (&act, 0, sizeof (act));
    // set signal handler for SIGRTMIN
    act.sa_sigaction = sig_rtmin_handler;
    act.sa_flags = SA_SIGINFO;
    if (sigaction (SIGRTMIN, &act, NULL) == -1)
        std::cerr << "sigaction" << std::endl;

    Position pos = {0, 0, 0};

    std::thread t1(getPosT, std::ref(pos));
    std::thread t2(printPosT, std::ref(pos));

    t1.join();
    t2.join();

    sem_close(semaphore);
    sem_unlink(semaphoreName);
  } else {
    std::cerr << "Another instance is already running." << std::endl;
  }

  if (realProg) shm_unlink(sharedMemoryName);
  return 0;

}

/* 0 normal -1 normal 0

-1 right  0 normal 90

1  left   0 normal -90

0         1 normal 180

-1        1        270

1         1        90 */
