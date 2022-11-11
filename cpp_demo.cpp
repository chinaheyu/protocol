#include <iostream>
#include <vector>
#include <memory>
#include <thread>
#include "serial/serial.h"
#include "protocol_lite.h"
#include "fmt/format.h"
#include "fmt/chrono.h"


std::shared_ptr<serial::Serial> serial_port;

std::string select_ports()
{
    std::vector<serial::PortInfo> devices_found = serial::list_ports();

    for (size_t i = 0; i < devices_found.size(); ++i) {
        fmt::print("{}:[{}][{}][{}]\n", i, devices_found[i].port.data(), devices_found[i].description, devices_found[i].hardware_id);
    }

    int i;
    std::cin >> i;

    return devices_found[i].port;
}

void frame_received_callback(uint16_t cmd_id, uint8_t* p_data, uint16_t len) {
    std::vector<uint8_t> data(p_data, p_data + len);

    switch (cmd_id) {
        case 0x0000:
            uint32_t frame_size = protocol_calculate_frame_size(len);
            auto buf = new uint8_t[frame_size];
            protocol_pack_data_to_buffer(0x0001, p_data, len, buf);
            serial_port->write(buf, frame_size);
            delete[] buf;
            break;
    }

    fmt::print("[{:%H:%M:%S}]Received frame, cmd_id: 0x{:04x}, data: {:02x}.\n",
               fmt::localtime(std::time(nullptr)),cmd_id, fmt::join(data.begin(), data.end(), " "));
}

uint8_t hex_to_byte(char hi, char lo) {
    std::string hex_string({hi, lo});
    return std::stoi(hex_string, 0, 16);
}

void shell_process() {
    std::cout << "Now you can send protocol frame.\n"
                 "Example: >> 0102        # input cmd_id as hex.\n"
                 "         >> 1a 2b 3c    # input data as hex split with space.\n"
                 ">> ";

    uint16_t cmd_id;
    uint8_t data[PROTOCOL_DATA_MAX_SIZE];
    uint8_t frame_buffer[PROTOCOL_FRAME_MAX_SIZE];

    while (serial_port->isOpen()) {
        std::cin >> std::hex >> cmd_id;
        fmt::print("cmd_id: 0x{:04x}\n>> ", cmd_id);

        std::cin.clear();
        std::fflush(stdin);

        uint32_t data_size = 0;
        for (int i = 0; i < PROTOCOL_DATA_MAX_SIZE; ++i) {
            char ch1 = std::cin.get();
            if ((ch1 == '\r') || (ch1 == '\n'))
            {
                break;
            }

            char ch2 = std::cin.get();
            if ((ch2 == '\r') || (ch2 == '\n'))
            {
                break;
            }

            uint8_t byte = hex_to_byte(ch1, ch2);
            data[i] = byte;
            data_size = i + 1;

            char ch3 = std::cin.get();
            if (ch3 != ' ')
            {
                break;
            }
        }

        std::cin.clear();
        std::fflush(stdin);

        uint32_t frame_size = protocol_pack_data_to_buffer(cmd_id, data, data_size, frame_buffer);
        std::vector<uint8_t> data(frame_buffer, frame_buffer + frame_size);
        fmt::print("frame: {:02x}\n", fmt::join(data.begin(), data.end(), " "));

        size_t send_size = serial_port->write(frame_buffer, frame_size);
        fmt::print("Send {} bytes to device.\n>> ", send_size);
    }
}


int main(int argc, char* argv[]) {
    std::string port_name = select_ports();

    uint32_t baudrate;
    fmt::print("Please enter the baudrate: ");
    std::cin >> baudrate;

    serial_port = std::make_shared<serial::Serial>(port_name, baudrate, serial::Timeout::simpleTimeout(200));
    serial_port->flush();

    std::thread shell_thread(shell_process);

    unpack_data_t unpack_data_obj;
    protocol_initialize_unpack_object(&unpack_data_obj);
    std::vector<uint8_t> buffer;
    while (serial_port->isOpen()) {
        serial_port->read(buffer, std::max<size_t>(serial_port->available(), 1));

        for (uint8_t byte : buffer) {
            if (protocol_unpack_byte(&unpack_data_obj, byte)) {
                frame_received_callback(unpack_data_obj.cmd_id, unpack_data_obj.data, unpack_data_obj.data_len);
            }
        }
    }

    shell_thread.join();

    return 0;
}
