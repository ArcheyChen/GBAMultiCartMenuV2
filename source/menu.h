#pragma once
#include <vector>
#include <gba_base.h>
#include <string>
#include <cstdio>

#define CONSOLE_HEIGHT 11

class Menu{
	const char* title;
	std::vector<std::string> options;
	int height;
	int selected;
	int offset;
	s8 timer;
	void printSelection();
	
	
	
	
	
	void moveUp();
	void moveDown();
	
	
public:
	Menu();
	Menu(const char* title);
	void clear();
	void addOption(std::string name);
	int getDecision();	
	void setTitle(const char* t);
	int size();

	
	
	
	
	void printNumericalSelection();
	int getNumerical();
	
	int numerical_min;
	int numerical_max;
	int numerical_increment;
};



Menu numericalMenu(const char* title, int min, int max, int increment);