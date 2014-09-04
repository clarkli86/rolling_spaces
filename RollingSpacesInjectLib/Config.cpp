#include "stdafx.h"
#include "Config.h"

CConfig::CConfig ()
{
	rowCount = 2;
	columnCount = 2;
	rollingSpacesAreaWidth = 50;
}

UINT CConfig::GetRowCount() 
{
	return rowCount;
}

UINT CConfig::GetColumnCount() 
{
	return columnCount;
}

UINT CConfig::GetRollingSpacesAreaWidth()
{
	return rollingSpacesAreaWidth;
}