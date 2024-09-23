#pragma once

#include <chrono>


class Timer {
  public:
  typedef std::conditional<std::chrono::high_resolution_clock::is_steady,
                           std::chrono::high_resolution_clock,
                           std::chrono::steady_clock>::type PrecisionClock;
  /*!
   * \brief Start one Simple Timer. No Statistics will be generated.
   */
  void start() {
    started = true;
    start_time = PrecisionClock::now();
  }

  /*!
   * \brief Reset the Timer.
   */
  void reset() { started = false; }

  /*!
   * \brief Return true if start() was called and reset() was not.
   */
  bool hasStarted() const { return started; }

  /*!
   * \brief Returns the time since start() was called. Timer continues to run.
   * \return The passed time in given template format: std::chrono:: time
   * duration
   */
  template <class T> T getPassedTime() const {
    const auto stop_time = PrecisionClock::now();
    if (!started) {
      return std::chrono::duration_cast<T>(stop_time - stop_time);
    }
    return std::chrono::duration_cast<T>(stop_time - start_time);
  }

  private:
  PrecisionClock::time_point start_time;
  bool started = false;
};
