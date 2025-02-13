#if !defined(PROTOCOL_H)
#define PROTOCOL_H

enum protocol_version {
protocol_unknown_version = -1,
protocol_v0 = 0,
protocol_v1 = 1,
protocol_v2 = 2,
};







enum protocol_version get_protocol_version_config(void);








enum protocol_version determine_protocol_version_server(void);





enum protocol_version determine_protocol_version_client(const char *server_response);

#endif 
