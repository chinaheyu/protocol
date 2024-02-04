#include "py/dynruntime.h"
#include "protocol.h"

STATIC mp_obj_t is_supported(void) {
    return mp_obj_new_bool(protocol_is_supported());
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(is_supported_obj, is_supported);

STATIC mp_obj_t calculate_frame_size(mp_obj_t data_length)
{
    return mp_obj_new_int(protocol_calculate_frame_size(mp_obj_get_int(data_length)));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(calculate_frame_size_obj, calculate_frame_size);

STATIC mp_obj_t pack_data(mp_obj_t cmd_id, mp_obj_t data)
{
    uint16_t _cmd_id = mp_obj_get_int(cmd_id);
    mp_buffer_info_t _data;
    mp_get_buffer_raise(data, &_data, MP_BUFFER_READ);
    uint32_t frame_size = protocol_calculate_frame_size(_data.len);
    uint8_t* out = (uint8_t*)m_malloc(frame_size);
    // FIXME: >>> p.pack_data(0x1234,b'abcd') device disconnected. Maybe compiler optimization error.
    protocol_pack_data_to_buffer(_cmd_id, _data.buf, _data.len, out);
    return mp_obj_new_bytearray_by_ref(frame_size, out);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(pack_data_obj, pack_data);

mp_obj_full_type_t mp_type_ProtocolFrame;

typedef struct _ProtocolFrame_obj_t {
    // All objects start with the base.
    mp_obj_base_t base;
    // Everything below can be thought of as instance attributes, but they
    // cannot be accessed by MicroPython code directly.
    mp_obj_t cmd_id_obj;
    mp_obj_t data_obj;
} ProtocolFrame_obj_t;

STATIC mp_obj_t ProtocolFrame_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 2, 2, false);

    // Allocates the new object and sets the type.
    ProtocolFrame_obj_t *self = m_new_obj(ProtocolFrame_obj_t);
    self->base.type = (mp_obj_type_t *)type;
    self->cmd_id_obj = args[0];
    self->data_obj = args[1];

    // The make_new function always returns self.
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t ProtocolFrame_cmd_id(mp_obj_t self_in) {
    ProtocolFrame_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return self->cmd_id_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ProtocolFrame_cmd_id_obj, ProtocolFrame_cmd_id);

STATIC mp_obj_t ProtocolFrame_data(mp_obj_t self_in) {
    ProtocolFrame_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return self->data_obj;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ProtocolFrame_data_obj, ProtocolFrame_data);

STATIC mp_obj_t ProtocolFrame_pack(mp_obj_t self_in) {
    ProtocolFrame_obj_t* self = MP_OBJ_TO_PTR(self_in);
    return pack_data(self->cmd_id_obj, self->data_obj);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(ProtocolFrame_pack_obj, ProtocolFrame_pack);

mp_map_elem_t ProtocolFrame_locals_dict_table[3];
STATIC MP_DEFINE_CONST_DICT(ProtocolFrame_locals_dict, ProtocolFrame_locals_dict_table);

mp_obj_full_type_t mp_type_UnpackStream;

typedef struct _UnpackStream_obj_t {
    // All objects start with the base.
    mp_obj_base_t base;
    // need to store this to prevent GC from reclaiming buf.
    mp_obj_t buf_obj;
    // Everything below can be thought of as instance attributes, but they
    // cannot be accessed by MicroPython code directly.
    protocol_stream_t stream_obj;
} UnpackStream_obj_t;

STATIC mp_obj_t UnpackStream_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // Allocates the new object and sets the type.
    UnpackStream_obj_t *self = m_new_obj(UnpackStream_obj_t);
    self->base.type = (mp_obj_type_t *)type;
    self->stream_obj.max_data_length = 0xffffu;
    self->stream_obj.protocol_packet_size = 8;
    self->stream_obj.protocol_packet_ptr = (uint8_t*)m_malloc(self->stream_obj.protocol_packet_size);
    self->buf_obj = mp_obj_new_bytearray_by_ref(self->stream_obj.protocol_packet_size, self->stream_obj.protocol_packet_ptr);
    protocol_initialize_unpack_stream(&self->stream_obj);

    // The make_new function always returns self.
    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t UnpackStream_unpack_data(mp_obj_t self_in, mp_obj_t data) {
    UnpackStream_obj_t* self = MP_OBJ_TO_PTR(self_in);
    mp_buffer_info_t _data;
    mp_get_buffer_raise(data, &_data, MP_BUFFER_READ);
    mp_obj_list_t *lst = MP_OBJ_TO_PTR(mp_obj_new_list(0, NULL));
    for (int i = 0; i < _data.len; ++i) {
        if (protocol_unpack_byte(&self->stream_obj, ((uint8_t*)_data.buf)[i])) {
            ProtocolFrame_obj_t *frame_obj = m_new_obj(ProtocolFrame_obj_t);
            frame_obj->base.type = (mp_obj_type_t *)&mp_type_ProtocolFrame;
            frame_obj->cmd_id_obj = mp_obj_new_int(self->stream_obj.cmd_id);
            frame_obj->data_obj = mp_obj_new_bytes(self->stream_obj.data, self->stream_obj.data_len);
            mp_obj_list_append(lst, frame_obj);
        }
        if (self->stream_obj.unpack_step == STEP_DATA_CRC16) {
            // reallocate
            if (self->stream_obj.protocol_packet_size < self->stream_obj.data_len + 8) {
                self->stream_obj.protocol_packet_size = self->stream_obj.data_len + 8;
                self->stream_obj.protocol_packet_ptr = (uint8_t*)m_realloc(self->stream_obj.protocol_packet_ptr, self->stream_obj.protocol_packet_size);
                self->buf_obj = mp_obj_new_bytearray_by_ref(self->stream_obj.protocol_packet_size, self->stream_obj.protocol_packet_ptr);
            }
        }
    }
    return MP_OBJ_TO_PTR(lst);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(UnpackStream_unpack_data_obj, UnpackStream_unpack_data);

STATIC mp_obj_t UnpackStream_call(mp_obj_t self_in, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    return UnpackStream_unpack_data(self_in, args[0]);
}

mp_map_elem_t UnpackStream_locals_dict_table[1];
STATIC MP_DEFINE_CONST_DICT(UnpackStream_locals_dict, UnpackStream_locals_dict_table);

// This is the entry point and is called when the module is imported
mp_obj_t mpy_init(mp_obj_fun_bc_t *self, size_t n_args, size_t n_kw, mp_obj_t *args) {
    // This must be first, it sets up the globals dict and other things
    MP_DYNRUNTIME_INIT_ENTRY

    mp_type_ProtocolFrame.base.type = (void*)&mp_type_type;
    mp_type_ProtocolFrame.name = MP_QSTR_ProtocolFrame;
    ProtocolFrame_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_cmd_id), MP_OBJ_FROM_PTR(&ProtocolFrame_cmd_id_obj) };
    ProtocolFrame_locals_dict_table[1] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_data), MP_OBJ_FROM_PTR(&ProtocolFrame_data_obj) };
    ProtocolFrame_locals_dict_table[2] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_pack), MP_OBJ_FROM_PTR(&ProtocolFrame_pack_obj) };
    MP_OBJ_TYPE_SET_SLOT(&mp_type_ProtocolFrame, make_new, ProtocolFrame_make_new, 0);
    MP_OBJ_TYPE_SET_SLOT(&mp_type_ProtocolFrame, locals_dict, (void*)&ProtocolFrame_locals_dict, 1);

    mp_type_UnpackStream.base.type = (void*)&mp_type_type;
    mp_type_UnpackStream.name = MP_QSTR_UnpackStream;
    UnpackStream_locals_dict_table[0] = (mp_map_elem_t){ MP_OBJ_NEW_QSTR(MP_QSTR_unpack_data), MP_OBJ_FROM_PTR(&UnpackStream_unpack_data_obj) };
    MP_OBJ_TYPE_SET_SLOT(&mp_type_UnpackStream, make_new, UnpackStream_make_new, 0);
    MP_OBJ_TYPE_SET_SLOT(&mp_type_UnpackStream, call, UnpackStream_call, 1);
    MP_OBJ_TYPE_SET_SLOT(&mp_type_UnpackStream, locals_dict, (void*)&UnpackStream_locals_dict, 2);

    // Make the function available in the module's namespace
    mp_store_global(MP_QSTR___name__, MP_OBJ_NEW_QSTR(MP_QSTR_py_protocol));
    mp_store_global(MP_QSTR_is_supported, MP_OBJ_FROM_PTR(&is_supported_obj));
    mp_store_global(MP_QSTR_calculate_frame_size, MP_OBJ_FROM_PTR(&calculate_frame_size_obj));
    mp_store_global(MP_QSTR_pack_data, MP_OBJ_FROM_PTR(&pack_data_obj));
    mp_store_global(MP_QSTR_ProtocolFrame, MP_OBJ_FROM_PTR(&mp_type_ProtocolFrame));
    mp_store_global(MP_QSTR_UnpackStream, MP_OBJ_FROM_PTR(&mp_type_UnpackStream));

    // This must be last, it restores the globals dict
    MP_DYNRUNTIME_INIT_EXIT
}
