#include <open62541/client_config_default.h>
#include <open62541/client_highlevel.h>
#include <open62541/plugin/log_stdout.h>
#include <open62541/plugin/securitypolicy.h>
#include <open62541/types.h>
#include <open62541/types_generated_handling.h>

#include "opcua.h"
#include "rx_threads.h"

static UA_Client *_client = NULL; // connection to opcua

static HANDLE _mutex;

#define NODE_INDEX 6

/* loadFile parses the certificate file.
 *
 * @param  path specifies the file name
 * @return Returns the file content after parsing
 */
static UA_INLINE UA_ByteString loadFile(const char *path)
{
	UA_ByteString fileContents = UA_STRING_NULL;

	/* Open the file */
	FILE *fp;
	fp = fopen(path, "rb");
	if (!fp)
	{
		errno = 0; /* We read errno also from the tcp layer... */
		return fileContents;
	}

	/* Get the file length, allocate the data and read */
	fseek(fp, 0, SEEK_END);
	fileContents.length = (size_t)ftell(fp);
	fileContents.data = (UA_Byte *)UA_malloc(fileContents.length * sizeof(UA_Byte));
	if (fileContents.data)
	{
		fseek(fp, 0, SEEK_SET);
		size_t read = fread(fileContents.data, sizeof(UA_Byte), fileContents.length, fp);
		if (read != fileContents.length) UA_ByteString_clear(&fileContents);
	}
	else
	{
		fileContents.length = 0;
	}
	fclose(fp);

	return fileContents;
}

/**
 * @brief set a value of a node of the type kind
 * @param _node_name name of the node (path separated with ".")
 * @param value new value to set
 * @param type type of the value
 * @return 0 if success
 */
static int opcua_set_value(char *_node_name, void *value, int type);

/**
 * @brief get a value in UA_Variant
 * @param _node_name name of the node (path separated with ".")
 * @param val value read
 * @return 0 if success
*/
static int opcua_get_val(char *_node_name, UA_Variant *val)
{
	if (_client == NULL) return UA_STATUSCODE_BADCONNECTIONCLOSED;
	UA_NodeId nodeId = UA_NODEID_STRING(NODE_INDEX, _node_name);
	rx_mutex_lock(_mutex);
	UA_StatusCode retval = UA_Client_readValueAttribute(_client, nodeId, val);
	rx_mutex_unlock(_mutex);
	return retval;
}


int opcua_connect(char *address, int port, char *cert, char *key)
{
	UA_StatusCode retval = UA_STATUSCODE_GOOD;

	if (_client == NULL) // Initiate the client configuration
	{
		_mutex = rx_mutex_create();
		/* Load certificate and private key */
		UA_ByteString certificate = loadFile(cert);
		UA_ByteString privateKey = loadFile(key);

		_client = UA_Client_new();
		UA_ClientConfig *config = UA_Client_getConfig(_client);

		UA_ClientConfig_setDefaultEncryption(config, certificate, privateKey, NULL, 0, NULL, 0);

		UA_ByteString_clear(&certificate);
		UA_ByteString_clear(&privateKey);
	}
	rx_mutex_lock(_mutex);
	UA_SessionState ss;
	UA_Client_getState(_client, NULL, &ss, NULL);
	if (ss != UA_SESSIONSTATE_ACTIVATED)
	{
		char endpointUrl[256];
		sprintf(endpointUrl, "opc.tcp://%s:%d", address, port);
		// Connect or reconnect
		retval = UA_Client_connect(_client, endpointUrl);
	}
	rx_mutex_unlock(_mutex);
	return retval;
}

static int opcua_set_value(char *_node_name, void *value, int type)
{
	if (_client == NULL) return UA_STATUSCODE_BADCONNECTIONCLOSED;
	UA_StatusCode retval;
	UA_Variant *myVariant = UA_Variant_new();
	UA_Variant_setScalarCopy(myVariant, value, &UA_TYPES[type]);
	rx_mutex_lock(_mutex);
	retval = UA_Client_writeValueAttribute(_client, UA_NODEID_STRING(NODE_INDEX, _node_name), myVariant);
	rx_mutex_unlock(_mutex);
	UA_Variant_delete(myVariant);
	return retval;
}

int opcua_get_bool(char *_node_name, unsigned char *value)
{
	UA_Variant *val = UA_Variant_new();
	UA_StatusCode retval = opcua_get_val(_node_name, val);
	if (retval == UA_STATUSCODE_GOOD)
	{
		if (UA_Variant_isScalar(val) && val->type == &UA_TYPES[UA_DATATYPEKIND_BOOLEAN])
		{
			*value = *((unsigned char *)val->data);
		}
		else
		{
			retval = UA_STATUSCODE_BADINVALIDARGUMENT;
		}
	}
	UA_Variant_delete(val);
	return retval;
}

int opcua_set_bool(char *_node_name, unsigned char value)
{
	return opcua_set_value(_node_name, &value, UA_TYPES_BOOLEAN);
}

int opcua_get_float(char *_node_name, float *value)
{
	UA_Variant *val = UA_Variant_new();
	UA_StatusCode retval = opcua_get_val(_node_name, val);
	if (retval == UA_STATUSCODE_GOOD)
	{
		if (UA_Variant_isScalar(val) && val->type == &UA_TYPES[UA_TYPES_FLOAT])
		{
			*value = *((UA_Float *)val->data);
		}
		else
		{
			retval = UA_STATUSCODE_BADINVALIDARGUMENT;
		}
	}
	UA_Variant_delete(val);
	return retval;
}


int opcua_get_int(char *_node_name, short *value)
{
	UA_Variant *val = UA_Variant_new();
	UA_StatusCode retval = opcua_get_val(_node_name, val);
	if (retval == UA_STATUSCODE_GOOD)
	{
		if (UA_Variant_isScalar(val) && val->type == &UA_TYPES[UA_TYPES_INT16])
		{
			*value = *((UA_Int16 *)val->data);
		}
		else
		{
			retval = UA_STATUSCODE_BADINVALIDARGUMENT;
		}
	}
	UA_Variant_delete(val);
	return retval;
}

int opcua_set_int(char *_node_name, short value)
{
	return opcua_set_value(_node_name, &value, UA_DATATYPEKIND_INT16);
}

int opcua_set_float(char *_node_name, float value)
{
	return opcua_set_value(_node_name, &value, UA_DATATYPEKIND_FLOAT);
}

int opcua_set_int_array(char *_node_name, short *value, int size)
{
	if (_client == NULL) return UA_STATUSCODE_BADCONNECTIONCLOSED;
	UA_StatusCode retval;
	UA_Variant *myVariant = UA_Variant_new();
	UA_Variant_setArrayCopy(myVariant, value, size, &UA_TYPES[UA_TYPES_INT16]);
	rx_mutex_lock(_mutex);
	retval = UA_Client_writeValueAttribute(_client, UA_NODEID_STRING(NODE_INDEX, _node_name), myVariant);
	rx_mutex_unlock(_mutex);
	UA_Variant_delete(myVariant);
	return retval;
}
