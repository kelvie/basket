#ifndef ICON_NAMES_H
#define ICON_NAMES_H

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

#undef StrRes
}


#endif // ICON_NAMES_H
