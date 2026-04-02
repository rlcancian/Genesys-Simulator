#pragma once

#include <string>

/*
struct Color {
    static constexpr Color red() { return Color(255, 0, 0); }
    static constexpr Color green() { return Color(0, 255, 0); }
    static constexpr Color blue() { return Color(0, 0, 255); }
    static constexpr Color black() { return Color(0, 0, 0); }
    static constexpr Color white() { return Color(255, 255, 255); }
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : r(r), g(g), b(b), a(a) {}
     uint8_t r;
     uint8_t g;
     uint8_t b;
     uint8_t a;
};
*/

/*
class ColorCA {
public:

	ColorCA() {
		this->r = 0.0;
		this->g = 0.0;
		this->b = 0.0;
		this->a = 1.0;
	}
    ColorCA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255) {
		this->r = (double) (r)/255.0;
		this->g = (double) (g)/255.0;
		this->b = (double) (b)/255.0;
		this->a = (double) (a)/255.0;
    }
    std::string show() {
        return "(" + std::to_string(r)+"," + std::to_string(g)+"," + std::to_string(b)+")";
	}
	//uint8_t
	double r;
	double g;
	double b;
	double a;
};
*/

/*
template<class T>
class State {
public:
	State<T>() {}
	State<T>(T value) {_value=value;}
	State<T>(const State& orig){}
	virtual ~State<T>() = default;
public:
	T getValue(){return _value;}
	void setValue(T value) {_value=value;}
protected:
	T _value;
private:
};
*/

class State {
public:
	State() {}
	State(long value) {_value=value;}
	State(const State& orig){}
	virtual ~State() = default;
public:
	long getValue(){return _value;}
	void setValue(long value) {_value=value;}
	std::string show();
protected:
	long _value;
private:
};
