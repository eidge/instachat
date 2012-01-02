#include "x.h"

using namespace std;
using boost::asio::ip::tcp;

//------------------------------Screen Methods----------------------------------//

bool Screen::is_there_one_screen = false;

int Screen::draw_main_menu()
{
	clear();
	
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	init_pair(3, COLOR_WHITE, COLOR_RED);
	init_pair(4, COLOR_WHITE, COLOR_BLACK);
	init_pair(5, COLOR_BLUE, COLOR_BLACK);

	//Print all asterisks in red
	attron(COLOR_PAIR(1));
	mvprintw((max_row-15)/2, (max_col-51)/2, "***************************************************");
	mvprintw((max_row-15)/2 + 4, (max_col-51)/2, "***************************************************");
	mvprintw((max_row-15)/2 + 6, (max_col-51)/2, "***************************************************");
	mvprintw((max_row-15)/2 + 14, (max_col-51)/2, "***************************************************");

	for(size_t i = 0; i < 15; ++i)
			if(i != 5)
				{
					mvprintw((max_row-15)/2 + i, (max_col-51)/2, "*");
					mvprintw((max_row-15)/2 + i, (max_col-51)/2 + 50, "*");
				}
	attroff(COLOR_PAIR(1));
	//Done with asterisks, printing strings, default option is number 1.

	draw_menu_choose_option(0);

	attron(COLOR_PAIR(2));
	mvprintw((max_row-15)/2 + 2, (max_col-app_name.size())/2, "%s", app_name.c_str());
	mvprintw((max_row-15)/2 + 14, (max_col-copyright.size())/2, "%s", copyright.c_str());
	attroff(COLOR_PAIR(2));
	
	refresh();
	//Done with drawing update screen.
	//Getting Input:
	int key_pressed;
	int cur_option = 0;

	while((key_pressed = getch()) != ENTER)
		{
			switch(key_pressed)
				{
					case '1':
						return 0;
						break;

					case '2':
						return 1;
						break;

					case '3':
						return 2;
						break;

					case '0':
						return 3;
						break;

					case KEY_DOWN:
						++cur_option;
						if(cur_option > 3)
							cur_option = 0;

						draw_menu_choose_option(cur_option);
						break;
													
					case KEY_UP:
						--cur_option;
						if(cur_option < 0)
							cur_option = 3;

						draw_menu_choose_option(cur_option);
						break;

					default:
						break;
				}
		}

	return cur_option;
}

void Screen::draw_menu_choose_option(int option)
{
	attron(COLOR_PAIR(4));
	mvprintw((max_row-15)/2 + 8, (max_col-12)/2, "%s", opt_1.c_str());
	mvprintw((max_row-15)/2 + 9, (max_col-12)/2, "%s", opt_2.c_str());
	mvprintw((max_row-15)/2 + 10, (max_col-12)/2, "%s", opt_3.c_str());
	mvprintw((max_row-15)/2 + 12, (max_col-12)/2, "%s", opt_4.c_str());
	attroff(COLOR_PAIR(4));

	attron(COLOR_PAIR(3));
	switch(option)
		{
			case 0:
				mvprintw((max_row-15)/2 + 8, (max_col-12)/2, "%s", opt_1.c_str());
				break;
			case 1:
				mvprintw((max_row-15)/2 + 9, (max_col-12)/2, "%s", opt_2.c_str());
				break;
			case 2:
				mvprintw((max_row-15)/2 + 10, (max_col-12)/2, "%s", opt_3.c_str());
				break;
			case 3:
				mvprintw((max_row-15)/2 + 12, (max_col-12)/2, "%s", opt_4.c_str());
				break;
			default:
				throw runtime_error("No option available.");
				break;
		}
	attroff(COLOR_PAIR(3));

	refresh();
}

void Screen::enter(int &cur_y, int &cur_x, int max_y = screen.get_max_y())//Data is definitive
{
	++cur_y;
	cur_x = 0;
}

void Screen::backspace(int &cur_y, int &cur_x, int min_y = 0)
{
	boost::unique_lock<boost::recursive_mutex> lock(Screen_Mutex);

	if(cur_x < 0)
		{
			--cur_y;
			cur_x = get_max_x();
		}
	else
		--cur_x;

	screen.print(cur_y, cur_x, ' ');
}
//------------------------------ChatZone Methods----------------------------------//
void ChatZone::put(int c, short color)
{
	if(is_full())
		return;

	if(x_cur == x_max)
		{
			++y_cur;
			x_cur = 0;
		}

	screen->print(y_cur, x_cur++, c, color);
	content[char_count++] = c;
}

void ChatZone::break_line()
{
	if(y_cur == y_max)
		return;
	
	for(int i = x_cur; i != x_max; ++i) //Fill end of line with spaces
		put(' ');
	
	++y_cur;
	x_cur = 0; 
}

void ChatZone::backspace()
{
	if(x_cur == 0)
		{
			if(y_cur == y_min)
				return;	
				
			--y_cur;
			x_cur = x_max-1;
		}
	else
		--x_cur;

	screen->print(y_cur, x_cur, ' ');
	--char_count;

}


//---------------------------------------DEV------------------------------------//


void options()
{
	string dev = "Options() was not implemented yet!";

	screen.clear();
	screen.print_in_middle(dev);
	screen.hold();
}




//-----------------------------------HOST CLASS------------------------------------//

//---------------------------------TALK METHODS-------------------------------------------//
void Talk::comm()
{
	//Wait for socket to be connected
	{
		boost::unique_lock<boost::mutex> lock(connectedMutex);
		is_connected.wait(lock);
	}

	try
	{

	}
	catch(boost::thread_interrupted &)
	{

	}

}

void Talk::keyboard_input()
{
	//Wait for socket to be connected
	{
		boost::unique_lock<boost::mutex> lock(connectedMutex);
		is_connected.wait(lock);
	}
	
	try
	{
		int c;
		c = getch();

		Input_Buf.push_back(c);
	}
	catch(boost::thread_interrupted &)
	{
	
	}
	
}

void Talk::operator()()
{
	screen.clear();
	screen.print_in_middle("Waiting for Client to connect...");
	screen.refresh();

	/* HERE
	make user able to cancel the connection process
	make sure that on cancel or error connecting everything is cleaned and main menu is presented again
	*/
	TalkSocket = Make_Connection(io, HSerror);

	if(HSerror){
		screen.clear();
		screen.print_in_middle("Error Connecting to client.");
		screen.refresh();

		getch();
		//Remember to kill threads!
		return;
	}

	screen.clear();
	screen.print_in_middle("Client connected.");
	screen.refresh();

	boost::this_thread::sleep(boost::posix_time::milliseconds(500));
	screen.clear();

	//print divisories
	screen.print(3,0, string(screen.get_max_x(), '*'));
	screen.print(screen.get_max_y() - 4,0, string(screen.get_max_x(), '*'));

	while(true)
	{
		
	}	
}