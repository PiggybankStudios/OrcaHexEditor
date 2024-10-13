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
#include "open_file_dialog.cpp"

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
	
	if (KeyPressed(Key_Enter) && KeyDown(Key_Control))
	{
		HandleKeyExtended(Key_Enter);
		// if (!app->openFileDialog.isOpen)
		// {
		// 	LaunchOpenFileDialog(&app->openFileDialog, NewStr("D:/gamedev/"), mainHeap);
		// }
		// else
		// {
		// 	CloseOpenFileDialog(&app->openFileDialog);
		// }
		
		OC_FileDialogDesc_t dialogDesc = {};
		dialogDesc.kind      = OC_FILE_DIALOG_OPEN;
		dialogDesc.flags     = OC_FILE_DIALOG_FILES | OC_FILE_DIALOG_MULTIPLE;
		dialogDesc.title     = ToOcStr8(NewStr("Select a file to open in Hex Editor..."));
		dialogDesc.okLabel   = ToOcStr8(NewStr("Open"));
		dialogDesc.startPath = ToOcStr8(NewStr("F:/gamedev/"));
		OC_ListInit(&dialogDesc.filters.list);
		OC_Str8ListPush(&platform->ocArena, &dialogDesc.filters, NewStr("*.*"));
		OC_FileOpenWithDialogResult_t result = OC_FileOpenWithDialog(&platform->ocArena, OC_FILE_ACCESS_READ, OC_FILE_OPEN_NONE, &dialogDesc);
		if (result.button == OC_FILE_DIALOG_OK)
		{
			OC_ListFor(result.selection, selectedFile, oc_file_open_with_dialog_elt, listElt)
			{
				PrintLine_I("Selected %u", (u32)selectedFile->file.h);
				OC_FileClose(selectedFile->file);
			}
		}
		else
		{
			WriteLine_W("Open file cancelled");
		}
	}
	
	UpdateOpenFileDialog(&app->openFileDialog);
	
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
	
	RenderOpenFileDialog(&app->openFileDialog);
	
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
