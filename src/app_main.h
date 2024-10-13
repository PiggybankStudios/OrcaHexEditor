/*
File:   main.h
Author: Taylor Robbins
Date:   09\22\2024
*/

#ifndef _MAIN_H
#define _MAIN_H

enum OpenFileDialogResult_t
{
	OpenFileDialogResult_None = 0,
	OpenFileDialogResult_Escape,
	OpenFileDialogResult_Cancel,
	OpenFileDialogResult_Ok,
	OpenFileDialogResult_NumValues,
};
const char* GetOpenFileDialogResultStr(OpenFileDialogResult_t enumValue)
{
	switch (enumValue)
	{
		case OpenFileDialogResult_None:   return "None";
		case OpenFileDialogResult_Escape: return "Escape";
		case OpenFileDialogResult_Cancel: return "Cancel";
		case OpenFileDialogResult_Ok:     return "Ok";
		default: return "Unknown";
	}
}

struct OpenFileDialogFile_t
{
	MyStr_t fileName;
	
	bool isSelected;
	
	rec mainRec;
	v2 textSize;
	rec iconRec; //relative to mainRec
	v2 textPos; //relative to mainRec
};

struct OpenFileDialog_t
{
	MemArena_t* allocArena;
	bool isOpen;
	MyStr_t currentPath; //no trailing slash
	r32 fontSize;
	OC_Font_t* font;
	
	u64 numSelectedFiles;
	i64 primarySelectedIndex;
	VarArray_t files; //OpenFileDialogFile_t
	
	OpenFileDialogResult_t result;
	
	v2 scroll;
	v2 scrollGoto;
	v2 maxScroll;
	
	rec mainRec;
	v2 filesSize;
	rec currentPathRec; //relative to mainRec
	rec fileViewRec; //relative to mainRec
};

struct AppState_t
{
	RandomSeries_t rand;
	
	OpenFileDialog_t openFileDialog;
};

#endif //  _MAIN_H
