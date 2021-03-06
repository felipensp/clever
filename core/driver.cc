/**
 * Clever programming language
 * Copyright (c) Clever Team
 *
 * This file is distributed under the MIT license. See LICENSE for details.
 */

#include <fstream>
#include <setjmp.h>
#include "core/cstring.h"
#include "core/driver.h"
#include "core/parser.hh"
#include "core/position.hh"
#include "core/vm.h"
#include "core/scanner.h"

namespace clever {

int* g_clever_argc;
char*** g_clever_argv;

Interpreter::Interpreter(int* argc, char*** argv)
{
	g_clever_argc = argc;
	g_clever_argv = argv;
}

/// Executes the script
void Interpreter::execute(bool interactive)
{
	if ((getCompilerFlags() & clever::Compiler::PARSER_ONLY)
		|| m_loaded == false) {
		return;
	}

	int status = setjmp(fatal_error);

	if (status == 0) {
		m_compiler.genCode();

		VM vm(m_compiler.getIR());

		vm.setConstEnv(m_compiler.getConstEnv());
		vm.setGlobalEnv(m_compiler.getGlobalEnv());

#ifdef CLEVER_DEBUG
		if (m_dump_opcode) {
			vm.dumpOpcodes();
		}
#endif
		vm.run();
	}
}

/// Frees the resource used to load and execute the script
void Interpreter::shutdown()
{
	m_compiler.shutdown();
}

/// Read the file defined in file property
void Driver::readFile(std::string& source) const
{
	std::string line;
	std::fstream filep(m_file->c_str());

	if (!filep) {
		std::cerr << "Couldn't open file " << *m_file << std::endl;
		exit(1);
	}
	source = "";

	while (!filep.eof()) {
		getline(filep, line);
		source += line + '\n';
	}

	filep.clear();
	filep.close();
}

/// Starts the parsing of the supplied file
/// \returns -1 when a parser error happens, otherwise 0 is returned
int Driver::loadFile(const std::string& filename, const std::string& ns_name)
{
	m_compiler.setFlags(m_cflags);
	m_compiler.setNamespace(ns_name);

	ScannerState* new_scanner = new ScannerState;
	Parser parser(*this, *new_scanner, m_compiler);
	std::string& source = new_scanner->getSource();

	m_is_file = true;
	m_file = CSTRING(filename);

	readFile(source);

	m_scanners.push(new_scanner);

	const unsigned char* s = reinterpret_cast<const unsigned char*>(source.c_str());

	m_scanners.top()->set_cursor(s);
	m_scanners.top()->set_limit(s + source.length());

	// Bison debug option
	parser.set_debug_level(m_trace_parsing);

	int result = parser.parse();

	delete new_scanner;
	m_scanners.pop();

	if (m_scanners.empty()) {
		m_loaded |= result == 0;
	}

	return result;
}

/// Starts the parsing of the supplied string
/// \returns -1 when a parser error happens, otherwise 0 is returned
int Driver::loadStr(const std::string& code, bool importStd)
{
	m_compiler.setFlags(m_cflags);

	ScannerState *new_scanner = new ScannerState;
	Parser parser(*this, *new_scanner, m_compiler);
	std::string& source = new_scanner->getSource();

	if (importStd) {
		source += std::string("import std.*;");
	}
	source += code;

	// Set the source code to scanner read it
	m_scanners.push(new_scanner);

	const unsigned char* s = reinterpret_cast<const unsigned char*>(source.c_str());
	m_scanners.top()->set_cursor(s);
	m_scanners.top()->set_limit(s + source.length());

	// Bison debug option
	parser.set_debug_level(m_trace_parsing);

	int result = parser.parse();

	delete new_scanner;
	m_scanners.pop();

	if (m_scanners.empty()) {
		m_loaded |= result == 0;
	}

	return result;
}

/// Prints an error message and exit
void Driver::error(const location& location, const std::string& msg) const
{
	position last = location.end - 1;

	std::cerr << "Error: ";
	if (last.filename) {
		std::cerr << msg << " in " << *last.filename <<
			" on line " << last.line << std::endl;
	} else {
		std::cerr << msg << " on line " << last.line << std::endl;
	}
}

/// Prints an error message
void Driver::error(const std::string& message) const
{
	std::cerr << message << std::endl;
}

} // clever
