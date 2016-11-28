#pragma once

#include <cinttypes>
#include <vector>

class FPS_Meter {
  public:
	FPS_Meter(bool print = false, float purge = 1) : print_fps(print), purge_rate(purge){};
	void frame(std::size_t lightcount);
	float get_fps(std::size_t lightcount);
	uint64_t get_frame_number();
	float get_time();
	float get_delta_time();

  private:
	void update_fps(std::size_t light_count);

	bool print_fps;

	std::vector<float> frame_times;
	bool fps_ready   = false;
	float fps        = 0;
	float timeperlight = 0;
	float purge_rate = 1;

	uint64_t frame_number = 0;

	float frame_time      = 0;
	float last_frame_time = 0;

	float last_print_time = 0;
};
