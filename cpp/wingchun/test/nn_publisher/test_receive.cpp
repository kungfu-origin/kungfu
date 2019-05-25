//
// Created by PolarAir on 2019-04-09.
//

#include <nanomsg/nn.h>
#include <nanomsg/pubsub.h>
#include <iostream>

using namespace std;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        cerr << "invalid arguments!" << endl;
    }

    int socket = nn_socket(AF_SP, NN_SUB);
    nn_connect(socket, argv[1]);
    nn_setsockopt(socket, NN_SUB, NN_SUB_SUBSCRIBE, "", 0);

    while (true)
    {
        char* buf = nullptr;
        int rc = nn_recv(socket, &buf, NN_MSG, NN_DONTWAIT);

        if (rc > 0)
        {
            printf("%s\n", buf);
            nn_freemsg(buf);
        }
    }

    return 0;
}