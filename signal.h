#ifndef SIGNAL_H
#define SIGNAL_H

#include <functional>

// The type of a signal throughout the system. Will be in the range [-1, 1].
using SignalType = float;

// A function which takes in a signal, transforms it, and returns a new signal.
using SignalTransformFn = std::function<SignalType(SignalType)>;

#endif /* SIGNAL_H */
