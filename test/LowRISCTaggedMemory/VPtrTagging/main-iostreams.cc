/* Included here because the initialisation code needs to not break existing
 * global constructors. C++ iostreams use global initialisers. */

#include <iostream>

int main() {
	std::cout << "Hello world!\n";
	return 0;
}
