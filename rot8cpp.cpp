#include <fstream>
#include <iostream>
#include <cstdint>
#include <thread>
#include <chrono>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <cmath>

#include <mutex>

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
      if (d.name == "") d.name = p.filename();
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
  uint8_t i = 0;
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

void getPosition(Position& p, std::vector<Device> dev ) {
  uint8_t i = 0;
  float *pos = (float*)&p;
  for (const auto& d : dev[0].devs) {
    // std::cout << "Getting Position of: " << dev[0].name << std::endl;
    std::ifstream s(d);
    s >> pos[i];
    pos[i] = pos[i]/1e6;
    i++;
    if (i > 2)
      break;
  }
}

void getPosT (Position& p){

  std::vector<Device> dev;

  getRotDevices(dev);

  while(true){
    getPosition(p, dev);
    std::this_thread::sleep_for(std::chrono::milliseconds(POS_POLL_TIME));
  }
}

void printPosT (Position& p){

  Orientation current = { NORMAL_0, p };

  int rotatetdir=0;
  int oldrotatetdir=0;
  while(true){
    // std::cout << current.p.x << ", " << current.p.y << ", " << current.p.z << std::endl;
    float pseudox = std::round(current.p.x);
    float pseudoy = std::round(current.p.y);
    if (pseudox != 0 || pseudoy != 0) rotatetdir = 90 * pseudox + (current.p.y > 0.8 ? 180 : 0);

    // std::cout << "Rotationdir: " << rotatetdir << std::endl;
    if (rotatetdir != oldrotatetdir) std::cout << rotatetdir << std::endl;
    oldrotatetdir = rotatetdir;
    std::this_thread::sleep_for(std::chrono::milliseconds(ROT_POLL_TIME));
  }
}


bool positionMatch(const Position& p1, const Position& p2){
  return p1.x == p2.x && p1.y == p2.y && p1.z == p2.z;
}

int main (int argc, char* argv[]) {

  Position pos = {0, 0, 0};

  std::thread t1(getPosT, std::ref(pos));
  std::thread t2(printPosT, std::ref(pos));

  t1.join();
  t2.join();

  return 0;

}

/* 0 normal -1 normal 0

-1 right  0 normal 90

1  left   0 normal -90

0         1 normal 180

-1        1        270

1         1        90 */
