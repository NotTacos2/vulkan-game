#pragma once
#include <vector>
#include <iostream>

std::vector<std::string> split(std::string line, std::string delimiter);

enum class pipelineType {
	SKY,
	STANDARD
};