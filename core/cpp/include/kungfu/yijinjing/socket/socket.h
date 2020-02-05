//
// Created by qlu on 2019/7/15.
//

#ifndef KUNGFU_SOCKET_H
#define KUNGFU_SOCKET_H

#include "os.h"
#include <spdlog/spdlog.h>

namespace kungfu
{
    namespace yijinjing
    {
        namespace socket
        {
            class socket
            {
            public:
                socket(int domain, int type, int protocol = 0)
                {
                    sock_ = ::socket(domain, type, protocol);
                    if (sock_ < 0)
                    {
                        SPDLOG_ERROR("create socket error");
                    }
                }

                ~socket() { close(); }

                int sockfd() const { return sock_; }

                int make_non_blocking()
                {
                    int flags, s;
	                flags = fcntl(this->sockfd(), F_GETFL, 0);
                	if (flags == -1)
                	{
		                SPDLOG_ERROR("fcntl");
		                return -1;
	                }
	                flags |= O_NONBLOCK;
	                s = fcntl(this->sockfd(), F_SETFL, flags);
	                if (s == -1)
	                {
		                SPDLOG_ERROR("fcntl");
		                return -1;
	                }
                    return 0;
                }

                int make_reusable()
                {
                    int yes = 1;
                    return this->setsockopt(SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
                }

                int join_group(const std::string &local_address, const std::string &group_address)
                {
                    struct ip_mreq mreq;
                    memset(&mreq, 0, sizeof(struct ip_mreq));
                    mreq.imr_multiaddr.s_addr = inet_addr(group_address.c_str());
                    mreq.imr_interface.s_addr = inet_addr(local_address.c_str());

                    return this->setsockopt(IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(struct ip_mreq));
                }

                int bind(const struct sockaddr *addr, socklen_t addrlen)
                {
                    return ::bind(this->sockfd(), addr, addrlen);
                }

                int connect(const struct sockaddr *addr, socklen_t addrlen)
                {
                    return ::connect(this->sockfd(), addr, addrlen);
                }

                int getsockopt(int level, int optname, void *optval, socklen_t *optlen)
                {
                    return ::getsockopt(this->sockfd(), level, optname, optval, optlen);
                }

                int setsockopt(int level, int optname, const void *optval, socklen_t optlen)
                {
                    return ::setsockopt(this->sockfd(), level, optname, optval, optlen);
                }

                void close()
                {
                    ::close(this->sockfd());
                }

                int send(const char* data, size_t length) const
                {
                    return ::write(sock_, data, length);
                };

            private:
                int sock_;
            };

            typedef std::shared_ptr<socket> socket_ptr;

            class socket_factory
            {
            public:
                static socket_ptr tcp_client(const std::string &host, int port)
                {
                    int sock_type = SOCK_STREAM;
                    socket_ptr sock = std::make_shared<socket>(AF_INET, sock_type, 0);
                    if  (sock->sockfd() < 0)
                    {
                        return nullptr;
                    }
                    if (sock->make_reusable() != 0)
                    {
                        SPDLOG_ERROR("fail to allow port reuse");
                        return nullptr;
                    }
                    if (sock->make_non_blocking() != 0)
                    {
                        SPDLOG_ERROR("fail to set socket non-blocking");
                        return nullptr;
                    }

                    struct sockaddr_in serv_addr;
                    memset(&serv_addr, 0, sizeof(sockaddr_in));
                    struct hostent *server = gethostbyname(host.c_str());
                    serv_addr.sin_family = AF_INET;
                    bcopy(server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
                    serv_addr.sin_port = htons(port);

                    if (sock->connect((struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1 && errno != EINPROGRESS)
                    {
                        return nullptr;
                    }
                    else
                    {
                        return sock;
                    }
                }

                static socket_ptr udp_client(int listen_port)
                {
                    struct addrinfo hints;
                    struct addrinfo *result, *rp;
                    memset(&hints, 0, sizeof(struct addrinfo));
                    hints.ai_family = AF_UNSPEC;     /* Return IPv4 and IPv6 choices */
                    hints.ai_flags = AI_PASSIVE;     /* All interfaces */
                    hints.ai_socktype = SOCK_DGRAM;
                    int s = getaddrinfo(NULL, std::to_string(listen_port).c_str(), &hints, &result);
                    if (s != 0)
                    {
                        SPDLOG_ERROR("getaddrinfo: {}", gai_strerror(s));
                        return nullptr;
                    }

                    socket_ptr sock = nullptr;
                    for (rp = result; rp != NULL; rp = rp->ai_next)
                    {
                        sock = std::make_shared<socket>(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
                        if (sock->sockfd() == -1)
                            continue;
                        int s = sock->make_reusable();
                        if(s != 0)
                        {
                            SPDLOG_DEBUG("trying to set SO_REUSEADDR faild, ERRNO: {}|{}", errno, strerror(errno));
                            continue;
                        }
                        else
                        {
                            SPDLOG_INFO("trying to set SO_REUSEADDR succeed!");
                        }

                        s = sock->bind(rp->ai_addr, rp->ai_addrlen);
                        if (s == 0)
                        {
                            SPDLOG_INFO("success to bind to port {}", listen_port);
                            break;
                        }
                        else
                        {
                            SPDLOG_DEBUG("try to bind failed, ERRNO: {}|{}", errno, strerror(errno));
                            sock->close();
                        }
                    }

                    if (rp == NULL)
                    {
                        SPDLOG_ERROR("could not bind to port {}", listen_port);
                        return nullptr;
                    }

                    freeaddrinfo(result);

                    s = sock->make_non_blocking();
                    if (s == -1)
                    {
                        SPDLOG_ERROR("make socket non blocking failed, sockfd: {}", sock->sockfd());
                        sock->close();
                        return nullptr;
                    }
                    else
                    {
                        SPDLOG_INFO("success to set socket non-blocking");
                        return sock;
                    }
                }

                static socket_ptr udp_multicast_client(int listen_port, const std::string &local_address, const std::string &group_address)
                {
                    socket_ptr sock = udp_client(listen_port);
                    if (sock == nullptr)
                    {
                        return sock;
                    }
                    else
                    {
                        if (sock->join_group(local_address, group_address) == -1)
                        {
                            SPDLOG_ERROR("failed to setsockopt IP_ADD_MEMBERSHIP");
                            return nullptr;
                        }
                        else
                        {
                            SPDLOG_INFO("success to setsockopt IP_ADD_MEMBERSHIP");
                            return sock;
                        }
                    }
                }
            };
        }
    }
}

#endif //KUNGFU_SOCKET_H