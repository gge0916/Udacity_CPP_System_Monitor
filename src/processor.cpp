#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
  // Current usage.
  long active, total;
  total = LinuxParser::Jiffies();
  active = LinuxParser::ActiveJiffies();

  long deltaActive, deltaTotal;
  deltaActive = active - active_;
  deltaTotal = total - total_;

  active_ = active;
  total_ = total;

  return float(deltaActive) / float(deltaTotal);
}