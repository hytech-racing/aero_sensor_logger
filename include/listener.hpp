#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <string>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>
#include "aero_logger.hpp" // Include your logger header

class Listener : public std::enable_shared_from_this<Listener> {
public:
    Listener(boost::asio::io_context& io_context, const std::string& port_name);
    void start(std::queue<std::pair<std::vector<float>, std::string>>& data_queue,
               std::mutex& queue_mutex, std::condition_variable& cv);
    
private:
    void read();
    void on_read(const boost::system::error_code& error, std::size_t bytes_transferred);
    void process_buffer(const std::vector<uint8_t>& buffer);

    boost::asio::serial_port serial_port_;
    std::vector<uint8_t> buffer_;
    bool logging_enabled_;
};

#endif // LISTENER_HPP
