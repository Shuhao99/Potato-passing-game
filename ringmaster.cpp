#include "potato.h"
#include "socket.h"

#include <pthread.h>
#include <string>
#include <utility>
#include <cstdlib>
#include <ctime>
#include <algorithm>

void shut_down(int* socket_list, int size, int master_fd);

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        std::cout << "Usage: ringmaster <port_num> <num_players> <num_hops>" << std::endl;
        return 1;
    }
    //get port number 
    std::string listen_port(argv[1]);

    //get player number
    int player_num = 0;
    try
    {
        std::string temp(argv[2]);
        player_num = std::stoi(temp);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n' << "Please input legal player numebr!" << "\n";
        return 1;
    }
    if (player_num <= 1 || player_num > 1020){
        std::cout << player_num << std::endl;
        std::cout << "Please input legal player numebr!" << std::endl;
        return 1;
    }

    //get number hops
    int hops_num = -1;
    try
    {
        std::string temp(argv[3]);
        hops_num = std::stoi(temp);
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n' << "Please input legal hops numebr!" << "\n";
        return 1;
    }
    if (hops_num < 0 || player_num > 512){
        std::cout << "Please input legal hops numebr!" << std::endl;
        return 1;
    }

    potato cur_potato = potato(hops_num);

    //build listener
    int listen_fd = build_listener(listen_port.c_str());
    if (listen_fd == -1){
        std::cout << "Please input legal port numebr!" << std::endl;
        return 1;
    }
    
    //Game begin
    std::cout << "Potato Ringmaster" << std::endl;
    std::cout << "Players = " << player_num << std::endl;
    std::cout << "Hops = " << hops_num << std::endl;
    
    int player_fd_list[player_num];
    std::string ip_list[player_num];
    int port_list[player_num];
    
    //status >= 1 normal, status = 0, error
    int status = 1;
    //accept players
    for (int i = 0; i < player_num; i++) {
        struct sockaddr_storage socket_addr;

        socklen_t socket_addr_len = sizeof(socket_addr);

        int client_connect_fd = accept(listen_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
        
        struct sockaddr_in * addr = (struct sockaddr_in *)&socket_addr;
        
        //store ip
        std::string ip (inet_ntoa(addr->sin_addr));
        ip_list[i] = ip;

        //store fd
        player_fd_list[i] = client_connect_fd;

        //send player id 
        status = send(client_connect_fd, &i, sizeof(i), 0);
        
        //send player number
        status = send(client_connect_fd, &player_num, sizeof(player_num), 0);
        
        //receive listening port
        int port_num = 0;
        status = recv(client_connect_fd, &port_num, sizeof(port_num), 0);
        
        //store port number
        port_list[i] = port_num;  
        std::cout << "Player " << i << " is ready to play " << std::endl;
    }
    //send player id and neighbor info
    for (int i = 0; i < player_num; i++)
    {
        //send right_neighbor port
        if (i == player_num - 1)
        {
            status = send(player_fd_list[i], &port_list[0], sizeof(port_list[0]), 0);
        }
        else{
            status = send(player_fd_list[i], &port_list[i + 1], sizeof(port_list[i + 1]), 0);
        }
        
        //send right_neighbot ip
        if (i == player_num - 1)
        {
            status = send(player_fd_list[i], ip_list[0].c_str(), ip_list[0].length() + 1, 0);
        }
        else{
            status = send(player_fd_list[i], ip_list[i + 1].c_str(), ip_list[i + 1].length() + 1, 0);
        }
    }
    
    for (int i = 0; i < player_num; i++)
    {
        //receive id
        int id_confirm = -1;
        status = recv(player_fd_list[i], &id_confirm, sizeof(id_confirm), MSG_WAITALL);

        //set status as 1 or 0
        if (id_confirm != i){
            status = 0;
        }
    }

    for (int i = 0; i < player_num; i++)
    {
        int signal = 1;
        send(player_fd_list[i], &signal, sizeof(signal), 0);
    }
    
    if (status == 0)
    {
        shut_down(player_fd_list, player_num, listen_fd);
        return 1;
    }
    if (hops_num == 0)
    {
        //end send potato with hops = 0 and quit
        for (int i = 0; i < player_num; i++) {
            send(player_fd_list[i], &cur_potato, sizeof(cur_potato), 0);
        }
        std::cout << "Trace of potato:" << std::endl;
        for(int i = 0; i < cur_potato.index; i++){
            std::cout<< i << ',';
        }
        shut_down(player_fd_list, player_num, listen_fd);
        return 0;
    }
    
    //send out first potato
    srand((unsigned)time(NULL));
    int random_number = rand() % player_num;
    std::cout << "Ready to start the game, sending potato to player " << random_number << std::endl;

    send(player_fd_list[random_number], &cur_potato, sizeof(cur_potato), 0);

    //wait for potato
    fd_set readfds;
    int nfds = *max_element(player_fd_list, player_fd_list + player_num);

    FD_ZERO(&readfds);
    
    for (int i = 0; i < player_num; i++) {
      FD_SET(player_fd_list[i], &readfds);
    }

    select(nfds + 1, &readfds, NULL, NULL, NULL);
    for (int i = 0; i < player_num; i++) {
      if (FD_ISSET(player_fd_list[i], &readfds)) {
        recv(player_fd_list[i], &cur_potato, sizeof(cur_potato), MSG_WAITALL);
        break;
      }
    }
    
    //end send potato with hops = 0 and quit
    for (int i = 0; i < player_num; i++) {
        send(player_fd_list[i], &cur_potato, sizeof(cur_potato), 0);
    }
    std::cout << "Trace of potato:" << std::endl;
    for(int i = 0; i < cur_potato.index; i++){
        std::cout<< cur_potato.track[i] << ',';
    }

    shut_down(player_fd_list, player_num, listen_fd);
    return 0;
}

//close all fd when error occurs
void shut_down(int* socket_list, int size, int master_fd){
    for (int i = 0; i < size; i++)
    {
        if(socket_list[i] != 0){
            close(socket_list[i]);
        }
        close(master_fd);  
    }
};