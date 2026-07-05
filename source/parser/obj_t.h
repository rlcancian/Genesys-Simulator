#ifndef OBJ_T_H
#define OBJ_T_H
#include <string>

class obj_t {
public:
	obj_t();
	virtual ~obj_t();
	obj_t(double v, std::string t);
	obj_t(double v, std::string t, unsigned long uid);
	double valor;
	std::string tipo;
	unsigned long id;
	bool isReference;
	bool isAttributeReference;
};

#endif
