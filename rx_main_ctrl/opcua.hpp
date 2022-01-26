/**
 * @file ocpua.hpp
 */

#pragma once
#include <mutex>
#include <open62541/client_config_default.h>
#include <string>
#include <vector>

using std::mutex;
using std::string;
using std::vector;

namespace Mvt {
class OpcuaClient {
public:
  /**
   * @brief Constructor for connection to OPCUA server
   * @param _url opcua server address
   * @param _port opcua server port
   * @param _namespace namespace of variable to read
   * @param _prefix prefix of variable to read
   * @param _certfile certificate for secure connection
   * @param _keyfile private key for secure connection
   * @return 0 if success
   */
  OpcuaClient(const string &_url, const string &_port,
              const int16_t &namespace_, const string &_prefix = "",
              const string &_certfile = "",
              const string &_keyfile = "") noexcept;

  /**
   * @brief Destructor that closes the connection.
   */
  ~OpcuaClient();

  /**
   * @brief Open or reopen a connection to OPCUA server
   * @return 0 if success
   */
  int connect();

  /**
   * @brief set a value for a node
   * @param _name node name (without prefix)
   * @param _val value to set
   * @return 0 if success
   */
  int set(const string &_name, bool _val);

  /**
   * @brief get the value of a node
   * @param _name node name (without prefix)
   * @param _val reference to get
   * @return 0 if success
   */
  int get(const string &_name, bool &_val);

  //! @copydoc set(const string&, bool)
  int set(const string &_name, int16_t _val);
  //! @copydoc get(const string&, bool&)
  int get(const string &_name, int16_t &_val);

  //! @copydoc set(const string&, bool)
  int set(const string &_node_name, float _val);
  //! @copydoc get(const string&, bool&)
  int get(const string &_node_name, float &_val);

  //! @copydoc set(const string&, bool)
  int set(const string &_node_name, const vector<uint16_t> &_val);

private:
  UA_Client *client_; ///< connection to the opcua server
  string url_;        ///< url of the opcua server
  string port_;       ///< port for connection
  string cert_;       ///< certificate for secure connection
  string key_;        ///< private key for secure connection
  mutex mutex_;       ///< mutex for multi thread access
  int16_t namespace_; ///< ns= of variable to read
  string prefix_;     ///< prefix of variable to read

  /**
   * @brief set a value of a node of the type kind
   * @param _node_name name of the node (path separated with ".")
   * @param value new value to set
   * @param type type of the value
   * @return 0 if success
   */
  int set(const string &_node_name, void *value, int type);

  /**
   * @brief get a value in UA_Variant
   * @param _node_name name of the node (path separated with ".")
   * @param val value read
   * @return 0 if success
   */
  int get(const string &_nodename, UA_Variant *val);

  /**
   * @brief build the opcua "string nodeid" from a string
   * @note Use the ns and the prefix of the configuration
   * @note The node should be deleted (allocation of string) so should call
   * UA_NodeId_clear to release memory
   */
  UA_NodeId nodeId(const string &_node_name);
};

} // namespace Mvt
