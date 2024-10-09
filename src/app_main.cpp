/*
File:   main.cpp
Author: Taylor Robbins
Date:   09\21\2024
Description: 
	** This is the main file that is #included by oc_main.cpp and it includes all
	** other application specific files
*/

// +--------------------------------------------------------------+
// |                   Application Header Files                   |
// +--------------------------------------------------------------+
#include "version.h"
#include "app_main.h"

// +--------------------------------------------------------------+
// |                     Application Globals                      |
// +--------------------------------------------------------------+
AppState_t* app = nullptr;

// +--------------------------------------------------------------+
// |                   Application Source Files                   |
// +--------------------------------------------------------------+
//TODO: Any files we want to include?

// +--------------------------------------------------------------+
// |                           AppInit                            |
// +--------------------------------------------------------------+
void AppInit()
{
	app = AllocStruct(mainHeap, AppState_t);
	NotNull(app);
	ClearPointer(app);
	
	CreateRandomSeries(&app->rand);
	SeedRandomSeriesU64(&app->rand, (u64)OC_ClockTime(OC_CLOCK_DATE));
}

// +--------------------------------------------------------------+
// |                      AppUpdateAndRender                      |
// +--------------------------------------------------------------+
void AppUpdateAndRender()
{
	MemArena_t* scratch = GetScratchArena();
	
	OC_UiSetContext(&platform->ui);
	OC_CanvasContextSelect(platform->canvasContext);
	OC_SetColor(MonokaiBack);
	OC_Clear();
	
	OC_UiStyle_t uiStyle = {};
	uiStyle.font = platform->debugFont;
	OC_UiFrame(ScreenSize, &uiStyle, OC_UI_STYLE_FONT)
	{
		OC_UiMenuBar("menu_bar")
		{
			OC_UiMenu("File")
			{
				if (OC_UiMenuButton("Exit").pressed)
				{
					OC_RequestQuit();
				}
			}
		}
	}
	
	OC_SetFont(platform->debugFont);
	OC_SetFontSize(18);
	OC_SetColor(White);
	OC_TextFill(8, ScreenSize.height - 20 - 8, PrintInArenaStr(scratch, "App Version: v%d.%d(%03d)", APP_VERSION_MAJOR, APP_VERSION_MINOR, APP_VERSION_BUILD));
	OC_TextFill(8, ScreenSize.height - 8, PrintInArenaStr(scratch, "Orca Version: %s", ORCA_VERSION));
	
	OC_UiDraw();
	
	OC_CanvasRender(platform->renderer, platform->canvasContext, platform->surface);
	OC_CanvasPresent(platform->renderer, platform->surface);
	
	FreeScratchArena(scratch);
}

void GyLibAssertFailure(const char* filePath, int lineNumber, const char* funcName, const char* expressionStr, const char* messageStr)
{
	//TODO: Note we don't REALLY need this print out, since the Orca level assertion also does a log,
	//but this will serve as a placeholder until we decide more about error handling and log storage/routing
	if (messageStr != nullptr && messageStr[0] != '\0')
	{
		PrintLine_E("Assertion Failed! %s", messageStr);
		PrintLine_E("\tin %s:%d in function %s!", filePath, lineNumber, funcName);
	}
	else
	{
		PrintLine_E("Assertion Failed! (%s) is not true", expressionStr);
		PrintLine_E("\tin %s:%d in function %s!", filePath, lineNumber, funcName);
	}
}
