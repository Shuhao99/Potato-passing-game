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
    std::string master_name = argv[1]; 
    //get master port number
    std::string master_port = argv[2];
    
    //build conncetion with master
    int master_fd = build_sender(master_name.c_str(), master_port.c_str());
    if (master_fd == -1)
    {
        std::cout << "Join failed, check your input!" << std::endl;
        return 1;
    }
    
    //receive id
    int id = -1;
    recv(master_fd, &id, sizeof(id), 0);
    
    //receive player number
    int player_num = 0;
    recv(master_fd, &player_num, sizeof(player_num), 0);
    
    //build listener
    int left_fd = build_listener("");
    
    //get local port number
    struct sockaddr_in local_addr;
    socklen_t local_addr_len = sizeof(local_addr);
    if (getsockname(left_fd, (struct sockaddr*)&local_addr, &local_addr_len) < 0) {
        std::cerr << "getsockname() error" << std::endl;
        close(left_fd);
        close(master_fd);
        return 1;
    }
    int left_port = ntohs(local_addr.sin_port);
    
    //send listening port number
    send(master_fd, &left_port, sizeof(left_port), 0);
    
    //receive right neighbor port
    int right_port = -1;
    recv(master_fd, &right_port, sizeof(right_port), 0);

    //receive right neighbor ip
    char buffer[BUFFER_SIZE];
    int rec_len = recv(master_fd, buffer, sizeof(buffer), 0);
    std::string right_ip(buffer, rec_len);
    
    //send id
    send(master_fd, &id, sizeof(id), 0);

    //build sender
    int right_fd = build_sender(right_ip.c_str(), std::to_string(right_port).c_str());

    //print connected
    std::cout << "Connected as player " << id 
    << " out of " << player_num << " total players\n";

    //wait for potato
    potato cur_potato = potato();
    while (true)
    {
        fd_set readfds;
        int nfds = max(master_fd, max(left_fd, right_fd));
        FD_ZERO(&readfds);

        FD_SET(master_fd, &readfds);
        FD_SET(left_fd, &readfds);
        FD_SET(right_fd, &readfds);

        select(nfds + 1, &readfds, NULL, NULL, NULL);
        if (FD_ISSET(master_fd, &readfds)){
            if(recv(master_fd, &cur_potato, sizeof(cur_potato), 0) < 0){
                break;
            }
        }
        else if (FD_ISSET(right_fd, &readfds)){
            if(recv(master_fd, &cur_potato, sizeof(cur_potato), 0) < 0){
                break;
            }
        }
        else if (FD_ISSET(left_fd, &readfds)){
            if(recv(master_fd, &cur_potato, sizeof(cur_potato), 0) < 0){
                break;
            }  
        }
        //count down and add self to potato
        if (cur_potato.hops > 1){
            cur_potato.track[cur_potato.index] = id;
            cur_potato.index++;
            cur_potato.hops--;

            //randomly pass to left or right
            srand((unsigned)time(NULL));
            int random_number = rand() % 2;
            if(rand){
                int right_id = (id + player_num + 1) % player_num;
                std::cout << "Sending potato to " << right_id << std::endl;
                send(right_fd, &cur_potato, sizeof(cur_potato), 0);
            }else{
                int left_id = (id + player_num - 1) % player_num;
                std::cout << "Sending potato to " << left_id << std::endl;
                send(left_fd, &cur_potato, sizeof(cur_potato), 0);
            }
        }
        if (cur_potato.hops == 1){
            cur_potato.track[cur_potato.index] = id;
            std::cout << "I'm it" << std::endl;
            //send potato back to master
            send(master_fd, &cur_potato, sizeof(cur_potato), 0);
        }
        if (cur_potato.hops <= 0){
            break;
        }
        
    }
    //quit
    close(master_fd);
    close(right_fd);
    close(left_fd);
    
    return 0;
}
