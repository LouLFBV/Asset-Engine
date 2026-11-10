#pragma once

#include <string>
#include <memory>

struct Asset {

	std::string uiid; // pour asset
	char* path;
	//Pawn* objet; //s'il est load (un ptr),
	//et type RTTI (classe)
};
