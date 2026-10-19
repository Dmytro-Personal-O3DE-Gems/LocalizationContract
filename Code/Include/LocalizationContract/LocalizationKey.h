#pragma once

//! Marks a string literal as a localization key, e.g. LOCALIZATION_KEY("door.open").
//! For the compiler it is just that string. Tools find the keys in source code by searching for
//! this marker (LocalizationGem/Tools/check_localization.py), so write every key through it.
//! The "" in front makes anything but a string literal fail to compile: a key the tool cannot see
//! (a variable, a concatenation) is rejected right away.
#define LOCALIZATION_KEY(key) "" key