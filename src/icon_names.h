#ifndef ICON_NAMES_H
#define ICON_NAMES_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-variable"

/** @namespace IconNames
 *  @brief Icon names for KIconLoader used by BasKet */
namespace IconNames
{
#define StrRes static const char*

StrRes  LOADING = "process-idle";
StrRes  LOCKED = "object-locked";

// Insert stuff
StrRes  LINK = "insert-link";
StrRes  CROSS_REF = LINK;
StrRes  IMAGE = "insert-image";
StrRes  COLOR = "fill-color";
StrRes  LAUNCH = "system-run";
StrRes  KMENU = "kde";
StrRes  ICONS = "preferences-desktop-icons";

// Import from
StrRes  TOMBOY = "tomboy";

#undef StrRes
}

#pragma GCC diagnostic pop

#endif // ICON_NAMES_H
