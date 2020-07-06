#pragma once
#include <cstring>
#include <string>
#include <vector>

enum class role{admin, user, muted, banned};

class message{
public:
    static const size_t header_len = sizeof(uint32_t); //data len

    message(){
        m_data.resize(header_len+1, '\0');
    }
    void set_str(const std::string &str){
        uint32_t len=str.size()+1;
        m_data.resize(header_len+len, '\0');
        char* ptr = &m_data[0];
        memcpy(ptr, (char*)&len, sizeof(len));
        memcpy(ptr+header_len, str.data(), str.size());
    }
    std::string get_str()const{
        return m_data.substr(header_len);
    }
    char* header(){
        return &m_data[0];
    }
    const char* header()const{
        return &m_data[0];
    }
    char* data(){
        return &m_data[0]+header_len;
    }
    const char* data()const{
        return &m_data[0]+header_len;
    }
    size_t bytelen()const{
        return m_data.size();
    }
    size_t datalen(){
        uint32_t len=0;
        memcpy((char*)&len, m_data.data(), header_len);
        m_data.resize(header_len+len, '\0');
        return len;
    }
protected:
    std::string m_data;
};
