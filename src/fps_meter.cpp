#include "fps_meter.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <iostream>
#include <utility>

void FPS_Meter::frame(size_t lightcount) {
	last_frame_time = std::exchange(frame_time, SDL_GetTicks() / 1000.f);
	frame_times.push_back(frame_time);
	frame_number += 1;
	fps_ready = false;
	update_fps(lightcount);
}

float FPS_Meter::get_fps(size_t lightcount) {
	update_fps(lightcount);
	return fps;
}

uint64_t FPS_Meter::get_frame_number() {
	return frame_number;
}

float FPS_Meter::get_time() {
	return frame_time;
}

float FPS_Meter::get_delta_time() {
	return frame_time - last_frame_time;
}

void FPS_Meter::update_fps(size_t lightcount) {
	auto calc_fps = [&]() {
		fps = static_cast<float>(frame_times.size()) / (frame_times.back() - frame_times.front());
		timeperlight = ((1.0 / fps) * 1000) / lightcount;
	};

	auto print = [&]() {
		if (print_fps && frame_time - last_print_time >= 1) {
			std::cout << "FPS: " << fps << " - " << frame_times.size() << " - " << timeperlight << "ms/light" << std::endl;
			last_print_time = frame_time;
		}
	};

	if (fps_ready == false) {
		if (frame_times.size() > 10) {
			float front_time = frame_times.back();
			auto new_end     = std::remove_if(frame_times.begin(), frame_times.end(),
			                              [&front_time, this](float val) { return front_time - val > purge_rate; });
			frame_times.erase(new_end, frame_times.end());

			calc_fps();
			print();
		}
		else if (frame_times.size() != 0) {
			calc_fps();
			print();
		}
		else {
			fps = 0;
		}
		fps_ready = true;
	}
}
