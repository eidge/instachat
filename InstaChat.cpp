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

void Screen::enter(int &cur_y, int &cur_x, int max_y = screen.get_max_y())
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
void Talk::keyboard_input()
{	
	try
	{
		int c;
		while(true)
		{
			c = getch();

			Input_Buf.push_back(c);

			{
				boost::unique_lock<boost::mutex> lock(client_quitsMutex);
				if(client_quits)
					break;
			}
		}
	}
	catch(boost::thread_interrupted &)
	{
	
	}
	
}

void Talk::operator()()
{
	Print_Connecting_Message();

	Make_Connection();

	// For implementing cancel ability
	//while(!connection_over()){
	//	if(Input_Buf.available()){
	//		vector<int> input = Input_Buf.pop_all();
	//		if(count(input.begin(), input.end(), 27)){
	//			this->~Talk();
	//			worker_threads.interrupt_all();
	//			return;
	//		}
	//	}
	//}


	if(bad()){
		screen.clear();
		screen.print_in_middle("Error Connecting to client.");
		screen.refresh();

		getch();
		return;
		}

	screen.clear();
	screen.print_in_middle("Client connected.");
	screen.refresh();

	boost::this_thread::sleep(boost::posix_time::milliseconds(500)); //Give the user time to read the notice
	screen.clear();

	//print divisories
	screen.print(3,0, string(screen.get_max_x(), '*'));
	screen.print(screen.get_max_y() - 4,0, string(screen.get_max_x(), '*'));

	//Start keyboard input:
	boost::thread keyboard_thread(boost::bind(&Talk::keyboard_input, this));

	
	vector<int> input;
	vector<int> output(256);
	size_t bytes;
	bool there_was_work;
	while(this->good())
	{
		there_was_work = false; //Did anything get sent or received?
		//Sending Part
		if(Input_Buf.available()){
			there_was_work = true;

			input = Input_Buf.pop_all();
			bytes = boost::asio::write(*TalkSocket, boost::asio::buffer(input), HSerror); // write() ensures that all data is sent or an error ocurred. Which 
																				 // would be caught in the following loop iteration.
			//Sort data and output to screen
			for(size_t input_char = 0; input_char != bytes/sizeof(int) ; ++input_char){ //bytes / so(int) = number of ints received
				switch(input.at(input_char))
						{
							case ESC:
								{
									boost::unique_lock<boost::mutex> lock(client_quitsMutex);
									client_quits = true;
								}
								break;								
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
								zone3.put(input.at(input_char));					
								break;
						}
			}
		}
		
		//Receiving Part:
		if(bytes = TalkSocket->available()){
			there_was_work = true;

			boost::asio::read(*TalkSocket, boost::asio::buffer(output, bytes), HSerror);

			//Sort Data and output to screen
			for(size_t output_char = 0; output_char != bytes/sizeof(int) ; ++output_char){
				switch(output.at(output_char))
						{
							case ESC:
								{
									boost::unique_lock<boost::mutex> lock(client_quitsMutex);
									client_quits = true;
								}
								break;	
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
								zone1.put(output.at(output_char));			
								break;
						}
			}
		}

		screen.refresh();

		bool client_quits_2;
		{
			boost::unique_lock<boost::mutex> lock(client_quitsMutex);
			client_quits_2 = client_quits;
		}

		if(client_quits_2){
			screen.clear();
			screen.print_in_middle("Chat Terminated.");
			screen.refresh();

			boost::this_thread::sleep(boost::posix_time::milliseconds(500));
			break;
		}

		if(!there_was_work)
			boost::this_thread::sleep(boost::posix_time::milliseconds(5)); //This hopefully prevents cpu overload
	}

	keyboard_thread.join();
}