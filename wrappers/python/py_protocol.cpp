#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <fmt/format.h>
#include "protocol.h"

namespace py = pybind11;
using namespace pybind11::literals;

bool is_supported() {
    return protocol_is_supported();
}

ssize_t calculate_frame_size(uint16_t data_length) {
    return protocol_calculate_frame_size(data_length);
}

py::bytes pack_data(uint16_t cmd_id, py::bytes& data) {
    std::string data_string(data);
    size_t frame_size = protocol_calculate_frame_size(data_string.size());
    std::vector<char> out(frame_size);
    protocol_pack_data_to_buffer(cmd_id, reinterpret_cast<const uint8_t*>(data_string.data()), data_string.size(),
                                 reinterpret_cast<uint8_t *>(out.data()));
    return {out.data(), out.size()};
}

struct ProtocolFrame {
    ProtocolFrame(uint16_t _cmd_id, const std::string& _data) : cmd_id(_cmd_id), data(_data) { }
    ProtocolFrame(uint16_t _cmd_id, const char* _data, size_t _len) : cmd_id(_cmd_id), data(_data, _len) { }
    uint16_t cmd_id;
    py::bytes data;

    std::string __repr__() {
        return fmt::format("<ProtocolFrame: cmd_id: 0x{:04x}, data size: {:d} bytes.>", cmd_id, py::len(data));
    }

    std::string __str__() {
        std::string data_string = data;
        if (data_string.size() > 0)
            return fmt::format("cmd_id: 0x{:04x}, data: {:02x}.", cmd_id, fmt::join(data_string.begin(), data_string.end(), " "));
        else
            return fmt::format("cmd_id: 0x{:04x}, empty data.", cmd_id);
    }

    py::bytes pack() {
        return pack_data(cmd_id, data);
    }
};

struct UnpackStream {
    UnpackStream() {
        stream_obj = protocol_create_unpack_stream(0xffffu, true);
    }

    ~UnpackStream() {
        protocol_free_unpack_stream(stream_obj);
    }

    std::vector<ProtocolFrame> unpack_data(py::bytes& data) {
        std::string raw_data(data);
        std::vector<ProtocolFrame> out;
        for (char ch : raw_data) {
            if (protocol_unpack_byte(stream_obj, ch)) {
                out.emplace_back(stream_obj->cmd_id,
                                 reinterpret_cast<const char *>(stream_obj->data),
                                 stream_obj->data_len);
            }
        }
        return out;
    }

    std::vector<ProtocolFrame> __call__(py::bytes& data) {
        return unpack_data(data);
    }

    protocol_stream_t* stream_obj;
};


PYBIND11_MODULE(py_protocol, m) {
    m.doc() = "protocol python wrapper.";

    py::class_<ProtocolFrame>(m, "ProtocolFrame")
            .def(py::init<uint16_t , const std::string &>())
            .def_readwrite("cmd_id", &ProtocolFrame::cmd_id)
            .def_readwrite("data", &ProtocolFrame::data)
            .def("__repr__", &ProtocolFrame::__repr__)
            .def("__str__", &ProtocolFrame::__str__)
            .def("pack", &ProtocolFrame::pack);

    py::class_<UnpackStream>(m, "UnpackStream")
            .def(py::init<>())
            .def("unpack_data", &UnpackStream::unpack_data)
            .def("__call__", &UnpackStream::__call__);

    m.def("is_supported", is_supported, "Test whether the device supports the protocol.");

    m.def("calculate_frame_size", calculate_frame_size, "Calculate the size of packed frame.");

    m.def("pack_data", pack_data, "Pack data to frame.", py::arg("cmd_id"), py::arg("data") = py::bytes());
}
