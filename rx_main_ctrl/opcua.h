#pragma once

#ifndef HANDLE
#    define HANDLE void*
#endif

#ifdef __cplusplus
extern "C"
{
#endif
    int opcua_connect(HANDLE* client, const char* address, int port, short ns, const char* prefix, const char* cert, char* key);
    void opcua_close(HANDLE *client);
    int opcua_get_bool(HANDLE client, const char* _node_name, unsigned char* value);
    int opcua_set_bool(HANDLE client, const char* _node_name, unsigned char value);
    int opcua_get_float(HANDLE client, const char* _node_name, float* value);
    int opcua_set_float(HANDLE client, const char* _node_name, float value);
    int opcua_get_int(HANDLE client, char* _node_name, short* value);
    int opcua_set_int(HANDLE client, const char* _node_name, short value);
    int opcua_set_int_array(HANDLE client, char* _node_name, short* value, int size);
#ifdef __cplusplus
}
#endif
