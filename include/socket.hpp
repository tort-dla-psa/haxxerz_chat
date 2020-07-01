#pragma once
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <fcntl.h>

namespace io{

class socket{
public:
    using ret_t = long long;
    using len_t = unsigned long long;
protected:
    ret_t m_errno;
    int fd;
    struct sockaddr* addr;
    int m_domain;
    bool m_blocking;

    socket(int domain, int fd);
public:
    socket(int domain, int type, int portno, int protocol=0);
    socket(const socket &rhs);
    ~socket();

    void listen(int size);
    socket accept();
    void close();
    void set_blocking(bool flag);
    bool blocking()const;
    int err()const;

    template<class T>
    ret_t read(T &data);

    template<class T>
    ret_t write(const T &data);

    template<class T>
    ret_t read_all(T &data);

    template<class T>
    ret_t write_all(const T &data);

    ret_t read(char* data, const len_t &size);
    ret_t write(const char* data, const len_t &size);
    ret_t read_all(char* data, const len_t &size);
    ret_t write_all(const char* data, const len_t &size);

    socket& operator = (const socket &rhs);
};

inline socket::socket(int domain, int type, int portno, int protocol){
    if(domain == AF_INET){
        auto addr_loc = new struct sockaddr_in;
        int size = sizeof(struct sockaddr_in);
        this->addr = (struct sockaddr*)addr_loc;
        this->fd = ::socket(domain, type, protocol);
        addr_loc->sin_family = domain;
        addr_loc->sin_addr.s_addr = INADDR_ANY;
        addr_loc->sin_port = htons(portno);
        auto rc = ::bind(fd, addr, size);
    }else if(domain == AF_UNIX){
        struct sockaddr_in serv_addr, cli_addr;
    }
    this->m_domain = domain;
}

inline socket::socket(int domain, int fd){
    if(domain == AF_INET){
        auto addr_loc = new struct sockaddr_in;
        unsigned int size = sizeof(struct sockaddr_in);
        this->addr = (struct sockaddr*)addr_loc;
        this->fd = ::accept(fd, addr, &size);
    }
}

inline socket::socket(const socket &rhs){
    *this = rhs;
}

inline socket::~socket(){
    if(m_domain == AF_INET){
        auto addr_loc = (struct sockaddr_in*)addr;
        delete addr_loc;
    }
    ::close(fd);
}

inline void socket::listen(int size){
    ::listen(fd, size);
}

inline socket socket::accept(){
    return socket(this->m_domain, this->fd);
}

inline void socket::close(){
    ::shutdown(fd, SHUT_RDWR);
    ::close(fd);
}

inline void socket::set_blocking(bool flag){
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return;
    flags = flag ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    fcntl(fd, F_SETFL, flags);
    this->m_blocking = flag;
}

inline bool socket::blocking()const{
    return m_blocking;
}

inline int socket::err()const{
    return m_errno;
}

inline socket::ret_t socket::read(char* data, const len_t &size){
    auto rc = ::read(fd, data, size);
    if(rc < 0){
        m_errno = rc;
    }
    return rc;
}

inline socket::ret_t socket::write(const char* data, const len_t &size){
    auto rc = ::write(fd, data, size);
    if(rc < 0){
        m_errno = rc;
    }
    return rc;
}

inline socket::ret_t socket::read_all(char* data, const len_t &size){
    len_t i=0;
    int delta=0;
    for(; i<size; i+=delta) {
        delta = read(data+delta, size-delta);
        if(!m_blocking && delta <= 0){
            break;
        }
    }
    return i;
}

inline socket::ret_t socket::write_all(const char* data, const len_t &size){
    len_t i=0;
    ret_t delta=0;
    for( ;i<size; i+=delta) {
        delta = write(data+delta, size-delta);
        if(!m_blocking && delta <= 0){
            break;
        }
    }
    return i;
}

template<class T>
inline socket::ret_t socket::read(T &data){
    return this->read((char*)&data, sizeof(T));
}

template<class T>
inline socket::ret_t socket::write(const T &data){
    return this->write((char*)&data, sizeof(T));
}

template<class T>
inline socket::ret_t socket::read_all(T &data){
    return this->read_all((char*)&data, sizeof(T));
}

template<class T>
inline socket::ret_t socket::write_all(const T &data){
    return this->write_all((char*)&data, sizeof(T));
}

inline socket& socket::operator = (const socket &rhs){
    this->fd = rhs.fd;
    this->m_domain = rhs.m_domain;
    if(m_domain == AF_INET){
        auto temp = new struct sockaddr_in;
        *temp = *((struct sockaddr_in*)rhs.addr);
        this->addr = (struct sockaddr*)temp;
    }else if(m_domain == AF_UNIX){
    }
    return *this;
}

}
