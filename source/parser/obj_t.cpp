#include "obj_t.h"
#include <string>

obj_t::obj_t() {
	isReference = false;
	isAttributeReference = false;
}

obj_t::~obj_t() {

}

obj_t::obj_t(double v, std::string t) {
	valor = v;
	tipo = t;
	id = 0;
	isReference = false;
	isAttributeReference = false;
}

obj_t::obj_t(double v, std::string t, unsigned long uid) {
	valor = v;
	tipo = t;
	id = uid;
	isReference = false;
	isAttributeReference = false;
}
