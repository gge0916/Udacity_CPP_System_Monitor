#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();
 private:
  // Previous usage.
  long active_;
  long total_;
};

#endif