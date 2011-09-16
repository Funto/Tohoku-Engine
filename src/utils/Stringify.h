// Stringify.h
// Examples:
// STRINGIFY(WIDTH)       gives "640"
// STRINGIFY_FLOAT(WIDTH) gives "640.0"

#ifndef STRINGIFY_H
#define STRINGIFY_H

#define STRINGIFY(a)        __STRINGIFY(a)
#define STRINGIFY_FLOAT(a)  __STRINGIFY(a.0)

#define __STRINGIFY(a) #a

#endif // STINGIFY_H
