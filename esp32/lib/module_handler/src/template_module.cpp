#include "template_module.hpp"

ModuleHandler::Module::Module(void) {}

ModuleHandler::Module::~Module(void) {}

void ModuleHandler::Module::Reset(void) { this->state = 0; }

int ModuleHandler::Module::State(void) { return this->state; }

int ModuleHandler::Module::Type() { return type; }
