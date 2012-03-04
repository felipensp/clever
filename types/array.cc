/**
 * Clever programming language
 * Copyright (c) 2011-2012 Clever Team
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

#include "types/type.h"
#include "types/array.h"
#include "compiler/compiler.h"

namespace clever {

/**
 * Void Array::__assign__(Array)
 */
CLEVER_METHOD(Array::do_assign) {
	CLEVER_THIS()->copy(CLEVER_ARG(0));
}

/**
 * Void Array<T>::push(T)
 */
CLEVER_METHOD(Array::push) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	Value* val = new Value();
	val->copy(CLEVER_ARG(0));

	vec->push_back(val);
}

/**
 * T Array<T>::pop()
 */
CLEVER_METHOD(Array::pop) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	if (vec->size() > 0) {
		retval->copy(vec->back());
		vec->back()->delRef();
		vec->pop_back();
	}
	else {
		const Type* value_type = ((const TemplatedType*)CLEVER_THIS()
			->getTypePtr())->getTypeArg(0);

		Compiler::warningf("Popping from a empty Array<%S>! Returning"
			" default value of type %S.",
			value_type->getName(), value_type->getName());

		retval->setTypePtr(value_type);
		retval->initialize();
	}
}

/**
 * Int Array<T>::size()
 */
CLEVER_METHOD(Array::size) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	CLEVER_RETURN_INT(vec->size());
}

/**
 * Bool Array<T>::isEmpty()
 */
CLEVER_METHOD(Array::isEmpty) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	CLEVER_RETURN_BOOL(vec->empty());
}

/**
 * Void Array<T>::clear()
 */
CLEVER_METHOD(Array::clear) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	size_t sz = vec->size();
	for (size_t i = 0; i < sz; ++i) {
		vec->at(i)->delRef();
	}

	vec->clear();
}

/**
 * T Array<T>::at(Int)
 */
CLEVER_METHOD(Array::at) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());
	int64_t idx = CLEVER_ARG(0)->getInteger();
	uint64_t uidx = static_cast<uint64_t>(idx);
	int is_in_range = uidx < vec->max_size() &&
		uidx < vec->size() && idx >= 0;

	if (is_in_range) {
		retval->copy(vec->at(idx));
	}
	else {
		const Type* value_type = ((const TemplatedType*)CLEVER_THIS()
			->getTypePtr())->getTypeArg(0);

		clever_assert(value_type != NULL, "Cannot be NULL");

		if (idx < 0)  {
			Compiler::warningf("Indexing negative position %l an Array<%S>! "
					"Returning default value of type %S.",
				idx, value_type->getName(), value_type->getName());
		} else if (uidx > vec->max_size()) {
			clever_fatal("Attempted to access %l in an Array<%S>, but this platform limits "
					"arrays to %l entries.", idx, value_type->getName(), vec->max_size());
		} else {
			Compiler::warningf("Setting position %l an Array<%S> with %N elements.",
				idx, value_type->getName(), vec->size());
		}

		retval->setTypePtr(value_type);
		retval->initialize();
	}
}

/**
 * Void Array<T>::set(Int, T)
 */
CLEVER_METHOD(Array::set) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());
	int64_t idx = CLEVER_ARG(0)->getInteger();
	uint64_t uidx = static_cast<uint64_t>(idx);
	int is_in_range = uidx < vec->max_size() && uidx < vec->size() && idx >= 0;

	if (is_in_range) {
		Value* val = new Value();
		val->copy(CLEVER_ARG(1));

		vec->at(idx)->delRef();
		vec->at(idx) = val;
	}
	else {
		const Type* value_type = ((const TemplatedType*)CLEVER_THIS()
			->getTypePtr())->getTypeArg(0);

		if (idx < 0)  {
			Compiler::warningf("Setting negative position %l an Array<%S>!",
				idx, value_type->getName());
		} else if (uidx > vec->max_size()) {
			clever_fatal("Attempted to set %l in an Array<%S>, but this platform limits "
					"arrays to %l entries.", idx, value_type->getName(), vec->max_size());
		} else {
			Compiler::warningf("Setting position %l an Array<%S> with %N elements.",
				idx, value_type->getName(), vec->size());
		}
	}
}

/**
 * Void Array<T>::resize()
 */
CLEVER_METHOD(Array::resize) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());
	int64_t nsz = CLEVER_ARG(0)->getInteger();
	size_t sz = vec->size();

	if (static_cast<uint64_t>(nsz) >= vec->max_size()) {
		const Type* value_type = ((const TemplatedType*)CLEVER_THIS()
			->getTypePtr())->getTypeArg(0);

		clever_fatal("Attempted to resize an Array<%S> to %l entries, but this platform limits "
				"arrays to %l entries.", nsz, value_type->getName(), vec->max_size());
	}

	for (size_t i = 0; i < sz; ++i) {
		vec->at(i)->delRef();
	}

	vec->resize(nsz);

	for (size_t i = 0; i < static_cast<uint64_t>(nsz); ++i) {
		vec->at(i) = new Value();
		vec->at(i)->copy(CLEVER_ARG(1));
	}
}


CLEVER_METHOD(Array::slice) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());
	size_t sz = vec->size();

	int64_t start = CLEVER_ARG(0)->getInteger();
	int64_t length = CLEVER_ARG(1)->getInteger();

	int64_t r_start = 0;
	int64_t r_end = 0;

	if (start < 0) {
		r_start = sz + start;
	}
	else {
		r_start = start;
	}

	if (length <= 0) {
		r_end = sz + length;
	}
	else {
		r_end = r_start + length;
	}

	ValueVector* n_vec = new ValueVector;

	if (r_start >= (int64_t) sz) {
		Compiler::warningf("Value of start param (%l) is greater than Array size.", start);
	}
	else if (r_end > (int64_t) sz) {
		Compiler::warningf("Wrong value of length param (%l)", start);
	}
	else if (r_start > r_end) {
		Compiler::warningf("The length param value (%l) must be valid.", length);
	}
	else {
		for (int64_t i = r_start; i < r_end; ++i) {
			Value* val = new Value();
			val->copy(vec->at(i));
			n_vec->push_back(val);
		}
	}

	CLEVER_RETURN_ARRAY(n_vec);
}

/**
 * String Array<T>::toString()
 */
CLEVER_METHOD(Array::toString) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());
	std::string ret = "[", sep = ", ";

	for (size_t i = 0, j = vec->size(); i < j; ++i) {
		ret += vec->at(i)->toString();
		if (i+1 < j) {
			ret += sep;
		}
	}
	ret += "]";

	CLEVER_RETURN_STR(CSTRING(ret));
}

/**
 * Int Array<T>::find(T v)
 * Returns the position of the first value equal to `v' or -1 if
 * the value is not present in this Array
 */
CLEVER_METHOD(Array::find) {
	ValueVector* vec = CLEVER_GET_ARRAY(CLEVER_THIS());

	// Builds the ValueVector and the TypeVector to retrive and call the method
	ValueVector vv(2, CLEVER_ARG(0));
	TypeVector tv(2, CLEVER_THIS_ARG(0));

	// Gets the method
	const Method* const method =
		CLEVER_THIS_ARG(0)->getMethod(CSTRING(CLEVER_OPERATOR_EQUAL), &tv);

	Value ret;
	int64_t pos = -1;
	for (size_t i = 0, sz = vec->size(); i < sz; ++i) {
		vv[1] = vec->at(i);

		// Calls this[i] == CLEVER_ARG(0)
		method->call(&vv, &ret, vec->at(i));

		if (ret.getBoolean()) {
			pos = i;
			break;
		}
	}

	CLEVER_RETURN_INT(pos);
}

/**
 * Array type initializator
 */
void Array::init() {
	/**
	 * Check if we are in our "virtual" Array type
	 */
	if (CLEVER_TPL_ARG(0) == NULL) {
		return;
	}

	const Type* arr_t = CLEVER_GET_ARRAY_TEMPLATE->getTemplatedType(CLEVER_TPL_ARG(0));

	addMethod(
		(new Method(CLEVER_OPERATOR_ASSIGN, &Array::do_assign, CLEVER_STR, false))
			->addArg("rvalue", arr_t)
	);

	addMethod(new Method("toString", &Array::toString, CLEVER_STR));

	addMethod(
		(new Method("push", &Array::push, CLEVER_VOID, false))
			->addArg("arg1", CLEVER_TPL_ARG(0))
	);

	addMethod(
		new Method("pop", &Array::pop, CLEVER_TPL_ARG(0), false)
	);

	addMethod(new Method("size", &Array::size, CLEVER_INT));

	addMethod(new Method("isEmpty", &Array::isEmpty, CLEVER_BOOL));

	addMethod(new Method("clear", &Array::clear, CLEVER_VOID));

	addMethod((new Method("at", &Array::at, CLEVER_TPL_ARG(0)))
		->addArg("index", CLEVER_INT)
	);

	addMethod((new Method(CLEVER_OPERATOR_AT, &Array::at, CLEVER_TPL_ARG(0)))
		->addArg("index", CLEVER_INT)
	);

	addMethod((new Method("set", &Array::set, CLEVER_VOID, false))
		->addArg("index", CLEVER_INT)
		->addArg("element", CLEVER_TPL_ARG(0))
	);

	addMethod((new Method("resize", &Array::resize, CLEVER_VOID, false))
		->addArg("new_size", CLEVER_INT)
		->addArg("value", CLEVER_TPL_ARG(0))
	);

	addMethod((new Method("slice", &Array::slice, arr_t))
		->addArg("start", CLEVER_INT)
		->addArg("length", CLEVER_INT)
	);

	addMethod((new Method("find", &Array::find, CLEVER_INT))
		->addArg("value", CLEVER_TPL_ARG(0))
	);
}

DataValue* Array::allocateValue() const {
	return new ArrayValue;
}

} // clever
