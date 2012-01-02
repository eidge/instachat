#include "InstaChat.hpp"

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
void Talk::sender()
{

	try
	{
		int s_buf2[256];
		size_t s_size2;
		
		//Start Connection
		{
			boost::mutex::scoped_lock sr_lock(socketMutex);
			TalkSocket = &Make_Connection(io, HSerror);
		}

		if(HSerror)
			{
				screen.clear();
				screen.print_in_middle("Error Connecting to client.");
				screen.refresh();

				getch();
				return;
			}

		//Connection established, notify main and receiver threads
		is_connected.notify_all();

		while(!HSerror)
			{
				{
					boost::mutex::scoped_lock sr_lock(send_buf_mutex_dev);
					has_data.wait(sr_lock);
					
					array_copy(s_buf, s_buf2, s_size);
					
					s_size2 = s_size;
					s_size = 0;
				}
				
				{
					zone2.put(s_buf2[1]);
					boost::mutex::scoped_lock sr_lock2(socketMutex);
					TalkSocket->write_some(boost::asio::buffer(s_buf2, s_size2), HSerror); //FIX ME: DATA MIGHT NOT BE SENT COMPLETLY! s_buf2+1 is being sent, why?
				}
			
			}
	}
	catch(boost::thread_interrupted &)
	{
		if(TalkSocket != 0)
			{
				TalkSocket->close();
				delete TalkSocket;
			}
	}

}

void Talk::receiver()
{
	//Wait for socket to be connected
	{
		boost::unique_lock<boost::mutex> lock(connectedMutex);
		is_connected.wait(lock);
	}
	
	try
	{
		int r_buf[256];
		size_t r_size = 0;
		
		//Receiving Half FIXME:
		while(1)
		{
			{
				boost::unique_lock<boost::mutex> lock(socketMutex);
				r_size = TalkSocket->available();
				if(r_size > 255)
					r_size = 255;
				r_size = TalkSocket->read_some(boost::asio::buffer(r_buf, r_size), HSerror);
			}

			{
				boost::unique_lock<boost::mutex> sr_lock(srMutex);
				for(size_t i = 0; i < r_size; ++i)
					{

						switch(char(r_buf[i]))//char does make it, but why does int sent differs from received int?
						{
							case ENTER:
								if(zone1.count() != 0)
								{
									const int *zone1buf = zone1.contents();
									for(size_t i = 0; i < zone1.count(); ++i)
										{						
											if(zone2.is_full())
												{
													vector<int> old_contents(zone2.contents() + zone2.max_x(), zone2.contents() + zone2.count());	
													zone2.reset();
													zone2.break_line();
													for(vector<int>::const_iterator i = old_contents.begin() + zone2.max_x() ; i != old_contents.end(); ++i) //start printing from line 1
														{
															zone2.put(*i);
														}
													
													zone2.clean_line(zone2.max_y() - 1);
												}

											zone2.put(zone1buf[i]);
										}
									
									zone2.break_line();

									zone1.clean_zone();
								}
								break;
							case BACKSPACE:
								zone1.backspace();
								break;
							default:
								zone1.put(r_buf[i]);			
								break;
						}
					}

				screen.refresh();
			}

			boost::this_thread::sleep(boost::posix_time::milliseconds(10)); //Prevents cpu overload, I hope
		}
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

	boost::thread_group data_streaming;
	data_streaming.create_thread(boost::bind(&Talk::sender, this));
	data_streaming.create_thread(boost::bind(&Talk::receiver, this));

	{
		boost::unique_lock<boost::mutex> lock(connectedMutex);
		is_connected.wait(lock); //Wait for client to connect: FIXME: THIS BLOCKS UNTIL A CLIENT CONNECTS!
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
		//LOCAL IN/OUTPUT
		
		int c = getch();
		//boost::this_thread::sleep(boost::posix_time::milliseconds(10));//Prevent overloading with to much fast input DEV
		if( !zone3.is_full() || ((c == BACKSPACE || c == ENTER) && !zone3.is_empty()) ) 
			{
				{
					boost::unique_lock<boost::mutex> sr_lock(send_buf_mutex_dev);
					s_buf[s_size++] = c;
				}

				if(s_size == 255) //Queue is full, connection problems
					throw InstaException("Connection interrupted!");
		
		
				has_data.notify_one();
			}

		if(c == '0')
			break;
		
		{
			
			switch(c)
				{
					case ENTER:
						if(zone3.count() != 0)
						{
							const int *zone3buf = zone3.contents();

							for(size_t i = 0; i < zone3.count(); ++i)
								{						
									if(zone2.is_full())
										{
											vector<int> old_contents(zone2.contents() + zone2.max_x(), zone2.contents() + zone2.count());	
											zone2.reset();
											zone2.break_line();		
											for(vector<int>::const_iterator i = old_contents.begin() + zone2.max_x() ; i != old_contents.end(); ++i) //start printing from line 1
												{
													zone2.put(*i);
												}
											zone2.clean_line(zone2.max_y() - 1);
										}

									zone2.put(zone3buf[i]);
								}
							zone2.break_line();
							zone3.clean_zone();
						}
						break;
					case BACKSPACE:
						zone3.backspace();
						break;
					default:
						zone3.put(c);					
						break;
				}
		}
	}

	data_streaming.interrupt_all(); //DEV MUST INTERRUPT FIRST THE RECEIVING PART OTHERWISE EXCEPTIONS MIGHT BE THROWN!
	data_streaming.join_all();
	
}