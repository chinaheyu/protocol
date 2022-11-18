#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <vector>
#include <sstream>
#include <fmt/format.h>
#include "protocol_lite.h"

namespace py = pybind11;

struct UnpackedFrame {
    UnpackedFrame(uint16_t _cmd_id, const std::string& _data) : cmd_id(_cmd_id), data(_data) { }
    UnpackedFrame(uint16_t _cmd_id, const char* _data, uint32_t _len) : cmd_id(_cmd_id), data(_data, _len) { }
    uint16_t cmd_id;
    py::bytes data;

    std::string __repr__() {
        return fmt::format("<protocol_lite.UnpackedFrame: cmd_id: 0x{:04x}, data size: {:d} bytes.>", cmd_id, py::len(data));
    }

    std::string __str__() {
        std::string data_string = data;
        if (data_string.size() > 0)
            return fmt::format("cmd_id: 0x{:04x}, data: {:02x}.", cmd_id, fmt::join(data_string.begin(), data_string.end(), " "));
        else
            return fmt::format("cmd_id: 0x{:04x}, empty data.", cmd_id);
    }
};

struct UnpackStream {
    UnpackStream() {
        protocol_initialize_unpack_object(&unpack_data_obj);
    }

    std::vector<UnpackedFrame> unpack_data(py::bytes& data) {
        std::string raw_data(data);

        std::vector<UnpackedFrame> out;
        for (char ch : raw_data) {
            if (protocol_unpack_byte(&unpack_data_obj, ch)) {
                out.emplace_back(unpack_data_obj.cmd_id,
                               reinterpret_cast<const char *>(unpack_data_obj.data),
                               unpack_data_obj.data_len);
            }
        }

        return out;
    }

    unpack_data_t unpack_data_obj;
};

uint32_t calculate_frame_size(uint32_t data_size) {
    return protocol_calculate_frame_size(data_size);
}

py::bytes pack_data(uint16_t cmd_id, py::bytes& data) {
    std::string data_string(data);
    uint32_t frame_size = protocol_calculate_frame_size(data_string.size());
    if (frame_size > PROTOCOL_FRAME_MAX_SIZE) {
        return {};
    }
    std::vector<char> out(frame_size);
    protocol_pack_data_to_buffer(cmd_id, reinterpret_cast<const uint8_t*>(data_string.data()), data_string.size(),
                                 reinterpret_cast<uint8_t *>(out.data()));
    return {out.data(), out.size()};
}


PYBIND11_MODULE(py_protocol, m) {
    m.doc() = "protocol_lite python wrapper.";

    py::class_<UnpackedFrame>(m, "UnpackedFrame")
        .def(py::init<uint16_t , const std::string &>())
        .def_readwrite("cmd_id", &UnpackedFrame::cmd_id)
        .def_readwrite("data", &UnpackedFrame::data)
        .def("__repr__", &UnpackedFrame::__repr__)
        .def("__str__", &UnpackedFrame::__str__);

    py::class_<UnpackStream>(m, "UnpackStream")
            .def(py::init<>())
            .def("unpack_data", &UnpackStream::unpack_data);

    m.def("calculate_frame_size", calculate_frame_size, "Calculate the size of packed frame.");

    m.def("pack_data", pack_data, "Pack data to frame.", py::arg("cmd_id"), py::arg("data") = py::bytes());
}
