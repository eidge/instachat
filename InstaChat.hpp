#ifndef WITH_CURSES
#define WITH_CURSES

#include <curses.h>
#include <iostream>
#include <string>
#include <cstring>
#include <boost\thread.hpp>
#include <boost\asio.hpp>

class Screen;
class MonitorVar;

extern Screen screen;

const char ENTER = 13, BACKSPACE = 8;
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

		void print(int y, int x, std::string str, short color = 0) {attron(COLOR_PAIR(color));mvprintw(y, x, "%s", str.c_str());attroff(COLOR_PAIR(color));}
		void print(int y, int x, int c, short color = 0) {attron(COLOR_PAIR(color)); mvprintw(y, x, "%c", c);attroff(COLOR_PAIR(color));}
		void print_in_middle(std::string str) {print(max_row/2, (max_col - str.size())/2, str);}

		void enter(int &cur_y, int &cur_x, int max_y);
		void backspace(int &cur_y, int &cur_x, int min_y);

		void clear() {::clear();}
		void hold() {getch();}
		void refresh() {::refresh();}

		int get_max_x() {return max_col;}
		int get_max_y() {return max_row;}


	private:
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



//------------------------Talk Class---------------------------//
class Talk
{		
	Talk(const Talk &src); //No copying
	Talk& operator=(const Talk &src); //No assigning

	protected:
		boost::asio::io_service io;

		int s_buf[256];
		int s_size;
		boost::mutex srMutex;
		boost::condition_variable has_data;

		//DEV
		boost::mutex send_buf_mutex_dev;
		
		//\DEV

		boost::mutex connectedMutex;
		boost::condition_variable is_connected;

		boost::asio::ip::tcp::socket *TalkSocket;
		boost::mutex socketMutex;
		boost::system::error_code HSerror;

		//IN/OUTput
		ChatZone zone1;
		ChatZone zone2;
		ChatZone zone3;

		//Methods
		virtual boost::asio::ip::tcp::socket& Make_Connection(boost::asio::io_service &io, boost::system::error_code &HSerror) = 0;

		void array_copy(const int *arr_src, int *arr_dest, size_t arr_size) 
			{for(size_t i = 0; i < arr_size; ++i) *(arr_dest + i) = *(arr_src + i);}

		//Thread Functions
		void sender();
		void receiver();
		

	public:
		Talk(): s_size(0), zone1(&screen, 0, 3, screen.get_max_x() * 3),
				zone2(&screen, 4, screen.get_max_y() - 4, screen.get_max_x() * (screen.get_max_y() - 4 - 4)), TalkSocket(0),
				zone3(&screen, screen.get_max_y() - 3, screen.get_max_y(), screen.get_max_x() * 3) {}
				
		void operator()(); //Main Thread
};

//--------------------------------Host Class---------------------------------------//
class Host : public Talk
{
	boost::asio::ip::tcp::socket& Make_Connection(boost::asio::io_service &io, boost::system::error_code &HSerror)
		{
			boost::asio::ip::tcp::socket *HostSocket = new boost::asio::ip::tcp::socket(io);
			boost::asio::ip::tcp::acceptor acceptor(io, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), DEFAULT_PORT));

			acceptor.accept(*HostSocket, HSerror);

			return *HostSocket;
		}
};

//--------------------------------Client Class------------------------------------//
class Client : public Talk
{
		boost::asio::ip::tcp::socket& Make_Connection(boost::asio::io_service &io, boost::system::error_code &HSerror)
		{
			boost::asio::ip::tcp::socket *HostSocket = new boost::asio::ip::tcp::socket(io);
			boost::asio::ip::tcp::resolver resolver(io);
			std::string ip = get_ip();
			boost::asio::ip::tcp::resolver::query query(ip, "4445");//FIXME must ask for port
			boost::asio::ip::tcp::resolver::iterator iterator = resolver.resolve(query);

			boost::asio::connect(*HostSocket, iterator, HSerror);

			return *HostSocket;
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
};

#endif