#include "aero_sensor.ph.h"
#include "aero_logger.hpp"
#include "listener.hpp"

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

// implement an async function
std::future<void> append_sensor_data(std::queue<std::pair<std::vector<float>, std::string>>& queue, const std::vector<float>& data, const std::string& port_name, std::mutex& queue_mutex, std::condition_variable& cv) {
    // Add data to queue asynchronously
    return std::async(std::launch::async, [&queue, &data, &port_name, &queue_mutex, &cv]() {
        std::lock_guard<std::mutex> lock(queue_mutex);  // Lock the mutex
        queue.push(std::make_pair(data, port_name));    // Add to the queue
        cv.notify_one();                                 // Notify the worker thread
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

void handle_signal(int signal) {
    std::cout << "Received signal " << signal << ", running cleanup..." << std::endl;
    std::exit(0);
}

Listener::Listener(boost::asio::io_context& io_context, const std::string& port_name)
    : serial_port_(io_context, port_name), data_queue_(data_queue), 
      queue_mutex_(queue_mutex), cv_(cv), logging_enabled_(true) {
    serial_port_.set_option(boost::asio::serial_port_base::baud_rate(500000));
}

void Listener::start(std::queue<std::pair<std::vector<float>, std::string>>& data_queue,
                     std::mutex& queue_mutex, std::condition_variable& cv) {
    read();
    serial_port_.write_some(boost::asio::buffer("@"));d
    std::cout << "Successfully wrote '@'\n";
    serial_port_.write_some(boost::asio::buffer("D"));
    std::cout << "Successfully wrote 'D'\n";
}

void Listener::read() {
    buffer_.resize(64);  // Adjust buffer size as needed
    boost::asio::async_read(serial_port_, boost::asio::buffer(buffer_),
                             boost::bind(&Listener::on_read, shared_from_this(),
                                         boost::asio::placeholders::error,
                                         boost::asio::placeholders::bytes_transferred));
}

void Listener::on_read(const boost::system::error_code& error, std::size_t bytes_transferred) {
    if (!error) {
        buffer_.resize(bytes_transferred);
        process_buffer(buffer_);

        read();
    } else {
        std::cerr << "Error in read: " << error.message() << std::endl;
    }
}

void Listener::process_buffer(const std::vector<uint8_t>& buffer) {
    if (buffer.size() < 32) {
        return; // Not enough data
    }

    // Check if the buffer contains the delimiter '#'
    auto hash_pos = std::find(buffer.begin(), buffer.end(), '#');
    if (hash_pos != buffer.end()) {
        std::vector<uint8_t> after_hash(hash_pos + 1, buffer.end());
        if (after_hash.size() >= 46) {
            // Extract sensor data as 8 floats
            std::vector<float> data(8);
            std::memcpy(data.data(), after_hash.data(), sizeof(float) * 8);

            // Lock the queue and add the data
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (logging_enabled_) {
                    data_queue_->emplace(data, serial_port_.name());  // Use the port name for logging
                    cv_->notify_one();  // Notify the worker that new data is available

                    append_sensor_data(*data_queue_, data, serial_port_.name());
                }
            }

            // Update buffer
            buffer_ = std::vector<uint8_t>(after_hash.begin() + 46, after_hash.end());
        } else {
            buffer_ = std::vector<uint8_t>{'#'};
            buffer_.insert(buffer_.end(), after_hash.begin(), after_hash.end());
        }
    }
}

