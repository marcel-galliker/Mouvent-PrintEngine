#pragma once

/**
 * @brief Connect to a opcua server
 * @param address url of server
 * @param port port number of server
 * @param cert path to certificate file
 * @param key path to key file
 * @param user user name
 * @param password password of connection
 * @return 0 if success
 */
int opcua_connect(char *address, int port, char *cert, char *key);

/**
 * @brief get a boolean value of a node
 * @param _node_name name of the node (path separated with ".")
 * @param value returned value of the node
 * @return 0 if success
 */
int opcua_get_bool(char *_node_name, unsigned char *value);

/**
 * @brief set a boolean value of a node
 * @param _node_name name of the node (path separated with ".")
 * @param value value for the node
 * @return 0 if success
 */
int opcua_set_bool(char *_node_name, unsigned char value);

/**
 * @brief get a int16 value of a node
 * @param _node_name name of the node (path separated with ".")
 * @param value returned value of the node
 * @return 0 if success
 */
int opcua_set_int(char *_node_name, short value);

int opcua_set_float(char *_node_name, float value);

int opcua_get_int(char *_node_name, short *value);

int opcua_get_float(char *_node_name, float *value);

	/**
 * @brief get a int16 array of a node
 * @param _node_name name of the node (path separated with ".")
 * @param value array to set
 * @size size of the array
 * @return 0 if success
 */
int opcua_set_int_array(char *_node_name, short *value, int size);
