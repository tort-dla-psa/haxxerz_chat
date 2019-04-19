#include <sys/time.h>
#include <sys/types.h>
#include <string>
#include <vector>
#include <memory>
#include "file.h"
#include "socket_op.h"

template <typename T> using vect = std::vector<T>;
template <typename T> using sptr = std::shared_ptr<T>;

using namespace IO;

namespace chatlib{
	inline void send(const std::string &mes, sptr<IO::socket> sock, socket_op s_op){
		const int length = mes.size();
		s_op.send(sock, length);
		s_op.send(sock, mes);
	}

	inline std::string recv(sptr<IO::socket> sock, socket_op s_op){
		int length;
		std::string mes;
		s_op.recv(sock, length);
		s_op.recv(sock, mes, length);
		return mes;
	}
	const auto cmd_exit = "/exit";
	const auto cmd_end = "/end";
};
