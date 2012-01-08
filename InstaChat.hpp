//********************************
//*       InstaChat v0.3         *
//* Copyright 2011, Hugo Ribeira *
//*                              *
//********************************

//This file is part of InstaChat.

//   InstaChat is free software: you can redistribute it and/or modify
//   it under the terms of the GNU General Public License as published by
//   the Free Software Foundation, either version 3 of the License, or
//   any later version.

//   InstaChat is distributed in the hope that it will be useful,
//   but WITHOUT ANY WARRANTY; without even the implied warranty of
//   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//   GNU General Public License for more details.

//   You should have received a copy of the GNU General Public License
//   along with InstaChat.  If not, see <http://www.gnu.org/licenses/>.

#ifndef WITH_CURSES
#define WITH_CURSES

#include <curses.h>
#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <algorithm>
#include <boost\thread.hpp>
#include <boost\asio.hpp>
#include <boost\array.hpp>

class Screen;
class MonitorVar;

extern Screen screen;

const char ENTER = 13, BACKSPACE = 8, ESC = 27;
const int DEFAULT_PORT = 4445;

const std::string app_name = "InstaChat v0.2";
const std::string opt_1 = "1 - Connect";
const std::string opt_2 = "2 - Host";
const std::string opt_3 = "3 - Options";
const std::string opt_4 = "0 - Quit";
const std::string copyright = "EidgeAre 2011";


//----------------------InstaChat Exception Class---------------------//
class InstaException
{
	std::string error;

	public:
		InstaException(){}
		InstaException(const std::string &err): error(err) {}
		virtual ~InstaException() {}

		std::string what() {return error;}		
};

//-------------------------InstaChat Functions----------------------//

void options();

//--------------------------------------Screen Class--------------------------------------------//

class Screen
{
	public:
		Screen()
			{
				if(is_there_one_screen) 
					throw std::runtime_error("There can only be one instance of Screen at a time.");

				init_screen();
				is_there_one_screen = true;

				init_pair(1, COLOR_RED, COLOR_BLACK);
				init_pair(2, COLOR_GREEN, COLOR_BLACK);
				init_pair(3, COLOR_WHITE, COLOR_RED);
				init_pair(4, COLOR_WHITE, COLOR_BLACK);
				init_pair(5, COLOR_BLUE, COLOR_BLACK);
			}
		~Screen() {endwin(); is_there_one_screen = false;}

		inline void init_screen();
		inline void draw_init_screen();
		int draw_main_menu();

		void print(int y, int x, std::string str, short color = 0) {boost::unique_lock<boost::recursive_mutex> lock(Screen_Mutex); (COLOR_PAIR(color));mvprintw(y, x, "%s", str.c_str());attroff(COLOR_PAIR(color));}
		void print(int y, int x, int c, short color = 0) {boost::unique_lock<boost::recursive_mutex> lock(Screen_Mutex); attron(COLOR_PAIR(color)); mvprintw(y, x, "%c", c);attroff(COLOR_PAIR(color));}
		void print_in_middle(std::string str) {boost::unique_lock<boost::recursive_mutex> lock(Screen_Mutex); print(max_row/2, (max_col - str.size())/2, str);}

		void enter(int &cur_y, int &cur_x, int max_y);
		void backspace(int &cur_y, int &cur_x, int min_y);

		void clear() {::clear();}
		void hold() {getch();}
		void refresh() {::refresh();}

		int get_max_x() {return max_col;}
		int get_max_y() {return max_row;}


	private:
		boost::recursive_mutex Screen_Mutex;
		//No copying
		Screen(const Screen &src);

		int max_col;
		int max_row;

		static bool is_there_one_screen;

		void draw_menu_choose_option(int option);
};

void Screen::init_screen()
{
	initscr();
	raw();
	noecho();
	keypad(stdscr, true);

	max_col = getmaxx(stdscr);
	max_row = getmaxy(stdscr);

	if(has_colors() == false)
		throw InstaException("Your terminal does not support InstaChat.");
	
	start_color();


	if(max_col < 16)
		throw InstaException("Your terminal is too small. Try to resize it before starting InstaChat.");
}

void Screen::draw_init_screen()
{
	curs_set(2); //Sets cursor to big white square
	const int millis = 200; //Duration of sleep time
	
	std::string init_msg("Welcome to InstaChat");

	attron(COLOR_PAIR(2));
	for(size_t i = 0; i != init_msg.size(); ++i)
		{
			mvprintw(max_row/2, (max_col - init_msg.size())/2 + i, "%c", *(init_msg.begin()+i));
			refresh();
			boost::this_thread::sleep(boost::posix_time::milliseconds(millis));
		}
	attroff(COLOR_PAIR(1));

	boost::this_thread::sleep(boost::posix_time::milliseconds(1000));

	curs_set(0); //Goodbye cursor
}

//--------------------------------------Zone Class----------------------------------------------//
class ChatZone
{
	Screen *screen;

	int *content;
	
	int x_max;

	int y_min;
	int y_max;

	int x_cur;
	int y_cur;

	size_t char_max;
	size_t char_count;


	public:
		ChatZone(Screen *scr, int min_y, int max_y, int max_char): 
			screen(scr), x_max(screen->get_max_x()),y_min(min_y), 
			y_max(max_y), x_cur(0), y_cur(y_min), char_max(max_char), char_count(0) {content = new int[max_char + 1];}
		~ChatZone(){delete [] content;}

		void put(int c, short color = 0);
		void backspace();

		void clean_line(int y){screen->print(y, 0, std::string(x_max, ' '));  screen->refresh();}
		void clean_zone(){reset(); screen->print(y_min, 0, std::string(x_max*(y_max-y_min), ' ')); screen->refresh();} //FIXME
		void break_line();

		bool is_full() {return char_count >= char_max;}
		bool is_empty() {return !char_count;}
		size_t count() {return char_count;}
		const int* contents() {return content;}

		int max_x() {return x_max;}
		int max_y() {return y_max;}
		int max_char() {return char_max;}
		void reset() {char_count = 0; y_cur = y_min, x_cur = 0;} //DEV must be private!
};


//------------------------IO Buffer----------------------------//
class IO_Buffer
{
	private:
		boost::array<int, 256> buffer;
		size_t current_position;

		boost::mutex buffer_mutex;

	public:
		IO_Buffer(): current_position(0) {}

		void push_back(int c) 
			{if(current_position == 256) throw InstaException("Connection Loss");
			boost::unique_lock<boost::mutex> lock(buffer_mutex); buffer.at(current_position++) = c;}

		std::vector<int> pop_all()
			{boost::unique_lock<boost::mutex> lock(buffer_mutex); 
			 std::vector<int> copied_buf(buffer.begin(), buffer.begin() + current_position); 
			 current_position = 0; return copied_buf;}

		bool available() {boost::unique_lock<boost::mutex> lock(buffer_mutex); return current_position;}
};

//------------------------Talk Class---------------------------//
class Talk
{		
	Talk(const Talk &src); //No copying
	Talk& operator=(const Talk &src); //No assigning

	bool bad() { return HSerror; }
	bool good() { return !HSerror; }

	bool client_quits;
	boost::mutex client_quitsMutex;

	//bool connection_over() {return connection_process_over;} //For implementing ability to cancel connection

	protected:
		boost::asio::io_service io;
		
		//Status
		//bool connection_process_over; //For implementing ability to cancel connection

		//Comm
		IO_Buffer Output_Buf;

		//Keyboard Input
		IO_Buffer Input_Buf;

		//Socket:
		boost::asio::ip::tcp::socket *TalkSocket;
		boost::mutex socketMutex;
		boost::system::error_code HSerror;

		//IN/OUTput
		ChatZone zone1;
		ChatZone zone2;
		ChatZone zone3;

		//Methods
		virtual void Make_Connection() = 0;
		virtual void Print_Connecting_Message() = 0;

		void array_copy(const int *arr_src, int *arr_dest, size_t arr_size) 
			{for(size_t i = 0; i < arr_size; ++i) *(arr_dest + i) = *(arr_src + i);}

		//Thread Functions
		void keyboard_input();
		

	public:
		Talk(): zone1(&screen, 0, 3, screen.get_max_x() * 3), client_quits(false), /*connection_process_over(0),*/
				zone2(&screen, 4, screen.get_max_y() - 4, screen.get_max_x() * (screen.get_max_y() - 4 - 4)), TalkSocket(0),
				zone3(&screen, screen.get_max_y() - 3, screen.get_max_y(), screen.get_max_x() * 3) {}
		
		virtual ~Talk() { if(TalkSocket) {if(TalkSocket->is_open())TalkSocket->close(); delete TalkSocket;} }
				
		void operator()(); //Main Thread
};

//--------------------------------Host Class---------------------------------------//
class Host : public Talk
{
	void Make_Connection()
		{
			boost::unique_lock<boost::mutex> lock(socketMutex);

			boost::asio::ip::tcp::socket *HostSocket = new boost::asio::ip::tcp::socket(io);
			boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), DEFAULT_PORT));

			acceptor.accept(*HostSocket, HSerror);

			TalkSocket = HostSocket;

			//connection_process_over = true; //For implementing ability to cancel connection
		}

	virtual void Print_Connecting_Message()
		{
			screen.clear();
			screen.print_in_middle("Waiting for Client to connect...");
			screen.print(screen.get_max_y() - 1, 0 , "(Press Escape to Cancel)", 1);
			screen.refresh();
		}
};

//--------------------------------Client Class------------------------------------//
class Client : public Talk
{
		void Make_Connection()
		{
			boost::unique_lock<boost::mutex> lock(socketMutex);

			boost::asio::ip::tcp::socket *HostSocket = new boost::asio::ip::tcp::socket(io);
			boost::asio::ip::tcp::resolver resolver(io);
			std::string ip = get_ip();
			boost::asio::ip::tcp::resolver::query query(ip, "4445");//FIXME must ask for port
			boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

			boost::asio::connect(*HostSocket, iterator, HSerror);

			TalkSocket = HostSocket;


			//connection_process_over = true;

		}

		std::string get_ip()
		{
			std::string ip;

			screen.clear();
			screen.print_in_middle("Please enter the IP adress to connect to.");
			ChatZone IpField(&screen, screen.get_max_y() - 1, screen.get_max_y(), screen.get_max_x());

			IpField.put('I');
			IpField.put('P');
			IpField.put(':');
			IpField.put(' ');

			int key;
			while((key = getch()) != ENTER)
				{
				switch(key)
					{								
							case BACKSPACE:
								{
									if(ip.empty())
										break;
									IpField.backspace();
									ip.pop_back();
									break;
								}
							default:
								{
									IpField.put(key);
									ip.push_back(key);		
									break;
								}
					}
				}

			return ip;
				
		}

	virtual void Print_Connecting_Message()
		{
			return;
		}
};

#endif