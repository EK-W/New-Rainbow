#include "Color.hpp"
#include <stdexcept>
#include <string>
#include <cmath>

const double& Color::operator[](int i) const {
	switch(i) {
	case 0:
		return r;
	case 1:
		return g;
	case 2:
		return b;
	default:
		throw std::range_error(std::to_string(i) + " is not a valid color channel index!");
	}
}
double& Color::operator[](int i) {
	switch(i) {
	case 0:
		return r;
	case 1:
		return g;
	case 2:
		return b;
	default:
		throw std::range_error(std::to_string(i) + " is not a valid color channel index!");
	}
}

Color Color::pow(const Color& that) const {
	return Color(std::pow(r, that.r), std::pow(g, that.g), std::pow(b, that.b));
}

double Color::greyValue() const {
	return dist(Color(0, 0, 0)) / sqrt(3);
}
double Color::dist(const Color& that) const {
	return sqrt(std::pow(r - that.r, 2) + std::pow(g - that.g, 2) + std::pow(b - that.b, 2));
}