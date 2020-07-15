#include <string>

#include "format.h"

using std::string;

string Format::ElapsedTime(long seconds) {
    int h, m, s;
    h = seconds / 3600;
    m = (seconds - (3600 * h)) / 60;
    s = (seconds -(3600 * h) - (m * 60));
    return std::to_string(h) + ":" + std::to_string(m) + ":" +
           std::to_string(s);
}
