#include <iostream>
#include <thread>
#include <atomic>
#include <ncurses.h>
#include "cli.h"

class window{
	WINDOW* main;
	WINDOW*	second;
	int x,y,w,h;
	void _set_updated(){	wnoutrefresh(second); }
public:
	window(int x, int y, int w, int h, bool scroll){
		main = newwin(h, w, y, x);
		box(main,0,0);
		second = derwin(main, h-2, w-2, 1, 1);
		scrollok(second, scroll);
		wnoutrefresh(main);
		_set_updated();
	};
	~window(){
		delwin(second);
		delwin(main);
	}
	void update(){	doupdate(); }
	void write(const std::string& mes){
		wprintw(second, mes.c_str());
		_set_updated();
	}
	void clear(){
		werase(second);
		_set_updated();
	}
	int get_char(){	return wgetch(second); }
};

class interface{
	std::unique_ptr<window> chat, input, users;
public:
	interface(){
		initscr();
		keypad(stdscr, true);
		int w,h;
		getmaxyx(stdscr, h, w);
		chat = std::make_unique<window>(0, 0, w, h-3, true);
		input = std::make_unique<window>(0, h-3, w, 3, false);
		chat->update();
		input->update();
	}
	~interface(){
		endwin();
	}
	void draw_mes(const std::string &mes){
		if(mes.back() == '\n'){
			chat->write(mes);
		}else{
			chat->write(mes + '\n');
		}
		chat->update();
	}
	std::string get_input(){
		std::string mes;
		int ch;
		while ((ch = input->get_char()) != '\n') {
			mes.push_back(ch);
		}
		input->clear();
		input->update();
		return mes;
	}
};

std::atomic_bool end_requested;

void _recieve(sptr<client> cli, sptr<interface> iface){
	while(!end_requested){
		const auto messages = cli->get_mes_queue();
		for(const auto &mes:messages){
			iface->draw_mes(mes);
		}
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}

int main(int argc, char* argv[]){
	if(argc != 3){
		std::cerr<<"provide ip and port!\n";
		return -1;
	}
	sptr<interface> iface(new interface());
	iface->draw_mes("connecting\n");
	sptr<client> cli(new client());
	cli->connect(argv[1], atoi(argv[2]));
	iface->draw_mes("enter nickname\n");
	const auto nick = iface->get_input();
	cli->set_name(nick);
	cli->send(nick);
	std::thread recv_thread(_recieve, cli, iface);
	end_requested = false;
	while(!end_requested){
		std::string mes = iface->get_input();
		if(mes.empty()){
			continue;
		}
		cli->send(mes);
		if(mes == "/EXIT"){
			end_requested = true;
		}
	}
	if(recv_thread.joinable()){
		recv_thread.join();
	}
	cli->disconnect();
}
