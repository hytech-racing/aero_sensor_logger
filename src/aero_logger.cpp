int main() {
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTERM, handle_signal);

    asio::io_context io_context;


// serial connections
    Listener listener1(io_context, '/dev/ttyACM0');
    listerner1.read();

    Listener listener2(io_context, '/dev/ttyACM1');
    listerner2.read();

// http server
    Httpserver server(io_context, 4111);

    std::thread w1(worker);
    io_context.run();
    w1.join();
    
    cleanup();
    return 0;
}
