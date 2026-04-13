#include "Environment.h"

Environment* Environment::GetInstance() {
	static Environment gameSystem;
	return &gameSystem;
}