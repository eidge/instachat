//*******************************
//*       InstaChat v0.1        *
//* Hugo Ribeira                *
//* 2011 September              *
//*******************************


//TODO:
//
//1-Calibrate Keyboard on startup
//2-Pick a nick name
//3-Save preferences on disk
//5-IP to connect to
//7-Host blocks until a client connects


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