add_subdirectory(01-tcp-socket-beginner)
set_target_properties(tcp_socket_beginner PROPERTIES OUTPUT_NAME "04-01-tcp-socket")

add_subdirectory(02-udp-socket-beginner)
set_target_properties(udp_socket_beginner PROPERTIES OUTPUT_NAME "04-02-udp-socket")

add_subdirectory(03-network-access-manager-beginner)
set_target_properties(network_access_manager_beginner PROPERTIES OUTPUT_NAME "04-03-network-access-manager")

add_subdirectory(04-websocket-beginner)
set_target_properties(websocket_beginner PROPERTIES OUTPUT_NAME "04-04-websocket")

add_subdirectory(05-ssl-tls-beginner)
set_target_properties(ssl_tls_beginner PROPERTIES OUTPUT_NAME "04-05-ssl-tls")

add_subdirectory(06-serial-port-beginner)
set_target_properties(serial_port_beginner PROPERTIES OUTPUT_NAME "04-06-serial-port")
