#ifndef C_CONFIG_H
#define C_CONFIG_H

class CConfig {

public:
	CConfig ();

	UINT GetRowCount ();
	UINT GetColumnCount ();
	UINT GetRollingSpacesAreaWidth ();
private:
	UINT rowCount;
	UINT columnCount;

	UINT rollingSpacesAreaWidth;
};

#endif //C_CONFIG_H