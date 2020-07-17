#include <string>

#include "process.h"
#include "linux_parser.h"
#include <unistd.h>
using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid): pid_{pid} {}

// Return this process's id
int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const {
  long totalTime = LinuxParser::ActiveJiffies(pid_);
  long seconds = LinuxParser::UpTime() - LinuxParser::UpTime(pid_);
  long herz = sysconf(_SC_CLK_TCK);

  return 100 * ((totalTime / herz) / seconds);
}

// Return the command that generated this process
string Process::Command() const {
    return LinuxParser::Command(pid_);
}

// Return this process's memory utilization
string Process::Ram() const {
  return LinuxParser::Ram(pid_);
}

// Return the user (name) that generated this process
string Process::User() const {
  return LinuxParser::User(pid_);
}

// Return the age of this process (in seconds)
long int Process::UpTime() const {
  return LinuxParser::UpTime(pid_);
}

// Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return CpuUtilization() < a.CpuUtilization();
}