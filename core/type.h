/**
 * Clever programming language
 * Copyright (c) Clever Team
 *
 * This file is distributed under the MIT license. See LICENSE for details.
 */

#ifndef CLEVER_TYPE_H
#define CLEVER_TYPE_H

#ifdef CLEVER_MSVC
#include <unordered_map>
#else
#include <tr1/unordered_map>
#endif
#include <vector>
#include "core/refcounted.h"
#include "core/clever.h"
#include "core/cstring.h"

namespace clever {

class Value;
class Type;
class Function;

#define CLEVER_TYPE_OPERATOR_ARGS       Value* result, const Value* lhs, const Value* rhs, Clever* clever
#define CLEVER_TYPE_UNARY_OPERATOR_ARGS Value* result, const Value* lhs, Clever* clever
#define CLEVER_TYPE_AT_OPERATOR_ARGS    const Value* obj, const Value* index, bool is_write, Clever* clever

#define CLEVER_TYPE_OPERATOR(name)       void CLEVER_FASTCALL name(CLEVER_TYPE_OPERATOR_ARGS) const
#define CLEVER_TYPE_UNARY_OPERATOR(name) void CLEVER_FASTCALL name(CLEVER_TYPE_UNARY_OPERATOR_ARGS) const
#define CLEVER_TYPE_AT_OPERATOR(name)    Value* CLEVER_FASTCALL name(CLEVER_TYPE_AT_OPERATOR_ARGS) const

#define CLEVER_TYPE_VIRTUAL_METHOD_DECLARATIONS                          \
	void CLEVER_FASTCALL add(CLEVER_TYPE_OPERATOR_ARGS)           const; \
	void CLEVER_FASTCALL sub(CLEVER_TYPE_OPERATOR_ARGS)           const; \
	void CLEVER_FASTCALL mul(CLEVER_TYPE_OPERATOR_ARGS)           const; \
	void CLEVER_FASTCALL div(CLEVER_TYPE_OPERATOR_ARGS)           const; \
	void CLEVER_FASTCALL mod(CLEVER_TYPE_OPERATOR_ARGS)           const; \
	void CLEVER_FASTCALL equal(CLEVER_TYPE_OPERATOR_ARGS)         const; \
	void CLEVER_FASTCALL not_equal(CLEVER_TYPE_OPERATOR_ARGS)     const; \
	void CLEVER_FASTCALL greater(CLEVER_TYPE_OPERATOR_ARGS)       const; \
	void CLEVER_FASTCALL greater_equal(CLEVER_TYPE_OPERATOR_ARGS) const; \
	void CLEVER_FASTCALL less(CLEVER_TYPE_OPERATOR_ARGS)          const; \
	void CLEVER_FASTCALL less_equal(CLEVER_TYPE_OPERATOR_ARGS)    const; \
	void CLEVER_FASTCALL not_op(CLEVER_TYPE_UNARY_OPERATOR_ARGS)  const

#define CLEVER_TYPE_VIRTUAL_BITWISE_OPERATORS                           \
	void CLEVER_FASTCALL bw_and(CLEVER_TYPE_OPERATOR_ARGS)       const; \
	void CLEVER_FASTCALL bw_or(CLEVER_TYPE_OPERATOR_ARGS)        const; \
	void CLEVER_FASTCALL bw_xor(CLEVER_TYPE_OPERATOR_ARGS)       const; \
	void CLEVER_FASTCALL bw_not(CLEVER_TYPE_UNARY_OPERATOR_ARGS) const; \
	void CLEVER_FASTCALL bw_ls(CLEVER_TYPE_OPERATOR_ARGS)        const; \
	void CLEVER_FASTCALL bw_rs(CLEVER_TYPE_OPERATOR_ARGS)        const

#define CLEVER_TYPE_INIT(name) void name()

#define CLEVER_METHOD_ARGS Value* result, const Value* obj, const ::std::vector<Value*>& args, Clever* clever
#define CLEVER_METHOD_PASS_ARGS result, obj, args, clever
#define CLEVER_METHOD(name) void name(CLEVER_METHOD_ARGS) const

typedef void (Type::*MethodPtr)(CLEVER_METHOD_ARGS) const;

struct MemberData {
	enum { PUBLIC, PRIVATE };

	Value* value;
	size_t flags;

	MemberData(Value* value_, size_t flags_)
		: value(value_), flags(flags_) {}
};

typedef std::tr1::unordered_map<const CString*, MemberData> MemberMap;
typedef std::tr1::unordered_map<const CString*, MemberData> PropertyMap;
typedef std::tr1::unordered_map<const CString*, Function*> MethodMap;

// TODO(heuripedes): investigate the significance of this class.
class TypeObject : public RefCounted {
public:
	TypeObject()
		: m_initialized(false) {}

	virtual ~TypeObject();

	void copyMembers(const Type*);

	void addMember(const CString* name, MemberData data) {
		m_members.insert(MemberMap::value_type(name, data));
	}

	virtual MemberData getMember(const CString* name) const {
		MemberMap::const_iterator it = m_members.find(name);

		if (it != m_members.end()) {
			return it->second;
		}

		return MemberData(NULL, 0);
	}

	const MemberMap& getMembers() const { return m_members; }

	virtual TypeObject* clone() const { return NULL; }

	void initialize(const Type* type) {
		if (!m_initialized) {
			copyMembers(type);
		}
	}
private:
	MemberMap m_members;

	/// Flag to indicate if the members were loaded into the instance
	bool m_initialized;

	DISALLOW_COPY_AND_ASSIGN(TypeObject);
};

/**
 * @brief TypeObject specialization for simple/single data storage
 */
template <typename T>
struct SimpleTypeObject : public TypeObject {
	SimpleTypeObject()
		: TypeObject() {}

	explicit SimpleTypeObject(const T& v)
		: TypeObject(), value(v) {}

	virtual TypeObject* clone() const { return new SimpleTypeObject<T>(value); }

	T value;
};

class Type {
public:
	enum TypeFlag { INTERNAL_TYPE, USER_TYPE };

	Type()
		: m_flags(INTERNAL_TYPE) {}

	Type(const std::string& name, TypeFlag flags = INTERNAL_TYPE)
		: m_name(name), m_ctor(NULL), m_dtor(NULL), m_user_ctor(NULL),
			m_user_dtor(NULL), m_flags(flags) {}

	virtual ~Type() {}

	void deallocMembers();

	bool isUserDefined() const { return m_flags == USER_TYPE; }
	bool isInternal() const { return m_flags == INTERNAL_TYPE; }

	void addMember(const CString* name, MemberData data) {
		m_members.insert(MemberMap::value_type(name, data));
	}

	MemberData getMember(const CString* name) const {
		MemberMap::const_iterator it = m_members.find(name);

		if (it != m_members.end()) {
			return it->second;
		}

		return MemberData(NULL, 0);
	}

	bool hasMember(const CString* name) const {
		return getMember(name).value != NULL;
	}

	bool hasMembers() const { return !m_members.empty(); }
	const MemberMap& getMembers() const { return m_members; }

	Function* addMethod(Function*, size_t = MemberData::PUBLIC);

	void addProperty(const std::string& name, Value* value, size_t flags = MemberData::PUBLIC) {
		addProperty(CSTRING(name), value, flags);
	}

	void addProperty(const CString* name, Value* value, size_t flags = MemberData::PUBLIC) {
		addMember(name, MemberData(value, flags));
	}

	MemberData getProperty(const CString*) const;
	MemberData getMethod(const CString*) const;

	const MethodMap getMethods() const;
	const PropertyMap getProperties() const;

	/// Method for retrieve the type name
	const std::string& getName() const { return m_name; }

	void setConstructor(MethodPtr method);
	void setDestructor(MethodPtr method);

	const Function* getConstructor() const { return m_ctor; }
	const Function* getDestructor() const { return m_dtor; }

	void setUserConstructor(Function* func) { m_user_ctor = func; }
	const Function* getUserConstructor() const { return m_user_ctor; }
	bool hasUserConstructor() const { return m_user_ctor != NULL; }

	void setUserDestructor(Function* func) { m_user_dtor = func; }

	/// Virtual method for type initialization
	virtual void init() {}

	/// Virtual method for debug purpose
	virtual void dump(TypeObject* data) const { dump(data, std::cout); }
	virtual void dump(TypeObject* data, std::ostream& out) const { out << toString(data); }

	/// Virtual method for stringify object
	virtual std::string toString(TypeObject*) const { return getName(); }

	/// Operator methods
	virtual void CLEVER_FASTCALL add(CLEVER_TYPE_OPERATOR_ARGS)           const;
	virtual void CLEVER_FASTCALL sub(CLEVER_TYPE_OPERATOR_ARGS)           const;
	virtual void CLEVER_FASTCALL mul(CLEVER_TYPE_OPERATOR_ARGS)           const;
	virtual void CLEVER_FASTCALL div(CLEVER_TYPE_OPERATOR_ARGS)           const;
	virtual void CLEVER_FASTCALL mod(CLEVER_TYPE_OPERATOR_ARGS)           const;
	virtual void CLEVER_FASTCALL equal(CLEVER_TYPE_OPERATOR_ARGS)         const;
	virtual void CLEVER_FASTCALL not_equal(CLEVER_TYPE_OPERATOR_ARGS)     const;
	virtual void CLEVER_FASTCALL not_op(CLEVER_TYPE_UNARY_OPERATOR_ARGS)  const;
	virtual void CLEVER_FASTCALL greater(CLEVER_TYPE_OPERATOR_ARGS)       const;
	virtual void CLEVER_FASTCALL greater_equal(CLEVER_TYPE_OPERATOR_ARGS) const;
	virtual void CLEVER_FASTCALL less(CLEVER_TYPE_OPERATOR_ARGS)          const;
	virtual void CLEVER_FASTCALL less_equal(CLEVER_TYPE_OPERATOR_ARGS)    const;
	virtual void CLEVER_FASTCALL bw_and(CLEVER_TYPE_OPERATOR_ARGS)        const;
	virtual void CLEVER_FASTCALL bw_or(CLEVER_TYPE_OPERATOR_ARGS)         const;
	virtual void CLEVER_FASTCALL bw_xor(CLEVER_TYPE_OPERATOR_ARGS)        const;
	virtual void CLEVER_FASTCALL bw_ls(CLEVER_TYPE_OPERATOR_ARGS)         const;
	virtual void CLEVER_FASTCALL bw_rs(CLEVER_TYPE_OPERATOR_ARGS)         const;
	virtual void CLEVER_FASTCALL bw_not(CLEVER_TYPE_UNARY_OPERATOR_ARGS)  const;
	virtual Value* CLEVER_FASTCALL at_op(CLEVER_TYPE_AT_OPERATOR_ARGS)    const;
	virtual void increment(Value*, Clever*) const;
	virtual void decrement(Value*, Clever*) const;

	/// Virtual methods for serialization interface
	virtual std::pair<size_t, TypeObject*> serialize(const Value*) const;
	virtual Value* unserialize(const Type*, const std::pair<size_t, TypeObject*>&) const;
private:
	MemberMap m_members;
	std::string m_name;
	const Function* m_ctor;
	const Function* m_dtor;
	const Function* m_user_ctor;
	const Function* m_user_dtor;
	TypeFlag m_flags;

	DISALLOW_COPY_AND_ASSIGN(Type);
};

}

#endif // CLEVER_TYPE_H
