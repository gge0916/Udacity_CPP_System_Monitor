#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
// Reference:
// https://stackoverflow.com/questions/41224738/how-to-calculate-system-memory-usage-from-proc-meminfo-like-htop/41251290#41251290
float LinuxParser::MemoryUtilization() {
  std::ifstream fileStream(kProcDirectory + kMeminfoFilename);
  string line;
  string key;
  float value, memTotal, memFree, cached, buffers, sReclaimable, shmem;
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      while (lineStream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = value;
        } else if (key == "MemFree:") {
          memFree = value;
        } else if (key == "Cached:") {
          cached = value;
        } else if (key == "Buffers:") {
          buffers = value;
        } else if (key == "SReclaimable:") {
          sReclaimable = value;
        } else if (key == "Shmem:") {
          shmem = value;
        }
      }
      const float totalUsedMem = memTotal - memFree;
      const float cachedMem = cached + sReclaimable - shmem;
      const float nonCacheMem = totalUsedMem - (buffers + cachedMem);
      return nonCacheMem / memTotal;
    }
  }
  return 0;
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  std::ifstream fileStream(kProcDirectory + kUptimeFilename);
  string line;
  long value;
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream stringStream(line);
    stringStream >> value;
    return value;
  }
  return 0;
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// Read and return the number of active jiffies for a PID
// Reference:
// https://stackoverflow.com/questions/1420426/how-to-calculate-the-cpu-usage-of-a-process-by-pid-in-linux-from-c
long LinuxParser::ActiveJiffies(int pid) {
  string unwantedToken, line;
  long value, activeJiffies = 0;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    for (int i = 0; i < 13; i++) {
      lineStream >> unwantedToken;
    }
    // Column 14-17
    for (auto i = 0; i < 4; i++) {
      lineStream >> value;
      activeJiffies += value;
    }
  }
  return activeJiffies;
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpuUtilization = CpuUtilization();
  long activeJiffies = std::stol(cpuUtilization.at(CPUStates::kUser_)) +
                       std::stol(cpuUtilization.at(CPUStates::kNice_)) +
                       std::stol(cpuUtilization.at(CPUStates::kSystem_)) +
                       std::stol(cpuUtilization.at(CPUStates::kIRQ_)) +
                       std::stol(cpuUtilization.at(CPUStates::kSoftIRQ_)) +
                       std::stol(cpuUtilization.at(CPUStates::kSteal_)) +
                       std::stol(cpuUtilization.at(kGuest_)) +
                       std::stol(cpuUtilization.at(kGuestNice_));
  return activeJiffies;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpuUtilization = CpuUtilization();

  long idleJiffies = std::stol(cpuUtilization.at(CPUStates::kIdle_)) +
                     std::stol(cpuUtilization.at(CPUStates::kIOwait_));
  return idleJiffies;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  string line, cpu, value;
  vector<string> jiffies;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, line);
    std::istringstream lineStream(line);
    lineStream >> cpu;
    while (lineStream >> value) {
      jiffies.emplace_back(value);
    }
  }
  return jiffies;
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  string key, line;
  int processes = 0;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key;
      if (key == "processes") {
        lineStream >> processes;
        break;
      }
    }
  }
  return processes;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  string key, line;
  int runningProcesses = 0;
  std::ifstream fileStream(kProcDirectory + kStatFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key;
      if (key == "procs_running") {
        lineStream >> runningProcesses;
        break;
      }
    }
  }
  return runningProcesses;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmd;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (fileStream.is_open()) {
    std::getline(fileStream, cmd);
  }
  return cmd;
}

// Read and return the memory used by a process. Unit: MB.
string LinuxParser::Ram(int pid) {
  string key, line, unit;
  int vmSize = 0;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key;
      if (key == "VmSize:") {
        // KB
        lineStream >> vmSize;
        vmSize /= 1000;
      }
    }
  }
  return to_string(vmSize);
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) {
  string key, line, value;
  std::ifstream fileStream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::istringstream lineStream(line);
      lineStream >> key;
      if (key == "Uid:") {
        lineStream >> value;
      }
    }
  }
  return value;
}

// Read and return the user associated with a process
string LinuxParser::User(int pid) {
  string uid = Uid(pid);
  string line, userId, user, password;
  std::ifstream fileStream(kPasswordPath);
  if (fileStream.is_open()) {
    while (std::getline(fileStream, line)) {
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream lineStream(line);
      while (lineStream >> user >> password >> userId) {
        if (userId == uid) {
          return user;
        }
      }
    }
  }
  return user;
}

// Read and return the uptime of a process
long LinuxParser::UpTime(int pid) {
  string line, value;
  vector<string> values{};
  std::ifstream stream(kProcDirectory + std::to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream lineStream(line);
    while (lineStream >> value) {
      values.emplace_back(value);
    }
  }
  // Column #22: starttime - Time when the process started, measured in clock
  // ticks
  return stol(values[21]) / sysconf(_SC_CLK_TCK);
}