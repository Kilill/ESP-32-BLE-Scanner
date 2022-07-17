#ifndef __settings_h__
#define __settings_h__
#include <stdio.h>
#include <string>
#include <SPIFFS.h>


#define MAX_VAR_SIZE 255

class Settings {

public:

// TODO: fsValid should probably  static.
// it could possibly lead to inconsistencies
// where one instance is valid while another is not
// which does not make sence;

	bool fsValid;
	std::string fileName;
	File file;

	Settings(std::string fileName_arg) {
		fileName=fileName_arg;

		if (SPIFFS.begin())
			fsValid=true;
	}


	bool openFile() {return openFile("r");}
	bool openFile(const char* mode);

	void closeFile(){ file.close();};

	virtual bool load()=0;
	virtual bool save()=0;
};

#endif
