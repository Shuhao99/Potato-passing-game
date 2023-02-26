#include "socket.h"

int build_listener(const char * port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;
    const char *hostname = NULL;

    memset(&host_info, 0, sizeof(host_info));  //initialize, write 0 to &hostinfo

    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;
    host_info.ai_flags    = AI_PASSIVE;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list);  
    // get a list of IP addresses and port numbers for host hostname and service port
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family,     
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    //creates an endpoint for communication and returns a descriptor
    //return value is a descriptor referencing the socket.

    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    int yes = 1;
    status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));  
    //allow address reuse
    status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    if (status == -1) {
        cerr << "Error: cannot bind socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    status = listen(socket_fd, 100);  
    // accept incoming connections, define a queue limit for incoming connections
    if (status == -1) {
        cerr << "Error: cannot listen on socket" << endl; 
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    freeaddrinfo(host_info_list);

    return socket_fd;
}

int build_sender(const char * hostname, const char * port){
    int status;
    int socket_fd;
    struct addrinfo host_info;
    struct addrinfo *host_info_list;

    memset(&host_info, 0, sizeof(host_info));
    host_info.ai_family   = AF_UNSPEC;
    host_info.ai_socktype = SOCK_STREAM;

    status = getaddrinfo(hostname, port, &host_info, &host_info_list); 
    // get a list of IP addresses and port numbers for host hostname and service port
    if (status != 0) {
        cerr << "Error: cannot get address info for host" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    socket_fd = socket(host_info_list->ai_family, 
                host_info_list->ai_socktype, 
                host_info_list->ai_protocol);
    //creates an endpoint for communication and returns a descriptor
    //return value is a descriptor referencing the socket.

    if (socket_fd == -1) {
        cerr << "Error: cannot create socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if
    
    cout << "Connecting to " << hostname << " on port " << port << "..." << endl;
    
    status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
    //stream sockets may successfully connect() only once; Y
    //datagram sockets may use connect() multiple times to change their association.
    if (status == -1) {
        cerr << "Error: cannot connect to socket" << endl;
        cerr << "  (" << hostname << "," << port << ")" << endl;
        return -1;
    } //if

    freeaddrinfo(host_info_list); 
    return socket_fd;
}