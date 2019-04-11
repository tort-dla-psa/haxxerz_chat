#include <iostream>
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
		chat->write(mes);
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

int main(int argc, char* argv[]){
	if(argc!=3){
		std::cerr<<"provide ip and port!\n";
		return -1;
	}
	interface iface;
	iface.draw_mes("connecting\n");
	client cli;
	cli.connect(argv[1], atoi(argv[2]));
	iface.draw_mes("enter nickname\n");
	const auto nick = iface.get_input();
	cli.set_name(nick);
	bool end_requested = false;
	while(!end_requested){
		const auto messages = cli.get_mes_queue();
		for(const auto &mes:messages){
			iface.draw_mes(mes);
			if(mes == "/EXIT"){
				end_requested = true;
			}
		}
	}
	cli.disconnect();
}
