/**
 * @file ocpua.cpp
 */

/*
 * Access to an OPCUA Client using the open62541 C library
 */
#include "opcua.hpp"

#include "opcua.h"

#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>

using std::unique_lock;

namespace Mvt {
/* loadFile parses the certificate file.
 *
 * @param  path specifies the file name
 * @return Returns the file content after parsing
 */
static UA_INLINE UA_ByteString loadFile(const string &path) {
  UA_ByteString fileContents = UA_STRING_NULL;

  /* Open the file */
  FILE *fp;
  fp = fopen(path.c_str(), "rb");
  if (!fp) {
    errno = 0; /* We read errno also from the tcp layer... */
    return fileContents;
  }

  /* Get the file length, allocate the data and read */
  fseek(fp, 0, SEEK_END);
  fileContents.length = (size_t)ftell(fp);
  fileContents.data =
      (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
  if (fileContents.data) {
    fseek(fp, 0, SEEK_SET);
    size_t read =
        fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
    if (read != fileContents.length)
      UA_ByteString_clear(&fileContents);
  } else {
    fileContents.length = 0;
  }
  fclose(fp);

  return fileContents;
}

OpcuaClient::OpcuaClient(const string &_url, const string &_port,
                         const int16_t &_namespace, const string &_prefix,
                         const string &_certfile,
                         const string &_keyfile) noexcept
    : url_{_url}, port_{_port}, namespace_{_namespace}, prefix_{_prefix + "."} {
  /* Load certificate and private key */
  UA_ByteString certificate = loadFile(_certfile);
  UA_ByteString privateKey = loadFile(_keyfile);

  client_ = UA_Client_new();
  UA_ClientConfig *config = UA_Client_getConfig(client_);

  UA_ClientConfig_setDefaultEncryption(config, certificate, privateKey, nullptr,
                                       0, nullptr, 0);

  UA_ByteString_clear(&certificate);
  UA_ByteString_clear(&privateKey);
}

OpcuaClient::~OpcuaClient() {
  unique_lock<mutex> lock(mutex_);
  UA_Client_disconnect(client_);
  UA_Client_delete(client_);
}

int OpcuaClient::connect() {
  UA_StatusCode retval = UA_STATUSCODE_GOOD;

  unique_lock<mutex> lock(mutex_);
  UA_SessionState ss;
  UA_Client_getState(client_, nullptr, &ss, nullptr);
  if (ss != UA_SESSIONSTATE_ACTIVATED) {
    string endpointUrl = "opc.tcp://" + url_ + ":" + port_;
    // Connect or reconnect
    retval = UA_Client_connect(client_, endpointUrl.c_str());
  }
  return retval;
}

UA_NodeId OpcuaClient::nodeId(const string &_node_name) {
  return UA_NODEID_STRING_ALLOC(
      namespace_, const_cast<char *>((prefix_ + _node_name).c_str()));
}

int OpcuaClient::get(const string &_node_name, UA_Variant *val) {
  UA_NodeId id = nodeId(_node_name);
  unique_lock<mutex> lock(mutex_);
  UA_StatusCode retval = UA_Client_readValueAttribute(client_, id, val);
  UA_NodeId_clear(&id);
  return retval;
}

int OpcuaClient::set(const string &_node_name, void *_val, int _type) {
  UA_StatusCode retval;
  UA_NodeId id = nodeId(_node_name);
  UA_Variant *myVariant = UA_Variant_new();
  if (myVariant == NULL)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  UA_Variant_setScalarCopy(myVariant, _val, &UA_TYPES[_type]);
  unique_lock<mutex> lock(mutex_);
  retval = UA_Client_writeValueAttribute(client_, id, myVariant);
  UA_Variant_delete(myVariant);
  UA_NodeId_clear(&id);
  return retval;
}

int OpcuaClient::get(const string &_name, bool &_value) {
  UA_Variant *val = UA_Variant_new();
  if (val == NULL)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  UA_StatusCode retval = get(_name, val);
  if (retval == UA_STATUSCODE_GOOD) {
    if (UA_Variant_isScalar(val) &&
        val->type == &UA_TYPES[UA_DATATYPEKIND_BOOLEAN]) {
      _value = *((unsigned char *)val->data);
    } else {
      retval = UA_STATUSCODE_BADINVALIDARGUMENT;
    }
  }
  UA_Variant_delete(val);
  return retval;
}

int OpcuaClient::set(const string &_node_name, bool _value) {
  return set(_node_name, &_value, UA_TYPES_BOOLEAN);
}

int OpcuaClient::get(const string &_node_name, float &_value) {
  UA_Variant *val = UA_Variant_new();
  if (val == NULL)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  UA_StatusCode retval = get(_node_name, val);
  if (retval == UA_STATUSCODE_GOOD) {
    if (UA_Variant_isScalar(val) && val->type == &UA_TYPES[UA_TYPES_FLOAT]) {
      _value = *((UA_Float *)val->data);
    } else {
      retval = UA_STATUSCODE_BADINVALIDARGUMENT;
    }
  }
  UA_Variant_delete(val);
  return retval;
}

int OpcuaClient::set(const string &_node_name, float value) {
  return set(_node_name, &value, UA_DATATYPEKIND_FLOAT);
}

int OpcuaClient::get(const string &_node_name, int16_t &_value) {
  UA_Variant *val = UA_Variant_new();
  if (val == NULL)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  UA_StatusCode retval = get(_node_name, val);
  if (retval == UA_STATUSCODE_GOOD) {
    if (UA_Variant_isScalar(val) && val->type == &UA_TYPES[UA_TYPES_INT16]) {
      _value = *((UA_Int16 *)val->data);
    } else {
      retval = UA_STATUSCODE_BADINVALIDARGUMENT;
    }
  }
  UA_Variant_delete(val);
  return retval;
}

int OpcuaClient::set(const string &_node_name, int16_t value) {
  return set(_node_name, &value, UA_DATATYPEKIND_INT16);
}

int OpcuaClient::set(const string &_node_name, const vector<uint16_t> &_value) {
  UA_StatusCode retval;
  UA_Variant *myVariant = UA_Variant_new();
  if (myVariant == NULL)
    return UA_STATUSCODE_BADOUTOFMEMORY;
  // first read the value from the server, to be sure to have the good type
  // else we get a UA_STATUSCODE_BADTYPEMISMATCH 
  retval = get(_node_name, myVariant);
  if (retval == 0)
  {
      // then overwrite the array with the _value and write it to the server
      UA_Variant_setArrayCopy(myVariant, _value.data(), _value.size(),
                              &UA_TYPES[UA_TYPES_UINT16]);
      UA_NodeId id = nodeId(_node_name);
      unique_lock<mutex> lock(mutex_);
      retval = UA_Client_writeValueAttribute(client_, id, myVariant);
      UA_NodeId_clear(&id);
  }
  UA_Variant_delete(myVariant);
  return retval;
}
} // namespace Mvt

extern "C" {
using std::to_string;

int opcua_connect(HANDLE *client, const char *address, int port, short ns,
                  const char *prefix, const char *cert, char *key) {
  if (*client == NULL) {
    if ((*client = new Mvt::OpcuaClient(address, to_string(port), ns, prefix,
                                        cert, key)) == NULL)
      return UA_STATUSCODE_BADOUTOFMEMORY;
  }
  return ((Mvt::OpcuaClient *)*client)->connect();
}

void opcua_close(HANDLE *client) {
  if (*client != NULL)
    delete (Mvt::OpcuaClient *)*client;
  *client = NULL;
}

int opcua_get_bool(HANDLE client, const char *_node_name,
                   unsigned char *value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->get(_node_name, (bool &)*value);
}

int opcua_set_bool(HANDLE client, const char *_node_name, unsigned char value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->set(_node_name, (bool)value);
}

int opcua_get_float(HANDLE client, const char *_node_name, float *value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->get(_node_name, *value);
}

int opcua_set_float(HANDLE client, const char *_node_name, float value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->set(_node_name, value);
}

int opcua_get_int(HANDLE client, char *_node_name, short *value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->get(_node_name, *value);
}

int opcua_set_int(HANDLE client, const char *_node_name, short value) {
  if (client == NULL)
    return UA_STATUSCODE_BADCONNECTIONCLOSED;
  return ((Mvt::OpcuaClient *)client)->set(_node_name, value);
}

int opcua_set_uint_array(HANDLE client, char *_node_name, unsigned short *value,
                        int size) {
  vector<uint16_t> arr(value, value + size);

  return ((Mvt::OpcuaClient *)client)->set(_node_name, arr);
}
}