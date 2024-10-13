/*
File:   open_file_dialog.cpp
Author: Taylor Robbins
Date:   10\12\2024
Description: 
	** Holds functions that manage displaying a dialog for opening a file to view
	** in the hex editor. This might get replaced later with a OS level file
	** window but Orca doesn't have support for that right now
*/

#define OPEN_FILE_DIALOG_SIZE          NewVec2(800, 600)
#define OPEN_FILE_DIALOG_DEF_FONT_SIZE 20
#define OPEN_FILE_DIALOG_SCROLL_SPEED  20
#define OPEN_FILE_DIALOG_SCROLL_LAG    10 //divisor

void CloseOpenFileDialog(OpenFileDialog_t* dialog)
{
	if (dialog->isOpen)
	{
		FreeString(dialog->allocArena, &dialog->currentPath);
		VarArrayLoop(&dialog->files, fIndex)
		{
			VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
			FreeString(dialog->allocArena, &file->fileName);
		}
		FreeVarArray(&dialog->files);
		dialog->isOpen = false;
	}
}

void OpenFileDialogPopulateFiles(OpenFileDialog_t* dialog)
{
	VarArrayLoop(&dialog->files, fIndex)
	{
		VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
		FreeString(dialog->allocArena, &file->fileName);
	}
	VarArrayClear(&dialog->files);
	
	
}

void LaunchOpenFileDialog(OpenFileDialog_t* dialog, MyStr_t startingFolder, MemArena_t* memArena)
{
	NotNull2(dialog, memArena);
	NotEmptyStr(&startingFolder);
	ClearPointer(dialog);
	dialog->allocArena = memArena;
	
	if (StrEndsWith(startingFolder, "/") || StrEndsWith(startingFolder, "\\")) { startingFolder.length--; }
	dialog->currentPath = AllocString(dialog->allocArena, &startingFolder);
	
	dialog->isOpen = true;
	dialog->primarySelectedIndex = -1;
	dialog->font = &platform->debugFont;
	dialog->fontSize = OPEN_FILE_DIALOG_DEF_FONT_SIZE;
	CreateVarArray(&dialog->files, dialog->allocArena, sizeof(OpenFileDialogFile_t));
}

//NOTE: Check dialog->numSelectedFiles to know how many times you should call this
MyStr_t GetOpenFileDialogSelectedPath(const OpenFileDialog_t* dialog, MemArena_t* memArena, u64 selectionIndex = 0)
{
	NotNull2(dialog, memArena);
	Assert(selectionIndex < dialog->numSelectedFiles);
	u64 currentIndex = 0;
	VarArrayLoop(&dialog->files, fIndex)
	{
		VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
		if (file->isSelected)
		{
			if (currentIndex >= selectionIndex)
			{
				return PrintInArenaStr(memArena, "%.*s/%.*s", StrPrint(dialog->currentPath), StrPrint(file->fileName));
			}
			currentIndex++;
		}
	}
	AssertMsg(false, "numSelectedFiles in OpenFileDialog_t didn't match actual isSelected count in VarArray"); //should be unreachable
	return MyStr_Empty;
}

void LayoutOpenFileDialog(OpenFileDialog_t* dialog)
{
	NotNull2(dialog, dialog->font);
	
	OC_FontMetrics_t fontMetrics = OC_FontGetMetrics(*dialog->font, dialog->fontSize);
	
	dialog->mainRec.size = OPEN_FILE_DIALOG_SIZE;
	if (dialog->mainRec.width > ScreenSize.width) { dialog->mainRec.width = ScreenSize.width; }
	if (dialog->mainRec.height > ScreenSize.height) { dialog->mainRec.height = ScreenSize.height; }
	dialog->mainRec.topLeft = ScreenSize/2 - dialog->mainRec.size/2;
	RecAlign(&dialog->mainRec);
	
	// oc_rect ink;
	// oc_rect logical;
	// oc_vec2 advance;
	OC_TextMetrics_t currentPathMetrics = OC_FontTextMetrics(*dialog->font, dialog->fontSize, ToOcStr8(dialog->currentPath));
	dialog->currentPathRec.topLeft = Vec2_Zero;
	dialog->currentPathRec.height = currentPathMetrics.logical.h;
	dialog->currentPathRec.width = dialog->mainRec.width;
	RecAlign(&dialog->currentPathRec);
	
	dialog->fileViewRec = NewRec(0, dialog->currentPathRec.height, dialog->mainRec.width, dialog->mainRec.height - dialog->currentPathRec.height);
	RecAlign(&dialog->fileViewRec);
	
	dialog->filesSize = Vec2_Zero;
	VarArrayLoop(&dialog->files, fIndex)
	{
		VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
		
		file->mainRec.y = dialog->filesSize.height;
		file->mainRec.x = 0;
		OC_TextMetrics_t fileNameMetrics = OC_FontTextMetrics(*dialog->font, dialog->fontSize, ToOcStr8(file->fileName));
		file->textSize = NewVec2(fileNameMetrics.logical.w, fileNameMetrics.logical.h);
		file->mainRec.width = dialog->fileViewRec.width;
		file->mainRec.height = file->textSize.height;
		RecAlign(&file->mainRec);
		
		file->iconRec.topLeft = Vec2_Zero;
		file->iconRec.size = Vec2Fill(file->mainRec.height);
		RecAlign(&file->iconRec);
		
		file->textPos.x = file->iconRec.x + file->iconRec.width + 2;
		file->textPos.y = file->mainRec.height - fontMetrics.descent;
		Vec2Align(&file->textPos);
		
		dialog->filesSize.width = MaxR32(dialog->filesSize.width, file->mainRec.width);
		dialog->filesSize.height += file->mainRec.height;
	}
	
	dialog->maxScroll = dialog->filesSize - dialog->fileViewRec.size;
	if (dialog->maxScroll.x < 0) { dialog->maxScroll.x = 0; }
	if (dialog->maxScroll.y < 0) { dialog->maxScroll.y = 0; }
	dialog->scroll.x = ClampR32(dialog->scroll.x, 0, dialog->maxScroll.x);
	dialog->scroll.y = ClampR32(dialog->scroll.y, 0, dialog->maxScroll.y);
	dialog->scrollGoto.x = ClampR32(dialog->scrollGoto.x, 0, dialog->maxScroll.x);
	dialog->scrollGoto.y = ClampR32(dialog->scrollGoto.y, 0, dialog->maxScroll.y);
}

void UpdateOpenFileDialog(OpenFileDialog_t* dialog)
{
	NotNull(dialog);
	if (!dialog->isOpen) { return; }
	LayoutOpenFileDialog(dialog);
	
	rec fileViewRec = dialog->fileViewRec + dialog->mainRec.topLeft;
	
	//TODO: Add support for scrolling until the selected item is entirely inside the fileViewRec
	
	// +==============================+
	// |     Handle Scroll Wheel      |
	// +==============================+
	if (IsInsideRec(fileViewRec, MousePos))
	{
		//TODO: Change this into proper input functions and handling when that is set up in oc_input_api.cpp
		if (appInput->scrollDelta.x != 0) { dialog->scrollGoto.x -= appInput->scrollDelta.x * OPEN_FILE_DIALOG_SCROLL_SPEED; }
		if (appInput->scrollDelta.y != 0) { dialog->scrollGoto.y -= appInput->scrollDelta.y * OPEN_FILE_DIALOG_SCROLL_SPEED; }
	}
	
	// +==============================+
	// |        Update Scroll         |
	// +==============================+
	v2 scrollDelta = dialog->scrollGoto - dialog->scroll;
	if (Vec2Length(scrollDelta) > 1)
	{
		dialog->scroll += scrollDelta / OPEN_FILE_DIALOG_SCROLL_LAG;
	}
	else
	{
		dialog->scroll = dialog->scrollGoto;
	}
	
	LayoutOpenFileDialog(dialog);
	
	// +==============================+
	// |         Update Files         |
	// +==============================+
	v2 filesOffset = dialog->mainRec.topLeft + dialog->fileViewRec.topLeft - dialog->scroll;
	VarArrayLoop(&dialog->files, fIndex)
	{
		VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
		rec fileMainRec = file->mainRec + filesOffset;
		if (IsInsideRec(fileViewRec, MousePos) && IsInsideRec(fileMainRec, MousePos))
		{
			if (MousePressed(MouseBtn_Left))
			{
				if (KeyDown(Key_Control))
				{
					//Toggle selection on the current file, set primary index only if it's the first file to be selected
					if (dialog->numSelectedFiles == 0) { dialog->primarySelectedIndex = (u64)fIndex; }
					file->isSelected = !file->isSelected;
					if (file->isSelected) { dialog->numSelectedFiles++; }
					else { dialog->numSelectedFiles--; }
				}
				else if (KeyDown(Key_Shift) && dialog->primarySelectedIndex >= 0)
				{
					//Select all files between the current primarySelectedIndex and this file
					dialog->numSelectedFiles = 0;
					u64 minIndex = MinU64(fIndex, (u64)dialog->primarySelectedIndex);
					u64 maxIndex = MaxU64(fIndex, (u64)dialog->primarySelectedIndex);
					VarArrayLoop(&dialog->files, fIndex2)
					{
						VarArrayLoopGet(OpenFileDialogFile_t, otherFile, &dialog->files, fIndex2);
						if (fIndex2 >= minIndex && fIndex2 <= maxIndex) { otherFile->isSelected = true; dialog->numSelectedFiles++; }
						else { otherFile->isSelected = false; }
					}
				}
				else
				{
					//Deselect all other files, select only this one
					VarArrayLoop(&dialog->files, fIndex2)
					{
						VarArrayLoopGet(OpenFileDialogFile_t, otherFile, &dialog->files, fIndex2);
						otherFile->isSelected = false;
					}
					file->isSelected = true;
					dialog->primarySelectedIndex = (i64)fIndex;
					dialog->numSelectedFiles = 1;
				}
			}
		}
	}
}

void RenderOpenFileDialog(OpenFileDialog_t* dialog)
{
	NotNull(dialog);
	if (!dialog->isOpen) { return; }
	LayoutOpenFileDialog(dialog);
	
	OC_SetColor(MonokaiBack);
	OC_RectangleFill(dialog->mainRec);
	
	OC_SetColor(MonokaiDarkGray);
	OC_RectangleFill(dialog->currentPathRec + dialog->mainRec.topLeft);
	
	rec fileViewRec = dialog->fileViewRec + dialog->mainRec.topLeft;
	OC_ClipPush(fileViewRec);
	
	v2 filesOffset = dialog->mainRec.topLeft + dialog->fileViewRec.topLeft - dialog->scroll;
	VarArrayLoop(&dialog->files, fIndex)
	{
		VarArrayLoopGet(OpenFileDialogFile_t, file, &dialog->files, fIndex);
		rec fileMainRec = file->mainRec + filesOffset;
		if (RecsIntersect(fileViewRec, fileMainRec))
		{
			Color_t backColor = Transparent;
			if (file->isSelected)
			{
				backColor = MonokaiLightBlue;
			}	
			else if (IsInsideRec(fileViewRec, MousePos) && IsInsideRec(fileMainRec, MousePos))
			{
				backColor = ColorTransparent(MonokaiLightBlue, 0.5f);
			}
			OC_SetColor(backColor);
			OC_RectangleFill(fileMainRec);
			
			OC_TextFill(file->textPos + fileMainRec.topLeft, file->fileName);
		}
	}
	
	OC_ClipPop();
}
