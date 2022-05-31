#ifndef EKW_RAINBOW_SMOKE_HEADERS_COLOR_HPP
#define EKW_RAINBOW_SMOKE_HEADERS_COLOR_HPP

class Color {
public:
	using ParameterType = const Color&;

	double r;
	double g;
	double b;

	constexpr Color(double r, double g, double b): r(r), g(g), b(b) {};
	constexpr Color(double val): Color(val, val, val) {};
	constexpr Color(): Color(0, 0, 0) {};

	const double& operator[](int i) const;
	double& operator[](int i);


	// Equality Operators
	constexpr bool operator==(ParameterType that) {
		return (this->r == that.r) && (this->g == that.g) && (this->b == that.b);
	}
	constexpr bool operator!=(ParameterType that) {
		return (this->r != that.r) || (this->g != that.g) || (this->b != that.b);
	}
	
	// Simple Operations
	constexpr Color operator+(ParameterType that) const {
		return Color(r + that.r, g + that.g, b + that.b);
	}
	constexpr Color operator-(ParameterType that) const {
		return Color(r - that.r, g - that.g, b - that.b);
	}
	constexpr Color operator*(ParameterType that) const {
		return Color(r * that.r, g * that.g, b * that.b);
	}
	constexpr Color operator/(ParameterType that) const {
		return Color(r / that.r, g / that.g, b / that.b);
	}


	Color pow(ParameterType that) const;

	double greyValue() const;
	constexpr double dot(ParameterType that) const {
		return (r * that.r) + (g * that.g) + (b * that.b);
	}

	double dist(ParameterType that) const;

	constexpr static Color min(ParameterType c0, ParameterType c1) {
		return Color(
			(c0.r < c1.r)? c0.r : c1.r,
			(c0.g < c1.g)? c0.g : c1.g,
			(c0.b < c1.b)? c0.b : c1.b
		);
	}
	constexpr static Color max(ParameterType c0, ParameterType c1) {
		return Color(
			(c0.r > c1.r)? c0.r : c1.r,
			(c0.g > c1.g)? c0.g : c1.g,
			(c0.b > c1.b)? c0.b : c1.b
		);
	}
	constexpr Color clamp(ParameterType low, ParameterType high) const {
		return Color::min(Color::max(*this, low), high);
	}
};

struct IntegralColor {
	unsigned int r;
	unsigned int g;
	unsigned int b;
};

using RB_Color = Color;

#endif