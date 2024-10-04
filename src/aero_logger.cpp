#include "aero_sensor.ph.h"
#include "aero_logger.hpp"

#include <iostream>
#include <mcap/mcap.hpp>
#include <ctime>
#include <chrono>
#include <filesystem>
#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <thread>
#include <queue>
#include <csignal>
#include <cstdlib>
#include <future>
#include <mutex>
#include <condition_variable>
#include <array>

struct SensorData {
    std::array<float, 8> readings;  // Fixed array for eight 32-bit floats
};


bool start_new_log = false;
bool stop_current_log = false;

void log_sensor_data(mcap::McapWriter& mcap_logger, const std::vector<float>& data, const std::string& port_name) {
    // process mcap logger message
    aero_sensor::aero_data msg;
    for (float reading : data) {
        msg.add_readings_pa(reading);
    }

    // extract sensor name from port name
    string sensor_name = port_name.substr(port_name.find_last_of("/") + 1);

    mcap_logger.write_message(
        msg.GetTypeName() + "_" + sensor_name + "_data",
        reinterpret_cast<const std::byte*>(serialized_data.data()),
        serialized_data.size(),
        std::chrono::nanoseconds(log_time),
        std::chrono::nanoseconds(log_time)
    );
}

// implement an async function
std::future<void> append_sensor_data(std::queue<std::pair<std::vector<float>, std::string>>& queue, const std::vector<float>& data, const std::string& port_name) {
    // add data to queue as await
    return std::async(std::launch::async, [&queue, &data, &port_name]() {
        std::lock_guard<std::mutex> lock(queue_mutex);
        queue.push(std::make_pair(data, port_name));
        cv.notify_one();
    });
}

SensorData process_buffer(const std::vector<uint8_t>& buffer) {
    if (len(buffer) < 32) {
        raise ValueError("Buffer does not contain enough data for eight 32-bit floats.");
    }
    
    SensorData data;
    std::copy(buffer.begin(), buffer.begin() + 32, data.readings.begin());
    return data;
}

std::pair(mcap::McapWriter, std::ofstream) open_new_writer() {
    std::string path_to_mcap = ".";
    if (std::filesystem::exists("/etc/nixos")) {
        path_to_mcap = "/home/nixos/aero_sensor_recordings";
    }
    auto now = std::chrono::system_clock::now();
    // convert the now time to strftime format with m_d_y_h_m_s + .mcap
    std::time_t end_time = std::chrono::system_clock::to_time_t(now);
    std::string date_time_filename = std::ctime(&end_time);
    date_time_filename = date_time_filename.substr(0, date_time_filename.length() - 1);
    date_time_filename = date_time_filename.substr(0, date_time_filename.length() - 1) + ".mcap";
    std::string date_time_mcap_path = std::filesystem::path(path_to_mcap) / date_time_filename;

    std::ofstream writing_file(date_time_mcap_path, std::ios::binary);

    return std::make_pair(mcap::McapWriter(writing_file), writing_file);
}

void cleanup(mcap::McapWriter& mcap_writer, std::ofstream& writing_file) {
    if (mcap_writer.is_open()) {
        std::cout << "Finalizing MCAP writer..." << std::endl;
        mcap_writer.finalize();
    }
    if (writing_file.isop()) {
        writing_file.close();
    }
}

void handle_signal(int signal) {
    std::cout << "Received signal " << signal << ", running cleanup..." << std::endl;
    cleanup(mcap_writer)
    std::exit(0);
}







