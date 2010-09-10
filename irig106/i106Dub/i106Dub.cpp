// i106_dub.cpp : main project file.

#include "stdafx.h"
#include "AboutDub.h"
#include "SelectForm.h"

using namespace Irig106DotNet;
using namespace i106Dub;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew SelectForm());
	return 0;
}
