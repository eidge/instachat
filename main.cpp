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

//TODO:
//
//1-Calibrate Keyboard on startup
//2-Pick a nick name
//3-Save preferences on disk
//7-Solve host blocking until a client connects


#include <iostream>
#include <cstring>

#include "InstaChat.hpp"

using namespace std;

Screen screen;


int main()
{

	int return_flag = EXIT_SUCCESS;
	init_pair(1, COLOR_GREEN, COLOR_BLACK);

	try
		{
			//Screen screen;
			screen.draw_init_screen();

			while(true)
			{
				switch(screen.draw_main_menu())
					{
						case 0:
							{
								Client b;
								b();
							}
							break;

						case 1:
							{
								Host a;
								a();
							}
							break;

						case 2:
							options();
							break;

						case 3:
							exit(0);
							break;
					}
			}
		}	
	catch(InstaException e)
		{
					screen.~Screen(); //Destruct Screen.
					cout << "Error: " << e.what() <<"\nENTER to continue...";
					getchar();
					cin.clear();
					cin.sync();
					return_flag = EXIT_FAILURE;
		}

	
	return return_flag;
}