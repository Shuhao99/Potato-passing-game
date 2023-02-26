#include "potato.h"
#include "socket.h"
#include <string>
#include <vector>
#include <algorithm>
#include <utility>
#include <cstdlib>
#include <ctime>


int main(int argc, char const *argv[])
{
    const int BUFFER_SIZE = 512;
    if (argc != 3) {
        std::cout << "Usage: player <machine_name> <port_num>" << std::endl;
        return 1;
    }
    //get master host name
    std::string master_name(argv[1]); 
    //get master port number
    std::string master_port(argv[2]);
    
    //build conncetion with master
    int master_fd = build_sender(master_name.c_str(), master_port.c_str());
    if (master_fd == -1)
    {
        std::cout << "Join failed, check your input!" << std::endl;
        return 1;
    }
    
    //receive id
    int id = -1;
    recv(master_fd, &id, sizeof(id), MSG_WAITALL);
    
    //receive player number
    int player_num = 0;
    recv(master_fd, &player_num, sizeof(player_num), MSG_WAITALL);
    
    //build listener
    int listener = build_listener("");
    
    //get local port number
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(listener, (struct sockaddr*)&local_addr, &local_addr_len) < 0) {
        std::cerr << "getsockname() error" << std::endl;
        close(listener);
        close(master_fd);
        return 1;
    }
    int left_port = ntohs(local_addr.sin_port);
    
    //send listening port number
    send(master_fd, &left_port, sizeof(left_port), 0);
    
    //print connected
    std::cout << "Connected as player " << id 
    << " out of " << player_num << " total players\n";
    
    //receive right neighbor port
    int right_port = -1;
    recv(master_fd, &right_port, sizeof(right_port), 0);

    //receive right neighbor ip
    char buffer[BUFFER_SIZE];
    int rec_len = recv(master_fd, buffer, sizeof(buffer), 0);
    std::string right_ip(buffer, rec_len);

    int left_fd = 0;
    int right_fd = 0;

    if(id == (player_num - 1)){
        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);
        left_fd = accept(listener, (struct sockaddr *)&socket_addr, &socket_addr_len);

        //build sender
        right_fd = build_sender(right_ip.c_str(), std::to_string(right_port).c_str());
    }
    else{
        //build sender
        right_fd = build_sender(right_ip.c_str(), std::to_string(right_port).c_str());

        struct sockaddr_storage socket_addr;
        socklen_t socket_addr_len = sizeof(socket_addr);

        left_fd = accept(listener, (struct sockaddr *)&socket_addr, &socket_addr_len);
    }
    
    //send id
    send(master_fd, &id, sizeof(id), 0);

    //reveive start msg
    int signal = 0;
    recv(master_fd, &signal, sizeof(signal), MSG_WAITALL);


    //wait for potato
    fd_set readfds;
    int nfds = max(master_fd, max(left_fd, right_fd));

    potato cur_potato = potato();
    srand((unsigned)time(NULL) + left_port);
    while (true)
    {
        FD_ZERO(&readfds);

        FD_SET(master_fd, &readfds);
        FD_SET(left_fd, &readfds);
        FD_SET(right_fd, &readfds);

        select(nfds + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(master_fd, &readfds)){
            if(recv(master_fd, &cur_potato, sizeof(cur_potato), MSG_WAITALL) <= 0){
                break;
            }
        }
        else if (FD_ISSET(right_fd, &readfds)){
            if(recv(right_fd, &cur_potato, sizeof(cur_potato), MSG_WAITALL) <= 0){
                break;
            }
        }
        else if (FD_ISSET(left_fd, &readfds)){
            if(recv(left_fd, &cur_potato, sizeof(cur_potato), MSG_WAITALL) <= 0){
                break;
            }  
        }
        //count down and add self to potato
        if (cur_potato.hops == 0){
            break;
        }
        else if (cur_potato.hops > 1){
            cur_potato.track[cur_potato.index] = id;
            cur_potato.index++;
            cur_potato.hops--;

            //randomly pass to left or right
            int random_number = rand() % 2;
            if(random_number == 1){
                int right_id = (id == player_num - 1) ? 0 : id + 1;
                std::cout << "Sending potato to " << right_id << std::endl;
                send(right_fd, &cur_potato, sizeof(cur_potato), 0);
            }else{
                int left_id = (id == 0) ? player_num - 1 : id - 1;
                std::cout << "Sending potato to " << left_id << std::endl;
                send(left_fd, &cur_potato, sizeof(cur_potato), 0);
            }
        }
        else if (cur_potato.hops == 1){
            cur_potato.track[cur_potato.index] = id;
            cur_potato.index++;
            cur_potato.hops--;
            std::cout << "I'm it" << std::endl;
            //send potato back to master
            send(master_fd, &cur_potato, sizeof(cur_potato), 0);
        }
        
        
    }
    //quit
    close(master_fd);
    close(right_fd);
    close(left_fd);
    
    return 0;
}
